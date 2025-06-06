#ifndef TS_TRANSMIT_INTERNAL_TOKEN_TOKEN_H_
#define TS_TRANSMIT_INTERNAL_TOKEN_TOKEN_H_

#include <memory>
#include <unordered_map>
#include "ts_transmit.h"
#include "token/token_data.h"
#include "utils/alloc_core.h"

TSTM_BEGIN_NAMESPACE

class Token
{
public:
    enum class Props
    {
        kProgress = 0,
        kTransTime,
        kNb,
    };

public:
    Token(const Token &) = delete;
    Token &operator=(const Token &) = delete;

    virtual ~Token();

    static std::shared_ptr<Token> New(size_t data_size,
        TokenData::AllocFunc data_alloc_func = nullptr,
        TokenData::FreeFunc data_free_func = nullptr,
        void *data_opaque = nullptr);
    std::shared_ptr<Token> Reference();

    TokenData *Data();

    int64_t GetProperty(Props type) const;
    void SetProperty(Props type, int64_t value);

protected:
    Token();
    Token(size_t data_size, TokenData::AllocFunc data_alloc_func, TokenData::FreeFunc data_free_func, void *data_opaque);

private:
    std::shared_ptr<TokenData> data_;
    std::vector<int64_t, CoreAllocator<int64_t>> props_;
};

TSTM_END_NAMESPACE

#endif // !TS_TRANSMIT_INTERNAL_PACKET_PACKET_H_
