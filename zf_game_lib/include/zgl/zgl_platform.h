#pragma once

#include <zcl.h>
#include <zgl/zgl_input.h>

namespace zf::platform {
    // Note that the window is not shown by default, you have to manually do this.
    void f_startup(const t_v2_i init_window_size);

    void f_shutdown();

    // Gives the time in seconds since the platform module was started.
    t_f64 f_get_time();

    // Also updates the given input state based on OS events.
    void f_poll_os_events(input::t_state *const input_state);

    void *f_get_native_display_handle();

    void *f_get_native_window_handle();

    void f_show_window();

    // Returns whether a window close has been requested.
    t_b8 f_should_window_close();

    void f_set_window_title(const t_str_rdonly title, t_arena *const temp_arena);

    // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
    void f_set_window_size(const t_v2_i size);

    // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave it as -1.
    void f_set_window_size_limits(const t_i32 min_width, const t_i32 min_height, const t_i32 max_width, const t_i32 max_height);

    void f_set_window_resizable(const t_b8 resizable);

    t_v2_i f_get_window_framebuffer_size_cache();

    t_b8 f_get_fullscreen();

    void f_set_fullscreen(const t_b8 active);

    inline void f_toggle_fullscreen() {
        f_set_fullscreen(!f_get_fullscreen());
    }

    // Calculates the size in pixels of whichever monitor the window most resides in.
    t_v2_i f_get_monitor_size_pixels();

    // Calculates the size (accounting for DPI scaling) of whichever monitor the window most resides in.
    t_v2_i f_get_monitor_size_logical();

    void f_set_cursor_visibility(const t_b8 visible);
}
