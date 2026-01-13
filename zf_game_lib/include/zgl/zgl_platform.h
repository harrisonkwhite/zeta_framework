#pragma once

#include <zcl.h>

namespace zgl {
    namespace input {
        struct t_state;
    }

    namespace platform {
        // Note that the window is not shown by default, you have to manually do this.
        void module_startup(const zcl::t_v2_i init_window_size);

        void module_shutdown();

        // Gives the time in seconds since the platform module was started.
        zcl::t_f64 get_time();

        // Also updates the given input state based on events.
        void poll_events(input::t_state *const input_state);

        void *display_get_native_handle();

        void *window_get_native_handle();

        void window_show();

        void window_request_close();
        zcl::t_b8 window_check_close_requested();

        void window_set_title(const zcl::strs::t_str_rdonly title, zcl::t_arena *const temp_arena);

        // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
        void window_set_size(const zcl::t_v2_i size);

        // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave it as -1.
        void window_set_size_limits(const zcl::t_i32 min_width, const zcl::t_i32 min_height, const zcl::t_i32 max_width, const zcl::t_i32 max_height);

        void window_set_resizable(const zcl::t_b8 resizable);

        zcl::t_v2_i window_get_framebuffer_size_cache();

        zcl::t_b8 window_check_fullscreen();

        void window_set_fullscreen(const zcl::t_b8 active);

        inline void window_toggle_fullscreen() {
            window_set_fullscreen(!window_check_fullscreen());
        }

        // Calculates the size in pixels of whichever monitor the window most resides in.
        zcl::t_v2_i monitor_calc_size_pixels();

        // Calculates the size (accounting for DPI scaling) of whichever monitor the window most resides in.
        zcl::t_v2_i monitor_calc_size_logical();

        void cursor_set_visible(const zcl::t_b8 visible);
    }
}
