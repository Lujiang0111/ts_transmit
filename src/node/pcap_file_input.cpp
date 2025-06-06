#include "node/pcap_file_input.h"

TSTM_BEGIN_NAMESPACE

constexpr size_t kPcapGlobalHeaderSize = 24;
constexpr size_t kPcapPacketHeaderSize = 24;
constexpr size_t kUdpHeaderSize = 8;
constexpr size_t kReadPacketBundle = 10;

enum PcapMaginNumbers : int64_t
{
    kUsBe = 0xA1B2C3D4,
    kUsLE = 0xD4C3B2A1,
    kNsBe = 0xA1B23C4D,
    kNsLE = 0x4D3CB2A1,
};

PcapFileInputNode::PcapFileInputNode(const std::string &id) :
    BaseNode("pcap_file_input", id),
    hint_init_(true),
    pcap_begin_(nullptr),
    pcap_end_(nullptr),
    pcap_size_(0),
    pcap_reader_(nullptr),
    ip_buffer_(65),
    bo_(lccl::bit::ByteOrders::kLe),
    version_(0),
    time_precision_(1),
    network_type_(NetworkTypes::kEthernet),
    loop_(0),
    loop_end_(false),
    curr_packet_(nullptr),
    prev_timestamp_(0),
    prev_out_timestamp_(0)
{
    out_token_list_ = TokenList::New();
}

PcapFileInputNode::~PcapFileInputNode()
{
    Deinit();
    out_token_list_ = nullptr;
}

bool PcapFileInputNode::Init(const rapidjson::Value &param_val)
{
    if (!ParseParam(param_val))
    {
        return false;
    }

    if (!ParseBasicInfo())
    {
        return false;
    }

    loop_ = 0;
    loop_end_ = false;
    return true;
}

void PcapFileInputNode::Deinit()
{
    out_token_list_->Clear();
    curr_packet_ = nullptr;
    basic_info_ = nullptr;
    file_mmap_ = nullptr;
    param_ = nullptr;
}

std::shared_ptr<TokenList> PcapFileInputNode::Do(std::shared_ptr<TokenList> in_token_list)
{
    int64_t curr_time = Utils::Instance()->GetSteadyTime();
    size_t read_packet_cnt = 0;
    while ((!loop_end_) && (read_packet_cnt < kReadPacketBundle))
    {
        if (!curr_packet_)
        {
            ReadPacket();

            if (!curr_packet_)
            {
                if (loop_ == param_->loop)
                {
                    loop_end_ = true;
                }
                else
                {
                    ++loop_;
                    pcap_reader_.Load(packet_start_sp_);
                    TSTM_NODE_LOG(TSTM_LOG_ERROR, "File {} loop Times={}", param_->file_name, loop_);
                }
                continue;
            }

            ++read_packet_cnt;
            switch (curr_packet_->type)
            {
            case PacketTypes::kUdp:
                break;
            default:
                curr_packet_ = nullptr;
                continue;
            }

            if (0 == prev_timestamp_)
            {
                curr_packet_->out_timestamp = curr_time;
            }
            else
            {
                int64_t timestamp_diff = curr_packet_->timestamp - prev_timestamp_;
                if ((timestamp_diff > 0) && (timestamp_diff < param_->error_time_ns))
                {
                    curr_packet_->out_timestamp = prev_out_timestamp_ + timestamp_diff;
                }
                else
                {
                    TSTM_NODE_LOG(TSTM_LOG_ERROR, "Timestamp diff={} error! force out", timestamp_diff);
                    curr_packet_->out_timestamp = prev_out_timestamp_;
                }
            }
        }

        if (param_->real_time)
        {
            if (curr_time < curr_packet_->out_timestamp)
            {
                break;
            }

            std::shared_ptr<Token> token = Token::New(curr_packet_->size);
            memcpy(token->Data()->Buf(), curr_packet_->data, curr_packet_->size);
            token->SetProperty(Token::Props::kTransTime, curr_packet_->out_timestamp);
            out_token_list_->Push(token);

            prev_timestamp_ = curr_packet_->timestamp;
            prev_out_timestamp_ = curr_packet_->out_timestamp;
        }
        else
        {
            std::shared_ptr<Token> token = Token::New(curr_packet_->size);
            memcpy(token->Data()->Buf(), curr_packet_->data, curr_packet_->size);
            out_token_list_->Push(token);
        }

        curr_packet_ = nullptr;
    }

    if (0 == out_token_list_->Size())
    {
        return nullptr;
    }

    std::shared_ptr<TokenList> out_token_list = TokenList::New();
    out_token_list.swap(out_token_list_);
    return out_token_list;
}

const PcapFileInputNode::BasicInfo *PcapFileInputNode::GetBasicInfo() const
{
    return (basic_info_) ? basic_info_.get() : nullptr;
}

bool PcapFileInputNode::ParseParam(const rapidjson::Value &param_val)
{
    param_ = std::make_shared<Param>();
    if (!lccl::GetJsonChild(param_val, "file_name", param_->file_name))
    {
        TSTM_NODE_LOG(TSTM_LOG_ERROR, "Can't find file_name");
        return false;
    }

    if (lccl::GetJsonChild(param_val, "hint_ip", param_->hint_ip))
    {
        std::shared_ptr<lccl::skt::IAddr> addr = lccl::skt::CreateAddr(param_->hint_ip.c_str(), 0, false);
        if (!addr)
        {
            param_->hint_ip.clear();
        }
        else
        {
            param_->hint_ip = addr->GetIp();
        }
    }

    lccl::GetJsonChild(param_val, "hint_port", param_->hint_port);

    if ((!param_->hint_ip.empty()) || (param_->hint_port > 0))
    {
        hint_init_ = false;
    }
    else
    {
        hint_init_ = true;
    }

    if (!lccl::GetJsonChild(param_val, "loop", param_->loop))
    {
        param_->loop = kLoopForever;
    }

    if (!lccl::GetJsonChild(param_val, "real_time", param_->real_time))
    {
        param_->real_time = true;
    }

    if (!lccl::GetJsonChild(param_val, "error_time", param_->error_time_ns))
    {
        param_->error_time_ns = 60;
    }
    param_->error_time_ns *= kSecInNs;

    return true;
}

bool PcapFileInputNode::ParseBasicInfo()
{
    file_mmap_ = lccl::file::CreateFileMmap(param_->file_name.c_str());
    if (!file_mmap_)
    {
        TSTM_NODE_LOG(TSTM_LOG_ERROR, "Open file fail! file={}", param_->file_name);
        return false;
    }

    pcap_size_ = file_mmap_->GetSize();
    pcap_begin_ = file_mmap_->GetData();
    pcap_end_ = pcap_begin_ + pcap_size_;

    if (pcap_size_ < kPcapGlobalHeaderSize)
    {
        TSTM_NODE_LOG(TSTM_LOG_ERROR, "Pcap file header size not enough! size={}", file_mmap_->GetSize());
        return false;
    }

    int64_t magic_number = (pcap_begin_[0] << 24) | (pcap_begin_[1] << 16) | (pcap_begin_[2] << 8) | pcap_begin_[3];
    switch (magic_number)
    {
    case PcapMaginNumbers::kUsBe:
        bo_ = lccl::bit::ByteOrders::kBe;
        time_precision_ = 1000;
        break;
    case PcapMaginNumbers::kUsLE:
        bo_ = lccl::bit::ByteOrders::kLe;
        time_precision_ = 1000;
        break;
    case PcapMaginNumbers::kNsBe:
        bo_ = lccl::bit::ByteOrders::kBe;
        time_precision_ = 1;
        break;
    case PcapMaginNumbers::kNsLE:
        bo_ = lccl::bit::ByteOrders::kLe;
        time_precision_ = 1;
        break;
    default:
        TSTM_NODE_LOG(TSTM_LOG_ERROR, "Unknown pcap magic number={}!", magic_number);
        return false;
    }

    pcap_reader_ = lccl::bit::Reader(pcap_begin_, bo_);

    pcap_reader_.SkipBytes(4);
    version_ = pcap_reader_.Read2Bytes() * 100 + pcap_reader_.Read2Bytes();

    pcap_reader_.SkipBytes(12);
    int64_t network_type = pcap_reader_.Read4Bytes();
    switch (network_type)
    {
    case NetworkTypes::kEthernet:
    case NetworkTypes::kCooked:
        network_type_ = static_cast<NetworkTypes>(network_type);
        break;
    default:
        TSTM_NODE_LOG(TSTM_LOG_ERROR, "Pcap network type={} not supported!", network_type);
        return false;
    }

    packet_start_sp_ = pcap_reader_.Save();
    return true;
}

void PcapFileInputNode::ReadPacket()
{
    if (pcap_reader_.CurrPtr() + kPcapPacketHeaderSize >= pcap_end_)
    {
        return;
    }

    int64_t timestamp = pcap_reader_.Read4Bytes() * kSecInNs + pcap_reader_.Read4Bytes() * time_precision_;
    size_t incl_len = static_cast<size_t>(pcap_reader_.Read4Bytes());
    size_t orig_len = static_cast<size_t>(pcap_reader_.Read4Bytes());
    if (version_ < 203)
    {
        std::swap(incl_len, orig_len);
    }

    if (pcap_reader_.CurrPtr() + incl_len >= pcap_end_)
    {
        return;
    }

    curr_packet_ = std::make_shared<Packet>();
    curr_packet_->timestamp = timestamp;

    lccl::bit::Reader pcap_data_reader(pcap_reader_.CurrPtr(), lccl::bit::ByteOrders::kBe);
    pcap_reader_.SkipBytes(incl_len);
    if (incl_len < orig_len)
    {
        TSTM_NODE_LOG(TSTM_LOG_ERROR, "packet offset={} truncation, incl_len={}, orig_len={}, drop",
            pcap_reader_.CurrSize(), incl_len, orig_len);

        curr_packet_->type = PacketTypes::kTruncation;
        return;
    }

    switch (network_type_)
    {
    case NetworkTypes::kEthernet:
    {
        if (!ReadEthernetHeader(pcap_data_reader, incl_len))
        {
            return;
        }
        break;
    }

    case NetworkTypes::kCooked:
    {
        break;
    }

    default:
        break;
    }

    switch (curr_packet_->type)
    {
    case PacketTypes::kUdp:
    {
        if (pcap_data_reader.CurrPtr() + kUdpHeaderSize >= pcap_end_)
        {
            curr_packet_->type = PacketTypes::kTruncation;
            return;
        }

        curr_packet_->src_port = static_cast<uint16_t>(pcap_data_reader.Read2Bytes());
        curr_packet_->dst_port = static_cast<uint16_t>(pcap_data_reader.Read2Bytes());

        curr_packet_->size = pcap_data_reader.Read2Bytes();
        if (curr_packet_->size < kUdpHeaderSize)
        {
            curr_packet_->type = PacketTypes::kTruncation;
            return;
        }
        curr_packet_->size -= kUdpHeaderSize;

        pcap_data_reader.SkipBytes(2);

        if (pcap_data_reader.CurrPtr() + curr_packet_->size >= pcap_end_)
        {
            curr_packet_->type = PacketTypes::kTruncation;
            return;
        }

        curr_packet_->data = pcap_data_reader.CurrPtr();
        break;
    }
    default:
        break;
    }

    if (!IsHint())
    {
        curr_packet_->type = PacketTypes::kUnsupported;
        return;
    }
}

bool PcapFileInputNode::ReadEthernetHeader(lccl::bit::Reader &reader, size_t size)
{
    reader.SkipBytes(12);
    int64_t ether_type = reader.Read2Bytes();
    switch (ether_type)
    {
    case 0x0800:    // IPv4
    {
        int64_t version = reader.ReadBits(4);
        if (4 != version)
        {
            curr_packet_->type = PacketTypes::kUnsupported;
            return false;
        }

        size_t header_length = static_cast<size_t>(reader.ReadBits(4) * 4);
        if (header_length < 20)
        {
            curr_packet_->type = PacketTypes::kTruncation;
            return false;
        }

        reader.SkipBytes(1);
        size_t total_length = static_cast<size_t>(reader.Read2Bytes());
        if ((total_length < header_length) || (total_length > size))
        {
            curr_packet_->type = PacketTypes::kTruncation;
            return false;
        }

        reader.SkipBytes(5);
        int64_t protocol = reader.ReadByte();
        switch (protocol)
        {
        case 0x11:
            curr_packet_->type = PacketTypes::kUdp;
            break;
        default:
            curr_packet_->type = PacketTypes::kUnsupported;
            return false;
        }
        reader.SkipBytes(2);

        if (lccl::skt::IpHexToStr(lccl::skt::AddrTypes::kIpv4, reader.CurrPtr(), ip_buffer_.data()))
        {
            curr_packet_->src_ip = ip_buffer_.data();
        }
        reader.SkipBytes(4);

        if (lccl::skt::IpHexToStr(lccl::skt::AddrTypes::kIpv4, reader.CurrPtr(), ip_buffer_.data()))
        {
            curr_packet_->dst_ip = ip_buffer_.data();
        }
        reader.SkipBytes(4);

        reader.SkipBytes(header_length - 20);
        break;
    }

    case 0x86DD:    // IPv6
    {
        curr_packet_->type = PacketTypes::kUnsupported;
        return false;
    }

    default:
        curr_packet_->type = PacketTypes::kUnsupported;
        return false;
    }

    return true;
}

bool PcapFileInputNode::IsHint()
{
    if (hint_init_)
    {
        return true;
    }

    if (!param_->hint_ip.empty())
    {
        if ((curr_packet_->src_ip == param_->hint_ip) || (curr_packet_->dst_ip == param_->hint_ip))
        {
            return true;
        }
    }

    if (param_->hint_port > 0)
    {
        if ((curr_packet_->src_port == param_->hint_port) || (curr_packet_->dst_port == param_->hint_port))
        {
            return true;
        }
    }

    return false;
}

TSTM_END_NAMESPACE
