#pragma once

#include <zcl.h>

namespace zf {
    struct s_input_state;

    namespace platform {
        // Note that the window is not shown by default, you have to manually do this.
        void ZF_Platform_StartupModule(const s_v2_i init_window_size);

        void ZF_Platform_ShutdownModule();

        // Gives the time in seconds since the platform module was started.
        t_f64 Time();

        // Also updates the given input state based on OS events.
        void PollOSEvents(s_input_state *const input_state);

        void *NativeDisplayHandle();

        void *NativeWindowHandle();

        void ShowWindow();

        // Returns whether a window close has been requested.
        t_b8 ShouldWindowClose();

        void SetWindowTitle(const s_str_rdonly title, s_arena *const temp_arena);

        // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
        void SetWindowSize(const s_v2_i size);

        // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave it as -1.
        void SetWindowSizeLimits(const t_i32 min_width, const t_i32 min_height, const t_i32 max_width, const t_i32 max_height);

        void SetWindowResizability(const t_b8 resizable);

        s_v2_i WindowFramebufferSizeCache();

        t_b8 IsFullscreen();

        void SetFullscreen(const t_b8 active);

        inline void ToggleFullscreen() {
            SetFullscreen(!IsFullscreen());
        }

        // Returns the size in pixels of whichever monitor the window most resides in.
        s_v2_i CalcMonitorSizeInPixels();

        // Returns the size (accounting for DPI scaling) of whichever monitor the window most resides in.
        s_v2_i CalcMonitorSizeLogical();

        void SetCursorVisibility(const t_b8 visible);
    }
}
