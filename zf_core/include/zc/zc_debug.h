#pragma once

#include <cstdio>
#include <zc/zc_basic.h>

namespace zf {
    // @todo: Need a custom implementation of these.
#define ZF_LOG(fmt, ...) static_cast<void>(0)
#define ZF_LOG_WARNING(fmt, ...) static_cast<void>(0)
#define ZF_LOG_ERROR(fmt, ...) static_cast<void>(0)
#define ZF_LOG_ERROR_SPECIAL(prefix, fmt, ...) static_cast<void>(0)
#define ZF_LOG_SUCCESS(fmt, ...) static_cast<void>(0)

#if 0
#define ZF_LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define ZF_LOG_WARNING(fmt, ...) fprintf(stderr, "Warning: " fmt "\n", ##__VA_ARGS__)
#define ZF_LOG_ERROR(fmt, ...) fprintf(stderr, "Error: " fmt "\n", ##__VA_ARGS__)
#define ZF_LOG_SUCCESS(fmt, ...) printf("Success: " fmt "\n", ##__VA_ARGS__)

#define ZF_LOG_ERROR_SPECIAL(prefix, fmt, ...) \
    do { \
        fprintf(stderr, prefix " Error: "); \
        fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
    } while(0)
#endif

    void ReportFailure(const char* const func_name_raw, const char* const file_name_raw, const t_s32 line, const char* const msg_raw = nullptr);

#define ZF_REPORT_FAILURE() zf::ReportFailure(__FUNCTION__, __FILE__, __LINE__)
#define ZF_REPORT_FAILURE_MSG(msg) zf::ReportFailure(__FUNCTION__, __FILE__, __LINE__, msg)

    void HandleAssertFailure(const char* const cond_raw, const char* const func_name_raw, const char* const file_name_raw, const t_s32 line, const char* const msg_raw = nullptr);

#ifdef ZF_DEBUG
    #define ZF_ASSERT(cond) \
        do { \
            if (!ZF_IN_CONSTEXPR() && !(cond)) { \
                zf::HandleAssertFailure(#cond, __FUNCTION__, __FILE__, __LINE__); \
            } \
        } while(0)

    #define ZF_ASSERT_MSG(cond, msg) \
        do { \
            if (!ZF_IN_CONSTEXPR() && !(cond)) { \
                zf::HandleAssertFailure(#cond, __FUNCTION__, __FILE__, __LINE__, msg); \
            } \
        } while(0)
#else
    #define ZF_ASSERT(cond) static_cast<void>(0)
    #define ZF_ASSERT_MSG(cond, msg) static_cast<void>(0)
#endif

    void ShowErrorBox(const char* const title_raw, const char* const contents_raw);
}
