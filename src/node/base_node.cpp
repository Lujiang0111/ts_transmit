#include "node/pcap_file_input.h"
#include "node/udp_output.h"
#include "utils/lib_log.h"

TSTM_BEGIN_NAMESPACE

BaseNode::BaseNode(const std::string &name, const std::string &id) :
    name_(name),
    id_(id),
    busy_(false)
{

}

BaseNode::~BaseNode()
{

}

const std::string &BaseNode::GetName() const
{
    return name_;
}

const std::string &BaseNode::GetId() const
{
    return id_;
}

bool BaseNode::GetBusy() const
{
    return busy_;
}

void BaseNode::SetBusy(bool busy)
{
    busy_ = busy;
}

std::shared_ptr<BaseNode> CreateNode(const std::string &id, const rapidjson::Value &node_val)
{
    std::string node_name;
    if (!lccl::GetJsonChild(node_val, "name", node_name))
    {
        TSTM_ID_LOG(TSTM_LOG_ERROR, id, "Node name not found!");
        return nullptr;
    }

    std::shared_ptr<BaseNode> node = nullptr;
    if ("pcap_file_input" == node_name)
    {
        node = std::make_shared<PcapFileInputNode>(id);
    }
    else if ("udp_output" == node_name)
    {
        node = std::make_shared<UdpOutputNode>(id);
    }

    if (!node)
    {
        TSTM_ID_LOG(TSTM_LOG_ERROR, id, "node name={} not found!", node_name);
        return nullptr;
    }

    if (lccl::IsJsonChildValid(node_val, "param", rapidjson::kObjectType))
    {
        if (!node->Init(node_val["param"]))
        {
            return nullptr;
        }
    }
    else
    {
        rapidjson::Value empty_json(rapidjson::kObjectType);
        if (!node->Init(empty_json))
        {
            return nullptr;
        }
    }

    return node;
}

TSTM_END_NAMESPACE
