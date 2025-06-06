#ifndef TS_TRANSMIT_INCLUDE_TS_TRANSMIT_C_H_
#define TS_TRANSMIT_INCLUDE_TS_TRANSMIT_C_H_

#include <stddef.h>
#include <stdint.h>

#if defined(_MSC_VER)
#if defined(TS_TRANSMIT_API_EXPORT)
#define TSTM_API __declspec(dllexport)
#else
#define TSTM_API __declspec(dllimport)
#endif
#else
#define TSTM_API
#endif

#ifdef __cplusplus
#define TSTM_EXTERN_C extern "C"
#else
#define TS_TRANSMIT_EXTERN_C
#endif

// log level
#define TSTM_LOG_DEBUG 0
#define TSTM_LOG_INFO 1
#define TSTM_LOG_WARN 2
#define TSTM_LOG_ERROR 3

#define TSTM_PROGRESS_DONE -1

typedef void *TransmitterHandle;

TSTM_EXTERN_C TSTM_API void SetTsTransmitLogCallback(
    void (*cb)(void *opaque, int level, const char *file_name, int file_line, const char *content, size_t len),
    void *opaque);

TSTM_EXTERN_C TSTM_API TransmitterHandle CreateTransmitterHandle(const char *json_param);
TSTM_EXTERN_C TSTM_API void DestroyTransmitterHandle(TransmitterHandle *p_hdl);

TSTM_EXTERN_C TSTM_API int TransmitterHandleGetProgress(TransmitterHandle hdl);

#endif // !TS_TRANSMIT_INCLUDE_TS_TRANSMIT_C_H_
