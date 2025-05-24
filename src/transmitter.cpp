#include "lib_log.h"
#include "transmitter.h"

TS_TRANSMIT_BEGIN_NAMESPACE

Transmitter::Transmitter()
{

}

Transmitter::~Transmitter()
{

}

bool Transmitter::Init(const std::string &json_param)
{
    if (!lccl::ParseStringToJson(param_doc_, json_param))
    {
        LIB_LOG(TS_TRANSMIT_LOG_ERROR, "Could not parse json_param={}", json_param)
        return false;
    }

    if (!lccl::GetJsonChildString(param_doc_, "id", id_))
    {
        LIB_LOG(TS_TRANSMIT_LOG_ERROR, "Could not parse id, json_param={}", json_param)
        return false;
    }

    return true;
}

void Transmitter::Deinit()
{

}

int Transmitter::GetProgress()
{
    return TS_TRANSMIT_PROGRESS_DONE;
}

TS_TRANSMIT_END_NAMESPACE
