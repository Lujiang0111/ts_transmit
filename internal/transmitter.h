#ifndef TS_TRANSMIT_INTERNAL_TRANSMITTER_H_
#define TS_TRANSMIT_INTERNAL_TRANSMITTER_H_

#include <condition_variable>
#include <string>
#include <thread>
#include <vector>
#include "ts_transmit.h"
#include "node/base_node.h"

TSTM_BEGIN_NAMESPACE

class Transmitter : public ITransmitter
{
public:
    Transmitter(const Transmitter &) = delete;
    Transmitter &operator=(const Transmitter &) = delete;

    Transmitter();
    virtual ~Transmitter();

    bool Init(const std::string &json_param);
    void Deinit();

    virtual int GetProgress();

    void WorkThread();

private:
    std::string id_;
    int64_t sleep_ns_;
    rapidjson::Document param_doc_;

    std::vector<std::shared_ptr<BaseNode>> nodes_;

    // 线程
    std::thread work_thread_;
    bool work_thread_running_;
};

TSTM_END_NAMESPACE

#endif // !TS_TRANSMIT_INTERNAL_TRANSMITTER_H_
