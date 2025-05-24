#ifndef TS_TRANSMIT_INCLUDE_TS_TRANSMIT_C_H_
#define TS_TRANSMIT_INCLUDE_TS_TRANSMIT_C_H_

#include <stddef.h>
#include <stdint.h>

#if defined(_MSC_VER)
#if defined(TS_TRANSMIT_API_EXPORT)
#define TS_TRANSMIT_API __declspec(dllexport)
#else
#define TS_TRANSMIT_API __declspec(dllimport)
#endif
#else
#define TS_TRANSMIT_API
#endif

#ifdef __cplusplus
#define TS_TRANSMIT_EXTERN_C extern "C"
#else
#define TS_TRANSMIT_EXTERN_C
#endif

// log level
#define TS_TRANSMIT_LOG_DEBUG 0
#define TS_TRANSMIT_LOG_INFO 1
#define TS_TRANSMIT_LOG_WARN 2
#define TS_TRANSMIT_LOG_ERROR 3

#define TS_TRANSMIT_PROGRESS_DONE -1

typedef void *TransmitterHandle;

TS_TRANSMIT_EXTERN_C TS_TRANSMIT_API void SetTsTransmitLogCallback(
    void (*cb)(void *opaque, int level, const char *file_name, int file_line, const char *content, size_t len),
    void *opaque);

TS_TRANSMIT_EXTERN_C TS_TRANSMIT_API TransmitterHandle CreateTransmitterHandle(const char *json_param);
TS_TRANSMIT_EXTERN_C TS_TRANSMIT_API void DestroyTransmitterHandle(TransmitterHandle *p_hdl);

TS_TRANSMIT_EXTERN_C TS_TRANSMIT_API int TransmitterHandleGetProgress(TransmitterHandle hdl);

#endif // !TS_TRANSMIT_INCLUDE_TS_TRANSMIT_C_H_
