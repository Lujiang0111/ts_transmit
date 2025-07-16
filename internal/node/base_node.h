#ifndef TS_TRANSMIT_INTERNAL_NOTE_BASE_NODE_H_
#define TS_TRANSMIT_INTERNAL_NOTE_BASE_NODE_H_

#include <string>
#include "lccl/oss/json.h"
#include "token/token_list.h"
#include "utils/lib_log.h"
#include "utils/utils.h"

TSTM_NAMESPACE_BEGIN

class BaseNode
{
public:
    BaseNode() = delete;
    BaseNode(const BaseNode &) = delete;
    BaseNode &operator=(const BaseNode &) = delete;

    BaseNode(const std::string &name, const std::string &id);
    virtual ~BaseNode();

    virtual bool Init(const rapidjson::Value &param_val) = 0;
    virtual void Deinit() = 0;

    virtual std::shared_ptr<TokenList> Do(std::shared_ptr<TokenList> in_token_list) = 0;

    const std::string &GetName() const;
    const std::string &GetId() const;

    bool GetBusy() const;
    void SetBusy(bool busy);

private:
    std::string name_;
    std::string id_;
    bool busy_;
};

std::shared_ptr<BaseNode> CreateNode(const std::string &id, const rapidjson::Value &node_val);

TSTM_NAMESPACE_END

#define TSTM_NODE_LOG(level, fmt, ...) tstm::LibLogFmt(level, __FILE__, __LINE__, "node[{}, {}], "#fmt, GetName(), GetId(), ##__VA_ARGS__)

#endif // !TS_TRANSMIT_INTERNAL_NOTE_BASE_NODE_H_
