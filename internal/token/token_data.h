#ifndef TS_TRANSMIT_INTERNAL_TOKEN_TOKEN_DATA_H_
#define TS_TRANSMIT_INTERNAL_TOKEN_TOKEN_DATA_H_

#include <functional>
#include "ts_transmit.h"

TSTM_NAMESPACE_BEGIN

class TokenData
{
public:
    using AllocFunc = std::function<uint8_t *(void *, size_t)>;
    using FreeFunc = std::function<uint8_t *(void *, uint8_t *)>;

public:
    TokenData() = delete;
    TokenData(const TokenData &) = delete;
    TokenData &operator=(const TokenData &) = delete;

    virtual ~TokenData();

    static std::shared_ptr<TokenData> New(size_t size, AllocFunc alloc_func, FreeFunc free_func, void *opaque);

    uint8_t *Buf();
    size_t Size() const;

protected:
    TokenData(size_t size, AllocFunc alloc_func, FreeFunc free_func, void *opaque);

private:
    AllocFunc alloc_func_;
    FreeFunc free_func_;
    void *opaque_;

    size_t size_;
    uint8_t *buf_;
};

TSTM_NAMESPACE_END

#endif // !TS_TRANSMIT_INTERNAL_TOKEN_TOKEN_DATA_H_
