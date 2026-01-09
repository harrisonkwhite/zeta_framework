#pragma once

#include <zcl.h>
#include <zgl/zgl_input.h>

namespace zgl::platform {
    // Note that the window is not shown by default, you have to manually do this.
    void module_startup(const zf::math::t_v2_i init_window_size);

    void module_shutdown();

    // Gives the time in seconds since the platform module was started.
    zf::t_f64 get_time();

    // Also updates the given input state based on OS events.
    void poll_os_events(input::t_state *const input_state);

    void *display_get_native_handle();

    void *window_get_native_handle();

    void window_show();

    // Returns whether a window close has been requested.
    zf::t_b8 window_should_close();

    void window_set_title(const zf::strs::t_str_rdonly title, zf::mem::t_arena *const temp_arena);

    // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
    void window_set_size(const zf::math::t_v2_i size);

    // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave it as -1.
    void window_set_size_limits(const zf::t_i32 min_width, const zf::t_i32 min_height, const zf::t_i32 max_width, const zf::t_i32 max_height);

    void window_set_resizable(const zf::t_b8 resizable);

    zf::math::t_v2_i window_get_framebuffer_size_cache();

    zf::t_b8 window_check_fullscreen();

    void window_set_fullscreen(const zf::t_b8 active);

    inline void window_toggle_fullscreen() {
        window_set_fullscreen(!window_check_fullscreen());
    }

    // Calculates the size in pixels of whichever monitor the window most resides in.
    zf::math::t_v2_i monitor_calc_size_pixels();

    // Calculates the size (accounting for DPI scaling) of whichever monitor the window most resides in.
    zf::math::t_v2_i monitor_calc_size_logical();

    void cursor_set_visible(const zf::t_b8 visible);
}
