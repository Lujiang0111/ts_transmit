#ifndef TS_TRANSMIT_INCLUDE_TS_TRANSMIT_H_
#define TS_TRANSMIT_INCLUDE_TS_TRANSMIT_H_

#include <memory>
#include "ts_transmit_c.h"

#define TS_TRANSMIT_BEGIN_NAMESPACE namespace tstm {
#define TS_TRANSMIT_END_NAMESPACE }

TS_TRANSMIT_BEGIN_NAMESPACE

class ITransmitter
{
public:
    virtual ~ITransmitter() = default;

    virtual int GetProgress() = 0;
};

TS_TRANSMIT_API std::shared_ptr<ITransmitter> CreateTransmitter(const char *json_param);

TS_TRANSMIT_END_NAMESPACE

#endif // !TS_TRANSMIT_INCLUDE_TS_TRANSMIT_H_
