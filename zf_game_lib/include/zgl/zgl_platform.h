#pragma once

#include <zcl.h>

namespace zgl {
    // @todo: Might be useful to make constness significant here. For example, rendering might want to just READ whether fullscreen is active or not.
    // Otherwise you'd have to create a bunch of wrappers over frame context.

    struct t_input_state;

    struct t_platform;

    // Gives the time in seconds since the platform module was started.
    zcl::t_f64 GetTime(t_platform *const platform);

    void *DisplayGetNativeHandle(t_platform *const platform);

    void *WindowGetNativeHandle(t_platform *const platform);

    void WindowRequestClose(t_platform *const platform);

    // Note that a window close request can also be triggered during OS event polling, not just by you.
    zcl::t_b8 WindowCheckCloseRequested(t_platform *const platform);

    void WindowSetTitle(t_platform *const platform, const zcl::t_str_rdonly title, zcl::t_arena *const temp_arena);

    // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
    void WindowSetSize(t_platform *const platform, const zcl::t_v2_i size);

    // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave it as -1.
    void WindowSetSizeLimits(t_platform *const platform, const zcl::t_i32 min_width, const zcl::t_i32 min_height, const zcl::t_i32 max_width, const zcl::t_i32 max_height);

    void WindowSetResizable(t_platform *const platform, const zcl::t_b8 resizable);

    zcl::t_v2_i WindowGetFramebufferSizeCache(t_platform *const platform);

    zcl::t_b8 WindowCheckFullscreen(t_platform *const platform);

    void WindowSetFullscreen(t_platform *const platform, const zcl::t_b8 active);

    inline void WindowToggleFullscreen(t_platform *const platform) {
        WindowSetFullscreen(platform, !WindowCheckFullscreen(platform));
    }

    // Calculates the size in pixels of whichever monitor the window most resides in.
    zcl::t_v2_i MonitorCalcSizePixels(t_platform *const platform);

    // Calculates the size (accounting for DPI scaling) of whichever monitor the window most resides in.
    zcl::t_v2_i MonitorCalcSizeLogical(t_platform *const platform);

    void CursorSetVisible(t_platform *const platform, const zcl::t_b8 visible);

    namespace detail {
        // Note that the window is not shown by default, you have to manually do this.
        t_platform *PlatformStartup(const zcl::t_v2_i init_window_size, t_input_state *const input_state, zcl::t_arena *const arena);

        void PlatformShutdown(t_platform *const platform);

        void WindowShow(t_platform *const platform);

        void PollOSEvents(t_platform *const platform);
    }
}
