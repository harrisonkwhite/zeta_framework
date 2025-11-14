#pragma once

#include <cstdio>
#include <zc/zc_basic.h>

#define ZF_LOG(format, ...) printf(format "\n", ##__VA_ARGS__)
#define ZF_LOG_WARNING(format, ...) fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_YELLOW "Warning: " ZF_ANSI_RESET format "\n", ##__VA_ARGS__)
#define ZF_LOG_ERROR(format, ...) fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_RED "Error: " ZF_ANSI_RESET format "\n", ##__VA_ARGS__)
#define ZF_LOG_ERROR_SPECIAL(prefix, format, ...) fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_BRED prefix " Error: " ZF_ANSI_RESET); \
    fprintf(stderr, format "\n", ##__VA_ARGS__)
#define ZF_LOG_SUCCESS(format, ...) fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_GREEN "Success: " ZF_ANSI_RESET format "\n", ##__VA_ARGS__)

#define ZF_FAILURE_DUMP() zf::HandleFailureDump(__FUNCTION__, __FILE__, __LINE__)
#define ZF_FAILURE_DUMP_MSG(msg) zf::HandleFailureDump(__FUNCTION__, __FILE__, __LINE__, msg)

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
    void HandleFailureDump(const char* const func, const char* const file, const t_s32 line, const char* const msg = nullptr);
    void HandleAssertFailure(const char* const condition, const char* const func, const char* const file, const t_s32 line, const char* const msg = nullptr); // @todo: This feels awkward here, might want to move somewhere else? But where?
}
