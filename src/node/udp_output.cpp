#include "node/udp_output.h"
#include "utils/utils.h"

TSTM_BEGIN_NAMESPACE

constexpr int64_t kBufferTime = 50 * kMsInNs;
constexpr size_t kMaxBufferToken = 1000;

UdpOutputNode::UdpOutputNode(const std::string &id) :
    BaseNode("udp_output", id),
    curr_time_(0),
    next_work_time_(0),
    work_state_(WorkStates::kDeinit),
    token_(nullptr),
    fd_(-1),
    next_send_time_(0)
{
    lccl::skt::InitEnv();
    buffer_token_list_ = TokenList::New();
}

UdpOutputNode::~UdpOutputNode()
{
    Deinit();
    buffer_token_list_ = nullptr;
    lccl::skt::DeinitEnv();
}

bool UdpOutputNode::Init(const rapidjson::Value &param_val)
{
    if (!ParseParam(param_val))
    {
        return false;
    }

    curr_time_ = 0;
    next_work_time_ = 0;
    work_state_ = WorkStates::kDeinit;
    token_ = nullptr;
    next_send_time_ = 0;
    return true;
}

void UdpOutputNode::Deinit()
{
    buffer_token_list_->Clear();
    param_ = nullptr;
}

std::shared_ptr<TokenList> UdpOutputNode::Do(std::shared_ptr<TokenList> in_token_list)
{
    if (in_token_list)
    {
        if (buffer_token_list_->Size() > kMaxBufferToken)
        {
            TSTM_NODE_LOG(TSTM_LOG_WARN, "Buffer token list size={}, clear", buffer_token_list_->Size());
            buffer_token_list_->Clear();
            token_ = nullptr;
        }

        buffer_token_list_->Combine(in_token_list.get());
    }

    curr_time_ = Utils::Instance()->GetSteadyTime();
    if (curr_time_ < next_work_time_)
    {
        return nullptr;
    }

    switch (work_state_)
    {
    case WorkStates::kInit:
        InitWorkState();
        break;
    case WorkStates::kDeinit:
        DeinitWorkState();
        break;
    case WorkStates::kFail:
        FailWorkState();
        break;
    case WorkStates::kSending:
        SendingWorkState();
        break;
    default:
        work_state_ = WorkStates::kFail;
        break;
    }

    return nullptr;
}

bool UdpOutputNode::ParseParam(const rapidjson::Value &param_val)
{
    param_ = std::make_shared<Param>();
    if (!lccl::GetJsonChild(param_val, "dst_ip", param_->dst_ip))
    {
        TSTM_NODE_LOG(TSTM_LOG_ERROR, "Can't find ip");
        return false;
    }

    if (!lccl::GetJsonChild(param_val, "dst_port", param_->dst_port))
    {
        TSTM_NODE_LOG(TSTM_LOG_ERROR, "Can't find port");
        return false;
    }

    lccl::GetJsonChild(param_val, "local_ip", param_->local_ip);
    lccl::GetJsonChild(param_val, "local_port", param_->local_port);
    lccl::GetJsonChild(param_val, "ttl", param_->ttl);
    return true;
}

void UdpOutputNode::InitWorkState()
{
    if (!InitSocket())
    {
        work_state_ = WorkStates::kFail;
        return;
    }

    work_state_ = WorkStates::kSending;
}

void UdpOutputNode::DeinitWorkState()
{
    if (fd_ >= 0)
    {
#if defined (_MSC_VER)
        closesocket(fd_);
#else
        close(fd_);
#endif
        fd_ = -1;
    }

    next_work_time_ = curr_time_ + kMsInNs;
    work_state_ = WorkStates::kInit;
}

void UdpOutputNode::FailWorkState()
{
    DeinitWorkState();
    next_work_time_ = curr_time_ + 3 * kSecInNs;
}

void UdpOutputNode::SendingWorkState()
{
    while (true)
    {
        if (!token_)
        {
            token_ = buffer_token_list_->Top();
            if (!token_)
            {
                break;
            }

            next_send_time_ = token_->GetProperty(Token::Props::kTransTime) + kBufferTime;
        }

        if (curr_time_ < next_send_time_)
        {
            break;
        }

        send(fd_, reinterpret_cast<const char *>(token_->Data()->Buf()), static_cast<int>(token_->Data()->Size()), 0);
        curr_time_ = Utils::Instance()->GetSteadyTime();
        buffer_token_list_->Pop();
        token_ = nullptr;
    }
}

bool UdpOutputNode::InitSocket()
{
    // 创建发送fd
    switch (lccl::skt::GetIpType(param_->dst_ip.c_str()))
    {
    case lccl::skt::AddrTypes::kIpv4:
        fd_ = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
        break;
    case lccl::skt::AddrTypes::kIpv6:
        fd_ = static_cast<int>(socket(AF_INET6, SOCK_DGRAM, 0));
        break;
    default:
        fd_ = -1;
        break;
    }

    if (fd_ < 0)
    {
        TSTM_NODE_LOG(TSTM_LOG_ERROR, "create fd fail! url={}:{}", param_->dst_ip, param_->dst_port);
        return false;
    }

    // 设置socket为非阻塞模式
    lccl::skt::SetBlockMode(fd_, false);

    // 设置socket收发缓冲区
    int opt = 1 << 24;  // 16MB
    setsockopt(fd_, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char *>(&opt), sizeof(opt));
    setsockopt(fd_, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char *>(&opt), sizeof(opt));

    if (param_->ttl > 0)
    {
        lccl::skt::SetTTL(fd_, param_->ttl);
    }

    // 创建目的地址
    std::shared_ptr<lccl::skt::IAddr> dst_addr = lccl::skt::CreateAddr(param_->dst_ip.c_str(), param_->dst_port, false);
    if (!dst_addr)
    {
        TSTM_NODE_LOG(TSTM_LOG_ERROR, "create dst sockaddr fail! url={}:{}", param_->dst_ip, param_->dst_port);
        return false;
    }

    if ((dst_addr->IsMulticast()) || (param_->local_port > 0))
    {
        if (param_->local_ip.empty())
        {
            TSTM_NODE_LOG(TSTM_LOG_ERROR, "no local ip! url={}:{}", param_->dst_ip, param_->dst_port);
            return false;
        }

        // 创建本机地址
        std::shared_ptr<lccl::skt::IAddr> local_addr = lccl::skt::CreateAddr(param_->local_ip.c_str(), param_->local_port, true);
        if (!local_addr)
        {
            TSTM_NODE_LOG(TSTM_LOG_ERROR, "create local sockaddr fail! url={}:{} local addr={}:{}",
                param_->dst_ip, param_->dst_port, param_->local_ip, param_->local_port);
            return false;
        }

        if (local_addr->Bind(fd_) < 0)
        {
            TSTM_NODE_LOG(TSTM_LOG_ERROR, "bind local sockaddr fail! url={}:{} local addr={}:{}",
                param_->dst_ip, param_->dst_port, param_->local_ip, param_->local_port);
            return false;
        }

        if (dst_addr->IsMulticast())
        {
            if (local_addr->MulticastIf(fd_) < 0)
            {
                TSTM_NODE_LOG(TSTM_LOG_ERROR, "set multicast if fail! url={}:{} local addr={}:{}",
                    param_->dst_ip, param_->dst_port, param_->local_ip, param_->local_port);
                return false;
            }
        }
    }

    dst_addr->Connect(fd_);
    return true;
}

TSTM_END_NAMESPACE
