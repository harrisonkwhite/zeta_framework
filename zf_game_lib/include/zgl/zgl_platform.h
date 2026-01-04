#pragma once

#include <zcl.h>
#include <zgl/zgl_input.h>

namespace zf {
    // Note that the window is not shown by default, you have to manually do this.
    void platform_startup(const s_v2_i init_window_size);

    void platform_shutdown();

    // Gives the time in seconds since the platform module was started.
    F64 platform_get_time();

    // Also updates the given input state based on OS events.
    void platform_poll_os_events(t_input_state *const input_state);

    void *platform_get_native_display_handle();

    void *platform_get_native_window_handle();

    void platform_show_window();

    // Returns whether a window close has been requested.
    B8 platform_get_window_should_close();

    void platform_set_window_title(const strs::StrRdonly title, s_arena *const temp_arena);

    // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
    void platform_set_window_size(const s_v2_i size);

    // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave it as -1.
    void platform_set_window_size_limits(const I32 min_width, const I32 min_height, const I32 max_width, const I32 max_height);

    void platform_set_window_resizable(const B8 resizable);

    s_v2_i platform_get_window_framebuffer_size_cache();

    B8 platform_get_fullscreen_active();

    void platform_set_fullscreen_active(const B8 active);

    inline void platform_toggle_fullscreen_active() {
        platform_set_fullscreen_active(!platform_get_fullscreen_active());
    }

    // Returns the size in pixels of whichever monitor the window most resides in.
    s_v2_i platform_calc_monitor_size_pixels();

    // Returns the size (accounting for DPI scaling) of whichever monitor the window most resides in.
    s_v2_i platform_calc_monitor_size_logical();

    void platform_set_cursor_visibility(const B8 visible);
}
