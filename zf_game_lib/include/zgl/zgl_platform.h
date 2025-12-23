#pragma once

#include <zcl.h>

namespace zf {
    struct s_input_state;

    // Initialises the platform layer module. The window is not shown by default, you have to manually do this. The given input state is written to as OS events are polled, and needs to exist for the lifetime of this module.
    void InitPlatform(const s_v2_i init_window_size);

    void ShutdownPlatform();

    // Gives the time in seconds since the platform module was initialised.
    t_f64 Time();

    // Also updates the given input state based on OS events.
    void PollOSEvents(s_input_state &input_state);

    s_ptr<void> NativeWindowHandle();
    s_ptr<void> NativeDisplayHandle();

    void ShowWindow();

    // Returns whether a window close has been requested.
    t_b8 ShouldWindowClose();

    void SetWindowTitle(const s_str_rdonly title, s_mem_arena &temp_mem_arena);

    // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
    void SetWindowSize(const s_v2_i size);

    // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave it as -1.
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
