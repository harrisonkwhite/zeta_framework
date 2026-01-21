#include <zgl/zgl_game.h>

#include <cstdio>

#ifdef ZCL_PLATFORM_WINDOWS
// @todo: These can probably be defined via the build system?
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #include <windows.h>
#endif

namespace zgl {
    constexpr const char *k_error_log_file_name_c_str = "error.log";

    void internal::ConfigErrorHandling() {
#ifndef ZCL_DEBUG
        // Intentionally using non-ZF functions here to minimize the number of possible ZCL fatal error triggers that could occur before this error handling setup is complete.

        {
            FILE *const file = freopen(k_error_log_file_name_c_str, "w", stderr);

            if (!file) {
                ZCL_FATAL(); // This one will not go to the error log file...
            }
        }

        const auto callback = []() {
            char err_msg_buf[256];
            snprintf(err_msg_buf, sizeof(err_msg_buf), "A fatal error occurred - please check \"%s\" for details.\nThe program will now exit.", k_error_log_file_name_c_str);

    #ifdef ZCL_PLATFORM_WINDOWS
            MessageBoxA(nullptr, err_msg_buf, "Fatal Error", MB_OK | MB_ICONERROR);
    #endif
        };

        zcl::FatalErrorSetCallback(callback);
#endif
    }
}
