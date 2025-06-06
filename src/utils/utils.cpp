#if defined(_MSC_VER)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "utils/utils.h"

TSTM_BEGIN_NAMESPACE

Utils::Utils() :
    init_time_(std::chrono::steady_clock::now())
{

}

Utils::~Utils()
{

}

int64_t Utils::GetSteadyTime()
{
    std::chrono::steady_clock::time_point curr_time = std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration duration = curr_time - init_time_;
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

void ExactSleep(int64_t sleep_ns)
{
#if defined(_MSC_VER)
    HANDLE timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
    if (!timer) return;

    LARGE_INTEGER due_time = { 0 };
    due_time.QuadPart = -sleep_ns / 100;

    SetWaitableTimer(timer, &due_time, 0, nullptr, nullptr, FALSE);
    WaitForSingleObject(timer, INFINITE);

    CloseHandle(timer);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_nsec += sleep_ns;

    if (ts.tv_nsec >= kSecInNs)
    {
        ts.tv_sec += 1;
        ts.tv_nsec -= kSecInNs;
    }

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, nullptr);
#endif
}

TSTM_END_NAMESPACE
