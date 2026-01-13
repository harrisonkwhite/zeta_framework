#pragma once

namespace zcl {
    namespace detail {
        void try_breaking_into_debugger_if(const bool cond);

        [[noreturn]] void handle_assert_error(const char *const cond_cstr, const char *const func_name_cstr, const char *const file_name_cstr, const int line);

#ifdef ZF_DEBUG
    #define ZF_DEBUG_BREAK() detail::try_breaking_into_debugger_if(true)
    #define ZF_DEBUG_BREAK_IF(cond) detail::try_breaking_into_debugger_if(cond)

    #define ZF_ASSERT(cond)                                                                    \
        do {                                                                                   \
            if (!ZF_IN_CONSTEXPR()) {                                                          \
                if (!(cond)) {                                                                 \
                    zcl::detail::handle_assert_error(#cond, __FUNCTION__, __FILE__, __LINE__); \
                }                                                                              \
            }                                                                                  \
        } while (0)
#else
    #define ZF_DEBUG_BREAK() static_cast<void>(0)
    #define ZF_DEBUG_BREAK_IF(cond) static_cast<void>(0)
    #define ZF_ASSERT(cond) static_cast<void>(0)
#endif

        [[noreturn]] void handle_fatal_error(const char *const func_name_cstr, const char *const file_name_cstr, const int line, const char *const cond_cstr = nullptr);

#define ZF_FATAL() zcl::detail::handle_fatal_error(__FUNCTION__, __FILE__, __LINE__)
#define ZF_UNREACHABLE() ZF_FATAL() // @todo: This should probably have some helper message to differentiate it from normal fatal errors.

#define ZF_REQUIRE(cond)                                                                  \
    do {                                                                                  \
        if (!ZF_IN_CONSTEXPR()) {                                                         \
            if (!(cond)) {                                                                \
                zcl::detail::handle_fatal_error(__FUNCTION__, __FILE__, __LINE__, #cond); \
            }                                                                             \
        }                                                                                 \
    } while (0)
    }
}
