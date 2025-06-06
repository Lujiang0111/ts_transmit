#ifndef TS_TRANSMIT_INTERNAL_UTILS_LIB_LOG_H_
#define TS_TRANSMIT_INTERNAL_UTILS_LIB_LOG_H_

#include "lccl/log.h"
#include "ts_transmit.h"

TSTM_BEGIN_NAMESPACE

void LibLogContent(int level, const char *file_name, int file_line, const char *content, size_t len);

template<typename... Args>
inline void LibLogFmt(int level, const char *file_name, int file_line, fmt::format_string<Args...> fmt, Args &&... args)
{
    std::string content = fmt::vformat(fmt, fmt::make_format_args(args...));
    LibLogContent(level, file_name, file_line, content.c_str(), content.length());
}

TSTM_END_NAMESPACE

#define TSTM_LOG(level, fmt, ...) tstm::LibLogFmt(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define TSTM_ID_LOG(level, fmt, ...) tstm::LibLogFmt(level, __FILE__, __LINE__, "[{}], "#fmt, id, ##__VA_ARGS__)

#endif // !TS_TRANSMIT_INTERNAL_UTILS_LIB_LOG_H_
