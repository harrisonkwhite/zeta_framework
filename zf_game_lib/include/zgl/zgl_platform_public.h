#pragma once

#include <zcl.h>

namespace zgl {
    struct t_input_state; // Forward declaration from Input.

    struct t_platform_ticket_rdonly {
        zcl::t_u64 val;
    };

    struct t_platform_ticket_mut {
        zcl::t_u64 val;

        operator t_platform_ticket_rdonly() const {
            return {val};
        }
    };

    // Gives the time in seconds since the platform module was started.
    zcl::t_f64 GetTime(const t_platform_ticket_rdonly platform_ticket);

    void WindowRequestClose(const t_platform_ticket_mut platform_ticket);

    // Note that a window close request can also be triggered during OS event polling, not just by you.
    zcl::t_b8 WindowCheckCloseRequested(const t_platform_ticket_rdonly platform_ticket);

    zcl::t_b8 WindowCheckFocused(const t_platform_ticket_rdonly platform_ticket);

    void WindowSetTitle(const t_platform_ticket_mut platform_ticket, const zcl::t_str_rdonly title, zcl::t_arena *const temp_arena);

    // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
    void WindowSetSize(const t_platform_ticket_mut platform_ticket, const zcl::t_v2_i size);

    // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave it as -1.
    void WindowSetSizeLimits(const t_platform_ticket_mut platform_ticket, const zcl::t_i32 min_width, const zcl::t_i32 min_height, const zcl::t_i32 max_width, const zcl::t_i32 max_height);

    void WindowSetResizable(const t_platform_ticket_mut platform_ticket, const zcl::t_b8 resizable);

    zcl::t_v2_i WindowGetFramebufferSizeCache(const t_platform_ticket_rdonly platform_ticket);

    zcl::t_b8 WindowCheckFullscreen(const t_platform_ticket_rdonly platform_ticket);
    void WindowSetFullscreen(const t_platform_ticket_mut platform_ticket, const zcl::t_b8 active);

    inline void WindowToggleFullscreen(const t_platform_ticket_mut platform_ticket) {
        WindowSetFullscreen(platform_ticket, !WindowCheckFullscreen(platform_ticket));
    }

    // Calculates the size in pixels of whichever monitor the window most resides in.
    zcl::t_v2_i MonitorCalcSizePixels(const t_platform_ticket_rdonly platform_ticket);

    // Calculates the size (accounting for DPI scaling) of whichever monitor the window most resides in.
    zcl::t_v2_i MonitorCalcSizeLogical(const t_platform_ticket_rdonly platform_ticket);

    void CursorSetVisible(const t_platform_ticket_mut platform_ticket, const zcl::t_b8 visible);

    namespace internal {
        // Note that the window is not shown by default, you have to manually do this.
        t_platform_ticket_mut PlatformStartup(const zcl::t_v2_i init_window_size, t_input_state *const input_state, zcl::t_arena *const arena);

        void PlatformShutdown(const t_platform_ticket_mut platform_ticket);

        void *DisplayGetNativeHandle(const t_platform_ticket_rdonly platform_ticket);

        void *WindowGetNativeHandle(const t_platform_ticket_rdonly platform_ticket);

        void WindowShow(const t_platform_ticket_mut platform_ticket);

        void PollOSEvents(const t_platform_ticket_mut platform_ticket, t_input_state *const input_state);
    }
}
