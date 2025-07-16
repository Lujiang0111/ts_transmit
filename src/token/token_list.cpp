#include "token/token_list.h"
#include "utils/alloc_core.h"

TSTM_NAMESPACE_BEGIN

TokenList::TokenList() :
    size_(0)
{

}

TokenList::~TokenList()
{

}

std::shared_ptr<TokenList> TokenList::New()
{
    static CoreAllocator<TokenList> allocator;
    return lccl::AllocateSharedProtected<TokenList>(allocator);
}

std::shared_ptr<TokenList> TokenList::Reference()
{
    std::shared_ptr<TokenList> rhs = New();

    auto &&rhs_tokens_ = rhs->tokens_;
    ForEach([&rhs_tokens_](Token *token) {
        rhs_tokens_.push_back(token->Reference());
        });

    rhs->size_ = size_;
    return rhs;
}

void TokenList::Push(std::shared_ptr<Token> token)
{
    tokens_.push_back(token);
    ++size_;
}

std::shared_ptr<Token> TokenList::Pop()
{
    if (0 == size_)
    {
        return nullptr;
    }

    std::shared_ptr<Token> token = tokens_.front();
    tokens_.pop_front();
    --size_;
    return token;
}

void TokenList::Combine(TokenList *rhs)
{
    tokens_.insert(tokens_.end(),
        std::make_move_iterator(rhs->tokens_.begin()),
        std::make_move_iterator(rhs->tokens_.end()));
    size_ += rhs->size_;
    rhs->Clear();
}

void TokenList::Swap(TokenList *rhs)
{
    tokens_.swap(rhs->tokens_);
    std::swap(size_, rhs->size_);
}

void TokenList::Clear()
{
    tokens_.clear();
    size_ = 0;
}

Token *TokenList::Top()
{
    if (0 == size_)
    {
        return nullptr;
    }

    return tokens_.front().get();
}

size_t TokenList::Size() const
{
    return size_;
}

TSTM_NAMESPACE_END
