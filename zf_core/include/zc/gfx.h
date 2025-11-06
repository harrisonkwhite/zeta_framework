#pragma once

#include <zc/io.h>
#include <zc/math.h>

namespace zf {
    namespace colors {
        constexpr s_v4<float> g_black = {0.0f, 0.0f, 0.0f, 1.0f};
        constexpr s_v4<float> g_dark_gray = {0.25f, 0.25f, 0.25f, 1.0f};
        constexpr s_v4<float> g_gray = {0.5f, 0.5f, 0.5f, 1.0f};
        constexpr s_v4<float> g_light_gray = {0.75f, 0.75f, 0.75f, 1.0f};
        constexpr s_v4<float> g_white = {1.0f, 1.0f, 1.0f, 1.0f};
        constexpr s_v4<float> g_red = {1.0f, 0.0f, 0.0f, 1.0f};
        constexpr s_v4<float> g_orange = {1.0f, 0.5f, 0.0f, 1.0f};
        constexpr s_v4<float> g_yellow = {1.0f, 1.0f, 0.0f, 1.0f};
        constexpr s_v4<float> g_lime = {0.75f, 1.0f, 0.0f, 1.0f};
        constexpr s_v4<float> g_green = {0.0f, 1.0f, 0.0f, 1.0f};
        constexpr s_v4<float> g_teal = {0.0f, 0.5f, 0.5f, 1.0f};
        constexpr s_v4<float> g_cyan = {0.0f, 1.0f, 1.0f, 1.0f};
        constexpr s_v4<float> g_blue = {0.0f, 0.0f, 1.0f, 1.0f};
        constexpr s_v4<float> g_purple = {0.5f, 0.0f, 0.5f, 1.0f};
        constexpr s_v4<float> g_magenta = {1.0f, 0.0f, 1.0f, 1.0f};
        constexpr s_v4<float> g_pink = {1.0f, 0.75f, 0.8f, 1.0f};
        constexpr s_v4<float> g_brown = {0.6f, 0.3f, 0.0f, 1.0f};
    }

    namespace origins {
        constexpr s_v2<float> g_topleft = {0.0f, 0.0f};
        constexpr s_v2<float> g_topcenter = {0.5f, 0.0f};
        constexpr s_v2<float> g_topright = {1.0f, 0.0f};
        constexpr s_v2<float> g_centerleft = {0.0f, 0.5f};
        constexpr s_v2<float> g_center = {0.5f, 0.5f};
        constexpr s_v2<float> g_centerright = {1.0f, 0.5f};
        constexpr s_v2<float> g_bottomleft = {0.0f, 1.0f};
        constexpr s_v2<float> g_bottomcenter = {0.5f, 1.0f};
        constexpr s_v2<float> g_bottomright = {1.0f, 1.0f};
    }

    namespace alignments {
        constexpr s_v2<float> g_topleft = {0.0f, 0.0f};
        constexpr s_v2<float> g_topcenter = {0.5f, 0.0f};
        constexpr s_v2<float> g_topright = {1.0f, 0.0f};
        constexpr s_v2<float> g_centerleft = {0.0f, 0.5f};
        constexpr s_v2<float> g_center = {0.5f, 0.5f};
        constexpr s_v2<float> g_centerright = {1.0f, 0.5f};
        constexpr s_v2<float> g_bottomleft = {0.0f, 1.0f};
        constexpr s_v2<float> g_bottomcenter = {0.5f, 1.0f};
        constexpr s_v2<float> g_bottomright = {1.0f, 1.0f};
    }

    struct s_rgba_texture {
        s_v2<int> size_in_pxs;
        c_array<t_byte> px_data; // 4 bytes per pixel (RGBA).
    };

    bool LoadRGBATextureFromRaw(s_rgba_texture& tex, c_mem_arena& mem_arena, const s_str_view file_path);
    bool PackRGBATexture(s_file_stream& fs, const s_rgba_texture tex);
    bool UnpackRGBATexture(s_rgba_texture& tex, s_file_stream& fs);
}
