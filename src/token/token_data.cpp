#include <cassert>
#include "token/token_data.h"
#include "utils/alloc_core.h"

TSTM_BEGIN_NAMESPACE

TokenData::TokenData(size_t size, AllocFunc alloc_func, FreeFunc free_func, void *opaque) :
    alloc_func_(alloc_func),
    free_func_(free_func),
    opaque_(opaque),
    size_(size),
    buf_(nullptr)
{
    assert(size_ > 0);
    if (!alloc_func)
    {
        buf_ = static_cast<uint8_t *>(CoreMalloc(size_ * sizeof(uint8_t)));
    }
    else
    {
        buf_ = alloc_func(opaque_, size_);
    }
}

TokenData::~TokenData()
{
    if (!free_func_)
    {
        CoreFree(buf_);
    }
    else
    {
        free_func_(opaque_, buf_);
    }
}

std::shared_ptr<TokenData> TokenData::New(size_t size, AllocFunc alloc_func, FreeFunc free_func, void *opaque)
{
    static CoreAllocator<TokenData> allocator;
    return lccl::AllocateSharedProtected<TokenData>(allocator, size, alloc_func, free_func, opaque);
}

uint8_t *TokenData::Buf()
{
    return buf_;
}

size_t TokenData::Size() const
{
    return size_;
}

TSTM_END_NAMESPACE
