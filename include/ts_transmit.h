#ifndef TS_TRANSMIT_INCLUDE_TS_TRANSMIT_H_
#define TS_TRANSMIT_INCLUDE_TS_TRANSMIT_H_

#include <memory>
#include "ts_transmit_c.h"

#define TSTM_BEGIN_NAMESPACE namespace tstm {
#define TSTM_END_NAMESPACE }

TSTM_BEGIN_NAMESPACE

class ITransmitter
{
public:
    virtual ~ITransmitter() = default;

    virtual int GetProgress() = 0;
};

TSTM_API std::shared_ptr<ITransmitter> CreateTransmitter(const char *json_param);

TSTM_END_NAMESPACE

#endif // !TS_TRANSMIT_INCLUDE_TS_TRANSMIT_H_
