#pragma once

#include <zcl.h>

namespace zf::platform {
    [[nodiscard]] t_b8 Init(const s_v2_i init_window_size);
    void Shutdown();

    // Gives the time in seconds since the platform module was initialised.
    t_f64 Time();

    void PollOSEvents();

    void *NativeWindowHandle();

    void *NativeDisplayHandle();

    void ShowWindow();

    t_b8 ShouldWindowClose();

    // Returns true iff the operation succeeded.
    [[nodiscard]] t_b8 SetWindowTitle(const s_str_rdonly title, s_mem_arena &temp_mem_arena);

    // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
    void SetWindowSize(const s_v2_i size);

    // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave -1.
    void SetWindowSizeLimits(const t_i32 min_width, const t_i32 min_height, const t_i32 max_width, const t_i32 max_height);

    void SetWindowResizability(const t_b8 resizable);

    s_v2_i WindowFramebufferSizeCache();

    // Returns the size in pixels of whichever monitor the window most resides in.
    s_v2_i CalcMonitorPixelSize();

    // Returns the size (accounting for DPI scaling) of whichever monitor the window most resides in.
    s_v2_i CalcMonitorLogicalSize();

    t_b8 IsFullscreen();
    void SetFullscreen(const t_b8 fs);

    inline void ToggleFullscreen() {
        SetFullscreen(!IsFullscreen());
    }

    void SetCursorVisibility(const t_b8 visible);
}
