#pragma once

#include <cstdio>
#include <zc/zc_basic.h>

#define ZF_LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define ZF_LOG_WARNING(fmt, ...) fprintf(stderr, "Warning: " fmt "\n", ##__VA_ARGS__)
#define ZF_LOG_ERROR(fmt, ...) fprintf(stderr, "Error: " fmt "\n", ##__VA_ARGS__)
#define ZF_LOG_ERROR_SPECIAL(prefix, fmt, ...) fprintf(stderr, prefix " Error: "); \
    fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define ZF_LOG_SUCCESS(fmt, ...) printf("Success: " fmt "\n", ##__VA_ARGS__)

#define ZF_REPORT_FAILURE() zf::ReportFailure(__FUNCTION__, __FILE__, __LINE__)
#define ZF_REPORT_FAILURE_MSG(msg) zf::ReportFailure(__FUNCTION__, __FILE__, __LINE__, msg)

#ifdef ZF_DEBUG
    #define ZF_ASSERT(condition) \
        do { \
            if (!(condition)) { \
                zf::HandleAssertFailure(#condition, __FUNCTION__, __FILE__, __LINE__); \
            } \
        } while(0)

    #define ZF_ASSERT_MSG(condition, msg) \
        do { \
            if (!(condition)) { \
                zf::HandleAssertFailure(#condition, __FUNCTION__, __FILE__, __LINE__, msg); \
            } \
        } while(0)
#else
    #define ZF_ASSERT(condition) static_cast<void>(0)
    #define ZF_ASSERT_MSG(condition, msg) static_cast<void>(0)
#endif

namespace zf {
    void ConfigErrorOutput();
    void ReportFailure(const char* const func, const char* const file, const t_s32 line, const char* const msg = nullptr);
    void HandleAssertFailure(const char* const condition, const char* const func, const char* const file, const t_s32 line, const char* const msg = nullptr); // @todo: This feels awkward here, might want to move somewhere else? But where?
}
