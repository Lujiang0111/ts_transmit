#ifndef TS_TRANSMIT_INTERNAL_TRANSMITTER_H_
#define TS_TRANSMIT_INTERNAL_TRANSMITTER_H_

#include <string>
#include "lccl/oss/json.h"
#include "ts_transmit.h"

TS_TRANSMIT_BEGIN_NAMESPACE

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

private:
    std::string id_;
    rapidjson::Document param_doc_;

};

TS_TRANSMIT_END_NAMESPACE

#endif // !TS_TRANSMIT_INTERNAL_TRANSMITTER_H_
