#ifndef TS_TRANSMIT_INTERNAL_TOKEN_TOKEN_LIST_H_
#define TS_TRANSMIT_INTERNAL_TOKEN_TOKEN_LIST_H_

#include <deque>
#include <functional>
#include "token/token.h"

TSTM_BEGIN_NAMESPACE

class TokenList
{
public:
    TokenList(const TokenList &) = delete;
    TokenList &operator=(const TokenList &) = delete;

    virtual ~TokenList();

    static std::shared_ptr<TokenList> New();
    std::shared_ptr<TokenList> Reference();

    void Push(std::shared_ptr<Token> token);
    std::shared_ptr<Token> Pop();

    void Combine(TokenList *rhs);
    void Swap(TokenList *rhs);
    void Clear();

    Token *Top();

    template<typename Callback, typename... Args>
    void ForEach(Callback callback, Args&&... args) const
    {
        for (auto &&token : tokens_)
        {
            callback(token.get(), std::forward<Args>(args)...);
        }
    }

    size_t Size() const;

protected:
    TokenList();

private:
    std::deque<std::shared_ptr<Token>, CoreAllocator<std::shared_ptr<Token>>> tokens_;
    size_t size_;
};

TSTM_END_NAMESPACE

#endif // !TS_TRANSMIT_INTERNAL_TOKEN_TOKEN_LIST_H_
