#include "transmitter.h"

TSTM_NAMESPACE_BEGIN

std::shared_ptr<ITransmitter> CreateTransmitter(const char *json_param)
{
    std::shared_ptr<Transmitter> transmitter = std::make_shared<Transmitter>();
    if (!transmitter->Init((json_param) ? json_param : ""))
    {
        return nullptr;
    }

    return transmitter;
}

TSTM_NAMESPACE_END

TransmitterHandle CreateTransmitterHandle(const char *json_param)
{
    tstm::Transmitter *transmitter = new tstm::Transmitter();
    if (!transmitter->Init((json_param) ? json_param : ""))
    {
        return nullptr;
    }

    return transmitter;
}

void DestroyTransmitterHandle(TransmitterHandle *p_hdl)
{
    if ((!p_hdl) || (!(*p_hdl)))
    {
        return;
    }

    tstm::Transmitter *transmitter = static_cast<tstm::Transmitter *>(*p_hdl);
    delete transmitter;
    return;
}

int TransmitterHandleGetProgress(TransmitterHandle hdl)
{
    tstm::Transmitter *transmitter = static_cast<tstm::Transmitter *>(hdl);
    return transmitter->GetProgress();
}
