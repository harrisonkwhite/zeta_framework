#pragma once

#include <zcl.h>

namespace zf {
    struct s_platform_layer_info;

    t_f64 Time();

    void SetWindowTitle(const s_ptr<const s_platform_layer_info> pli, const s_str_rdonly title);

    // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
    void SetWindowSize(const s_ptr<const s_platform_layer_info> pli, const s_v2_i size);

    // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave -1.
    void SetWindowSizeLimits(const s_ptr<const s_platform_layer_info> pli, const t_i32 min_width, const t_i32 min_height, const t_i32 max_width, const t_i32 max_height);

    void SetWindowResizability(const s_ptr<const s_platform_layer_info> pli, const t_b8 resizable);

    s_v2_i WindowFramebufferSizeCache(const s_ptr<const s_platform_layer_info> pli);

    // Returns the size in pixels of whichever monitor the window most resides in.
    s_v2_i CalcMonitorPixelSize(const s_ptr<const s_platform_layer_info> pli);

    // Returns the size (accounting for DPI scaling) of whichever monitor the window most resides in.
    s_v2_i CalcMonitorLogicalSize(const s_ptr<const s_platform_layer_info> pli);

    t_b8 IsFullscreen(const s_ptr<const s_platform_layer_info> pli);
    void SetFullscreen(const s_ptr<s_platform_layer_info> pli, const t_b8 fs);

    inline void ToggleFullscreen(const s_ptr<s_platform_layer_info> pli) {
        SetFullscreen(pli, !IsFullscreen(pli));
    }

    void SetCursorVisibility(const s_ptr<const s_platform_layer_info> pli, const t_b8 visible);

    struct s_input_state;

    namespace internal {
        [[nodiscard]] t_b8 InitPlatformLayer(const s_ptr<s_mem_arena> mem_arena, const s_ptr<s_input_state> input_state, const s_ptr<s_ptr<s_platform_layer_info>> o_pli);
        void ShutdownPlatformLayer(const s_ptr<const s_platform_layer_info> pli);

        void PollOSEvents();

        void ShowWindow(const s_ptr_nonnull<const s_platform_layer_info> pli);
        t_b8 ShouldWindowClose(const s_ptr<const s_platform_layer_info> pli);
        void SwapWindowBuffers(const s_ptr<const s_platform_layer_info> pli);
    }
}
