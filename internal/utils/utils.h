#ifndef TS_TRANSMIT_INTERNAL_UTILS_UTILS_H_
#define TS_TRANSMIT_INTERNAL_UTILS_UTILS_H_

#include <chrono>
#include "ts_transmit.h"

TSTM_BEGIN_NAMESPACE

constexpr int64_t kUsInNs = 1000LL;
constexpr int64_t kMsInNs = 1000LL * kUsInNs;
constexpr int64_t kSecInNs = 1000LL * kMsInNs;

constexpr int64_t kMsInTs = 90LL;
constexpr int64_t kSecInTs = 1000LL * kMsInTs;

class Utils
{

public:
    virtual ~Utils();

    inline static Utils *Instance()
    {
        static Utils instance;
        return &instance;
    }

    int64_t GetSteadyTime();

private:
    Utils();

private:
    std::chrono::steady_clock::time_point init_time_;
};

// 高精度sleep, sleep_ns需要小于kSecInNs
void ExactSleep(int64_t sleep_ns);

TSTM_END_NAMESPACE

#endif // !TS_TRANSMIT_INTERNAL_UTILS_UTILS_H_
