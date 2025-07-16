#include "utils/lib_log.h"
#include "utils/utils.h"
#include "transmitter.h"

TSTM_NAMESPACE_BEGIN

Transmitter::Transmitter() :
    sleep_ns_(kMsInNs),
    work_thread_running_(false)
{

}

Transmitter::~Transmitter()
{
    Deinit();
}

bool Transmitter::Init(const std::string &json_param)
{
    if (!lccl::ParseStringToJson(param_doc_, json_param))
    {
        TSTM_LOG(TSTM_LOG_ERROR, "Could not parse json_param={}", json_param);
        return false;
    }

    if (!lccl::GetJsonChild(param_doc_, "id", id_))
    {
        TSTM_LOG(TSTM_LOG_ERROR, "Could not parse id, json_param={}", json_param);
        return false;
    }

    if (lccl::GetJsonChild(param_doc_, "sleep_ns", sleep_ns_))
    {
        if (sleep_ns_ < 0)
        {
            sleep_ns_ = kMsInTs;
        }
        else if (sleep_ns_ > 500 * kMsInNs)
        {
            sleep_ns_ = 500 * kMsInNs;
        }
    }
    else
    {
        sleep_ns_ = kMsInTs;
    }

    if (!lccl::IsJsonChildValid(param_doc_, "nodes", rapidjson::kArrayType))
    {
        TSTM_LOG(TSTM_LOG_ERROR, "Could not parse nodes, json_param={}", json_param);
        return false;
    }

    const rapidjson::Value &nodes_val = param_doc_["nodes"];
    for (rapidjson::SizeType index = 0; index < nodes_val.Size(); ++index)
    {
        const rapidjson::Value &node_val = nodes_val[index];
        std::shared_ptr<BaseNode> node = CreateNode(id_, node_val);
        if (!node)
        {
            return false;
        }
        nodes_.push_back(node);
    }

    work_thread_running_ = true;
    work_thread_ = std::thread(&Transmitter::WorkThread, this);
    return true;
}

void Transmitter::Deinit()
{
    work_thread_running_ = false;
    if (work_thread_.joinable())
    {
        work_thread_.join();
    }

    nodes_.clear();
}

int Transmitter::GetProgress()
{
    return TSTM_PROGRESS_DONE;
}

void Transmitter::WorkThread()
{
#if defined(_MSC_VER)
#else
    pthread_t curr_thread = pthread_self();

    // 设置线程名
    std::string curr_thread_name = fmt::format("tm_{}", id_);
    pthread_setname_np(curr_thread, curr_thread_name.c_str());
#endif

    while (work_thread_running_)
    {
        bool busy = false;
        std::shared_ptr<TokenList> token_list = nullptr;

        for (auto &&node : nodes_)
        {
            node->SetBusy(false);
            token_list = node->Do(token_list);
            if ((!busy) &&
                ((node->GetBusy()) || ((token_list) && (token_list->Size() > 0))))
            {
                busy = true;
            }
        }

        if (!busy)
        {
            ExactSleep(sleep_ns_);
        }
    }
}

TSTM_NAMESPACE_END
