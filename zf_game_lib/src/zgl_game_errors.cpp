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
    const zcl::t_str_rdonly g_error_log_file_name = ZCL_STR_LITERAL("error.log");

    void internal::ConfigErrorHandling(zcl::t_arena *const temp_arena) {
#ifdef ZCL_DEBUG
        zcl::t_file_stream std_err = zcl::FileStreamCreateStdError();

        if (!zcl::FileReopen(&std_err, g_error_log_file_name, zcl::ek_file_access_mode_write, temp_arena, nullptr)) {
            ZCL_FATAL();
        }

        // @todo: Embed file name into error message.

        const auto callback = []() {
    #ifdef ZCL_PLATFORM_WINDOWS
            MessageBoxA(nullptr, "A fatal error occurred - please check \"error.log\" for details.\nThe program will now exit.", "Fatal Error", MB_OK | MB_ICONERROR);
    #endif
        };

        zcl::FatalErrorSetCallback(callback);
#endif
    }
}
