#pragma once

#include <zc/io.h>
#include <zc/math.h>

namespace zf {
    class c_color_rgba {
    public:
        constexpr c_color_rgba() = default;

        constexpr c_color_rgba(const float r, const float g, const float b, const float a = 1.0f) : m_r(r), m_g(g), m_b(b), m_a(a) {
            ZF_ASSERT(r >= 0.0f && r <= 1.0f
                && g >= 0.0f && g <= 1.0f
                && b >= 0.0f && b <= 1.0f
                && a >= 0.0f && a <= 1.0f);
        }

        constexpr float R() const { return m_r; }
        constexpr float G() const { return m_g; }
        constexpr float B() const { return m_b; }
        constexpr float A() const { return m_a; }

        constexpr void SetR(const float r) {
            ZF_ASSERT(r >= 0.0f && r <= 1.0f);
            m_r = r;
        }

        constexpr void SetG(const float g) {
            ZF_ASSERT(g >= 0.0f && g <= 1.0f);
            m_g = g;
        }

        constexpr void SetB(const float b) {
            ZF_ASSERT(b >= 0.0f && b <= 1.0f);
            m_b = b;
        }

        constexpr void SetA(const float a) {
            ZF_ASSERT(a >= 0.0f && a <= 1.0f);
            m_a = a;
        }

        constexpr c_color_rgba MixedWith(const c_color_rgba other, const float amount) const {
            ZF_ASSERT(amount >= 0.0f && amount <= 1.0f);

            return {
                Lerp(m_r, other.R(), amount),
                Lerp(m_g, other.G(), amount),
                Lerp(m_b, other.B(), amount),
                Lerp(m_a, other.A(), amount)
            };
        }

        constexpr float Luminance() const {
            return (0.2126f * m_r) + (0.7152f * m_g) + (0.0722f * m_b);
        }

        constexpr c_color_rgba Grayscale() const {
            const float lum = Luminance();
            return {lum, lum, lum, m_a};
        }

    private:
        float m_r = 0.0f;
        float m_g = 0.0f;
        float m_b = 0.0f;
        float m_a = 0.0f;
    };

    namespace colors {
        constexpr c_color_rgba g_black = {0.0f, 0.0f, 0.0f};
        constexpr c_color_rgba g_dark_gray = {0.25f, 0.25f, 0.25f};
        constexpr c_color_rgba g_gray = {0.5f, 0.5f, 0.5f};
        constexpr c_color_rgba g_light_gray = {0.75f, 0.75f, 0.75f};
        constexpr c_color_rgba g_white = {1.0f, 1.0f, 1.0f};
        constexpr c_color_rgba g_red = {1.0f, 0.0f, 0.0f};
        constexpr c_color_rgba g_orange = {1.0f, 0.5f, 0.0f};
        constexpr c_color_rgba g_yellow = {1.0f, 1.0f, 0.0f};
        constexpr c_color_rgba g_lime = {0.75f, 1.0f, 0.0f};
        constexpr c_color_rgba g_green = {0.0f, 1.0f, 0.0f};
        constexpr c_color_rgba g_teal = {0.0f, 0.5f, 0.5f};
        constexpr c_color_rgba g_cyan = {0.0f, 1.0f, 1.0f};
        constexpr c_color_rgba g_blue = {0.0f, 0.0f, 1.0f};
        constexpr c_color_rgba g_purple = {0.5f, 0.0f, 0.5f};
        constexpr c_color_rgba g_magenta = {1.0f, 0.0f, 1.0f};
        constexpr c_color_rgba g_pink = {1.0f, 0.75f, 0.8f};
        constexpr c_color_rgba g_brown = {0.6f, 0.3f, 0.0f};
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
