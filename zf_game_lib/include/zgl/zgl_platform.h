#pragma once

#include <zcl.h>

namespace zf {
    struct s_platform_layer_info;

    t_f64 Time();

    void SetWindowTitle(const s_platform_layer_info *const pli, const s_str_rdonly title);

    // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there
    // is DPI scaling.
    void SetWindowSize(const s_platform_layer_info *const pli, const s_v2_i size);

    // Set the LOGICAL window size limits. If you don't want to limit a particular dimension,
    // leave -1.
    void SetWindowSizeLimits(const s_platform_layer_info *const pli, const t_i32 min_width,
                             const t_i32 min_height, const t_i32 max_width,
                             const t_i32 max_height);

    void SetWindowResizability(const s_platform_layer_info *const pli, const t_b8 resizable);

    s_v2_i WindowFramebufferSizeCache(const s_platform_layer_info *const pli);

    // Returns the size in pixels of whichever monitor the window most resides in.
    s_v2_i CalcMonitorPixelSize(const s_platform_layer_info *const pli);

    // Returns the size (accounting for DPI scaling) of whichever monitor the window most
    // resides in.
    s_v2_i CalcMonitorLogicalSize(const s_platform_layer_info *const pli);

    t_b8 IsFullscreen(const s_platform_layer_info *const pli);
    void SetFullscreen(s_platform_layer_info *const pli, const t_b8 fs);

    inline void ToggleFullscreen(s_platform_layer_info *const pli) {
        SetFullscreen(pli, !IsFullscreen(pli));
    }

    void SetCursorVisibility(const s_platform_layer_info *const pli, const t_b8 visible);

    struct s_input_state;

    namespace internal {
        [[nodiscard]] s_platform_layer_info *InitPlatformLayer(
            s_mem_arena *const mem_arena, s_input_state *const input_state);
        void ShutdownPlatformLayer(const s_platform_layer_info *const pli);

        void PollOSEvents();

        void ShowWindow(const s_platform_layer_info *const pli);
        t_b8 ShouldWindowClose(const s_platform_layer_info *const pli);
        void SwapWindowBuffers(const s_platform_layer_info *const pli);
    }
}
