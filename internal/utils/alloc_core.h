#ifndef TS_TRANSMIT_INTERNAL_UTILS_ALLOC_CORE_H_
#define TS_TRANSMIT_INTERNAL_UTILS_ALLOC_CORE_H_

#include "jemalloc.h"
#include "lccl/utils/class_utils.h"
#include "ts_transmit.h"

TSTM_BEGIN_NAMESPACE

inline void *CoreMalloc(size_t size)
{
#if defined(TS_TRANSMIT_USE_JEMALLOC)
    return je_malloc(size);
#else
    return malloc(size);
#endif
}

inline void CoreFree(void *p)
{
#if defined(TS_TRANSMIT_USE_JEMALLOC)
    return je_free(p);
#else
    return free(p);
#endif
}

template <typename T>
class CoreAllocator
{
public:
    using value_type = T;

    template <typename U>
    struct rebind
    {
        using other = CoreAllocator<U>;
    };

    CoreAllocator() noexcept = default;
    CoreAllocator(const CoreAllocator &) noexcept = default;

    template <typename U>
    CoreAllocator(const CoreAllocator<U> &) noexcept {}

    T *allocate(std::size_t n) noexcept
    {
        return static_cast<T *>(CoreMalloc(n * sizeof(T)));
    }

    void deallocate(T *p, std::size_t n) noexcept
    {
        return CoreFree(p);
    }
};

template <typename T, typename U>
bool operator==(const CoreAllocator<T> &, const CoreAllocator<U> &) noexcept
{
    return true;
}

template <typename T, typename U>
bool operator!=(const CoreAllocator<T> &, const CoreAllocator<U> &) noexcept
{
    return false;
}

TSTM_END_NAMESPACE

#endif // !TS_TRANSMIT_INTERNAL_UTILS_ALLOC_CORE_H_
