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

    class c_rgba_texture {
    public:
        c_rgba_texture() = default;

        c_rgba_texture(const s_v2<int> size_in_pxs, const c_array<const t_byte> px_data) : m_size_in_pxs(size_in_pxs), m_px_data(px_data) {
            ZF_ASSERT(px_data.IsEmpty() || (px_data.Len() == 4 * size_in_pxs.x * size_in_pxs.y));
        }

        [[nodiscard]] bool LoadFromRaw(c_mem_arena& mem_arena, const s_str_view file_path);

        s_v2<int> SizeInPixels() const {
            ZF_ASSERT(IsLoaded());
            return m_size_in_pxs;
        }

        c_array<const t_byte> PixelData() const {
            ZF_ASSERT(IsLoaded());
            return m_px_data;
        }

        bool IsLoaded() const {
            return !m_px_data.IsEmpty();
        }

    private:
        s_v2<int> m_size_in_pxs;
        c_array<const t_byte> m_px_data; // 4 bytes per pixel (RGBA).
    };

    bool PackTexture(const s_str_view file_path, const c_rgba_texture tex, c_mem_arena& temp_mem_arena);
    bool UnpackTexture(c_rgba_texture& tex, const s_str_view file_path);
}
