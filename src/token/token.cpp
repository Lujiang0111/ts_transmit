#include "token/token.h"

TSTM_BEGIN_NAMESPACE

Token::Token() :
    data_(nullptr)
{

}

Token::Token(size_t data_size, TokenData::AllocFunc data_alloc_func, TokenData::FreeFunc data_free_func, void *data_opaque) :
    props_(static_cast<size_t>(Props::kNb), 0)
{
    data_ = TokenData::New(data_size, data_alloc_func, data_free_func, data_opaque);
}

Token::~Token()
{

}

std::shared_ptr<Token> Token::New(size_t data_size,
    TokenData::AllocFunc data_alloc_func, TokenData::FreeFunc data_free_func, void *data_opaque)
{
    static CoreAllocator<Token> allocator;

    if (0 == data_size)
    {
        return lccl::AllocateSharedProtected<Token>(allocator);
    }
    return lccl::AllocateSharedProtected<Token>(allocator, data_size, data_alloc_func, data_free_func, data_opaque);
}

std::shared_ptr<Token> Token::Reference()
{
    std::shared_ptr<Token> rhs = New(0, nullptr, nullptr, nullptr);
    rhs->data_ = data_;
    rhs->props_ = props_;
    return rhs;
}

TokenData *Token::Data()
{
    return data_.get();
}

int64_t Token::GetProperty(Props type) const
{
    return props_[static_cast<size_t>(type)];
}

void Token::SetProperty(Props type, int64_t value)
{
    props_[static_cast<size_t>(type)] = value;
}

TSTM_END_NAMESPACE
