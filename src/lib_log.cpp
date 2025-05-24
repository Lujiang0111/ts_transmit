#include "lib_log.h"

TS_TRANSMIT_BEGIN_NAMESPACE

static void DefaultLogCallback(void *opaque, int level, const char *file_name, int file_line, const char *content, size_t len)
{
    lccl::log::Levels lccl_level = lccl::log::Levels::kDebug;
    switch (level)
    {
    case TS_TRANSMIT_LOG_DEBUG:
        lccl_level = lccl::log::Levels::kDebug;
        break;
    case TS_TRANSMIT_LOG_INFO:
        lccl_level = lccl::log::Levels::kInfo;
        break;
    case TS_TRANSMIT_LOG_WARN:
        lccl_level = lccl::log::Levels::kWarn;
        break;
    case TS_TRANSMIT_LOG_ERROR:
        lccl_level = lccl::log::Levels::kError;
        break;
    default:
        return;
    }

    fmt::println("[ts_transmit]: {} {}:{} {:.{}}",
        lccl::log::LevelToString(lccl_level),
        file_name, file_line,
        content, len);
}

static void (*lib_log_cb)(void *opaque, int level, const char *file_name, int file_line, const char *content, size_t len) = DefaultLogCallback;
static void *lib_log_opaque = nullptr;

void SetTsTransmitLogCallback(
    void (*cb)(void *opaque, int level, const char *file_name, int file_line, const char *content, size_t len),
    void *opaque)
{
    lib_log_cb = (cb) ? cb : DefaultLogCallback;
    lib_log_opaque = opaque;
}

void LibLogContent(int level, const char *file_name, int file_line, const char *content, size_t len)
{
    lib_log_cb(lib_log_opaque, level, file_name, file_line, content, len);
}

TS_TRANSMIT_END_NAMESPACE
