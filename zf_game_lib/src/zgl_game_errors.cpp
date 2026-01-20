#include <zgl/zgl_game.h>

#include <cstdio>

#ifdef ZCL_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #include <windows.h>
    #include <dbghelp.h>
#endif

#include <cstdlib>

namespace zgl {
    void internal::ConfigErrorHandling() {
#ifndef ZCL_DEBUG
        FILE *const file = freopen("error.log", "w", stderr);

        if (!file) {
            ZCL_FATAL();
        }

        const auto callback = []() {
    #ifdef ZCL_PLATFORM_WINDOWS
            MessageBoxA(nullptr, "A fatal error occurred - please check \"error.log\" for details.\nThe program will now exit.", "Fatal Error", MB_OK | MB_ICONERROR);
    #endif
        };

        zcl::FatalErrorSetCallback(callback);
#endif
    }
}
