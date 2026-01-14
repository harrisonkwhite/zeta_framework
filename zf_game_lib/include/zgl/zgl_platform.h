#pragma once

#include <zcl.h>

namespace zgl {
    namespace input {
        struct t_state;
    }

    namespace platform {
        // Note that the window is not shown by default, you have to manually do this.
        void ModuleStartup(const zcl::t_v2_i init_window_size);

        void ModuleShutdown();

        // Gives the time in seconds since the platform module was started.
        zcl::t_f64 GetTime();

        // Also updates the given input state based on events.
        void PollEvents(input::t_state *const input_state);

        void *DisplayGetNativeHandle();

        void *WindowGetNativeHandle();

        void WindowShow();

        void WindowRequestClose();
        zcl::t_b8 WindowCheckCloseRequested();

        void WindowSetTitle(const zcl::t_str_rdonly title, zcl::t_arena *const temp_arena);

        // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
        void WindowSetSize(const zcl::t_v2_i size);

        // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave it as -1.
        void WindowSetSizeLimits(const zcl::t_i32 min_width, const zcl::t_i32 min_height, const zcl::t_i32 max_width, const zcl::t_i32 max_height);

        void WindowSetResizable(const zcl::t_b8 resizable);

        zcl::t_v2_i WindowGetFramebufferSizeCache();

        zcl::t_b8 WindowCheckFullscreen();

        void WindowSetFullscreen(const zcl::t_b8 active);

        inline void WindowToggleFullscreen() {
            WindowSetFullscreen(!WindowCheckFullscreen());
        }

        // Calculates the size in pixels of whichever monitor the window most resides in.
        zcl::t_v2_i MonitorCalcSizePixels();

        // Calculates the size (accounting for DPI scaling) of whichever monitor the window most resides in.
        zcl::t_v2_i MonitorCalcSizeLogical();

        void CursorSetVisible(const zcl::t_b8 visible);
    }
}
