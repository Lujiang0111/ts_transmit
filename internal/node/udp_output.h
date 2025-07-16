#ifndef TS_TRANSMIT_INTERNAL_NOTE_UDP_OUTPUT_H_
#define TS_TRANSMIT_INTERNAL_NOTE_UDP_OUTPUT_H_

#include "lccl/socket.h"
#include "node/base_node.h"

TSTM_NAMESPACE_BEGIN

class UdpOutputNode : public BaseNode
{
public:
    struct Param
    {
        std::string dst_ip;
        uint16_t dst_port = 0;
        std::string local_ip;
        uint16_t local_port = 0;
        int ttl = 0;
    };

public:
    explicit UdpOutputNode(const std::string &id);
    virtual ~UdpOutputNode();

    virtual bool Init(const rapidjson::Value &param_val);
    virtual void Deinit();

    virtual std::shared_ptr<TokenList> Do(std::shared_ptr<TokenList> in_token_list);

private:
    enum class WorkStates
    {
        kInit = 0,
        kDeinit,
        kFail,
        kSending,
    };

private:
    bool ParseParam(const rapidjson::Value &param_val);

    void InitWorkState();
    void DeinitWorkState();
    void FailWorkState();
    void SendingWorkState();

    bool InitSocket();

private:
    std::shared_ptr<Param> param_;

    int64_t curr_time_;
    int64_t next_work_time_;
    WorkStates work_state_;

    std::shared_ptr<TokenList> buffer_token_list_;
    Token *token_;

    int fd_;
    int64_t next_send_time_;
};

TSTM_NAMESPACE_END

#endif // !TS_TRANSMIT_INTERNAL_NOTE_UDP_OUTPUT_H_
