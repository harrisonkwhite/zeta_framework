#pragma once

#include <zcl.h>

namespace zf {
    // Note that the window is not shown by default, you have to manually do this.
    void f_platform_startup(const s_v2_i init_window_size);

    void f_platform_shutdown();

    // Gives the time in seconds since the platform module was started.
    t_f64 f_platform_get_time();

    struct t_input_state;

    // Also updates the given input state based on OS events.
    void f_platform_poll_os_events(t_input_state *const input_state);

    void *f_platform_get_native_display_handle();

    void *f_platform_get_native_window_handle();

    void f_platform_show_window();

    // Returns whether a window close has been requested.
    t_b8 f_platform_should_window_close();

    void f_platform_set_window_title(const t_str_rdonly title, t_arena *const temp_arena);

    // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
    void f_platform_set_window_size(const s_v2_i size);

    // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave it as -1.
    void f_platform_set_window_size_limits(const t_i32 min_width, const t_i32 min_height, const t_i32 max_width, const t_i32 max_height);

    void f_platform_set_window_resizable(const t_b8 resizable);

    s_v2_i f_platform_get_window_framebuffer_size_cache();

    t_b8 f_platform_get_fullscreen();

    void f_platform_set_fullscreen(const t_b8 active);

    inline void f_platform_toggle_fullscreen() {
        f_platform_set_fullscreen(!f_platform_get_fullscreen());
    }

    // Calculates the size in pixels of whichever monitor the window most resides in.
    s_v2_i f_platform_get_monitor_size_pixels();

    // Calculates the size (accounting for DPI scaling) of whichever monitor the window most resides in.
    s_v2_i f_platform_get_monitor_size_logical();

    void f_platform_set_cursor_visibility(const t_b8 visible);
}
