#pragma once

#include <zc/zc_io.h>
#include <zc/zc_math.h>

namespace zf {
    class c_color_rgba32f {
    public:
        constexpr c_color_rgba32f() = default;

        constexpr c_color_rgba32f(const t_f32 r, const t_f32 g, const t_f32 b, const t_f32 a = 1.0f) : m_r(r), m_g(g), m_b(b), m_a(a) {
            ZF_ASSERT(r >= 0.0f && r <= 1.0f
                && g >= 0.0f && g <= 1.0f
                && b >= 0.0f && b <= 1.0f
                && a >= 0.0f && a <= 1.0f);
        }

        constexpr t_f32 R() const { return m_r; }
        constexpr t_f32 G() const { return m_g; }
        constexpr t_f32 B() const { return m_b; }
        constexpr t_f32 A() const { return m_a; }

        constexpr void SetR(const t_f32 r) {
            ZF_ASSERT(r >= 0.0f && r <= 1.0f);
            m_r = r;
        }

        constexpr void SetG(const t_f32 g) {
            ZF_ASSERT(g >= 0.0f && g <= 1.0f);
            m_g = g;
        }

        constexpr void SetB(const t_f32 b) {
            ZF_ASSERT(b >= 0.0f && b <= 1.0f);
            m_b = b;
        }

        constexpr void SetA(const t_f32 a) {
            ZF_ASSERT(a >= 0.0f && a <= 1.0f);
            m_a = a;
        }

        constexpr c_color_rgba32f MixedWith(const c_color_rgba32f other, const t_f32 amount) const {
            ZF_ASSERT(amount >= 0.0f && amount <= 1.0f);

            return {
                Lerp(m_r, other.R(), amount),
                Lerp(m_g, other.G(), amount),
                Lerp(m_b, other.B(), amount),
                Lerp(m_a, other.A(), amount)
            };
        }

        constexpr t_f32 Luminance() const {
            return (0.2126f * m_r) + (0.7152f * m_g) + (0.0722f * m_b);
        }

        constexpr c_color_rgba32f Grayscale() const {
            const t_f32 lum = Luminance();
            return {lum, lum, lum, m_a};
        }

    private:
        t_f32 m_r = 0.0f;
        t_f32 m_g = 0.0f;
        t_f32 m_b = 0.0f;
        t_f32 m_a = 0.0f;
    };

    struct s_color_rgba8 {
        t_u8 r = 0;
        t_u8 g = 0;
        t_u8 b = 0;
        t_u8 a = 0;

        constexpr s_color_rgba8() = default;
        constexpr s_color_rgba8(const t_u8 r, const t_u8 g, const t_u8 b, const t_u8 a) : r(r), g(g), b(b), a(a) {}

        static constexpr s_color_rgba8 FromHex(const t_u32 hex) {
            const auto r = static_cast<t_u8>((hex & 0xFF000000) >> 24);
            const auto g = static_cast<t_u8>((hex & 0x00FF0000) >> 16);
            const auto b = static_cast<t_u8>((hex & 0x0000FF00) >> 8);
            const auto a = static_cast<t_u8>(hex & 0x000000FF);

            return {r, g, b, a};
        }

        constexpr operator c_color_rgba32f() {
            return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
        }
    };

    namespace colors {
        constexpr c_color_rgba32f g_black = {0.0f, 0.0f, 0.0f};
        constexpr c_color_rgba32f g_dark_gray = {0.25f, 0.25f, 0.25f};
        constexpr c_color_rgba32f g_gray = {0.5f, 0.5f, 0.5f};
        constexpr c_color_rgba32f g_light_gray = {0.75f, 0.75f, 0.75f};
        constexpr c_color_rgba32f g_white = {1.0f, 1.0f, 1.0f};
        constexpr c_color_rgba32f g_red = {1.0f, 0.0f, 0.0f};
        constexpr c_color_rgba32f g_orange = {1.0f, 0.5f, 0.0f};
        constexpr c_color_rgba32f g_yellow = {1.0f, 1.0f, 0.0f};
        constexpr c_color_rgba32f g_lime = {0.75f, 1.0f, 0.0f};
        constexpr c_color_rgba32f g_green = {0.0f, 1.0f, 0.0f};
        constexpr c_color_rgba32f g_teal = {0.0f, 0.5f, 0.5f};
        constexpr c_color_rgba32f g_cyan = {0.0f, 1.0f, 1.0f};
        constexpr c_color_rgba32f g_blue = {0.0f, 0.0f, 1.0f};
        constexpr c_color_rgba32f g_purple = {0.5f, 0.0f, 0.5f};
        constexpr c_color_rgba32f g_magenta = {1.0f, 0.0f, 1.0f};
        constexpr c_color_rgba32f g_pink = {1.0f, 0.75f, 0.8f};
        constexpr c_color_rgba32f g_brown = {0.6f, 0.3f, 0.0f};
    }

    namespace origins {
        constexpr s_v2<t_f32> g_topleft = {0.0f, 0.0f};
        constexpr s_v2<t_f32> g_topcenter = {0.5f, 0.0f};
        constexpr s_v2<t_f32> g_topright = {1.0f, 0.0f};
        constexpr s_v2<t_f32> g_centerleft = {0.0f, 0.5f};
        constexpr s_v2<t_f32> g_center = {0.5f, 0.5f};
        constexpr s_v2<t_f32> g_centerright = {1.0f, 0.5f};
        constexpr s_v2<t_f32> g_bottomleft = {0.0f, 1.0f};
        constexpr s_v2<t_f32> g_bottomcenter = {0.5f, 1.0f};
        constexpr s_v2<t_f32> g_bottomright = {1.0f, 1.0f};
    }

    namespace alignments {
        constexpr s_v2<t_f32> g_topleft = {0.0f, 0.0f};
        constexpr s_v2<t_f32> g_topcenter = {0.5f, 0.0f};
        constexpr s_v2<t_f32> g_topright = {1.0f, 0.0f};
        constexpr s_v2<t_f32> g_centerleft = {0.0f, 0.5f};
        constexpr s_v2<t_f32> g_center = {0.5f, 0.5f};
        constexpr s_v2<t_f32> g_centerright = {1.0f, 0.5f};
        constexpr s_v2<t_f32> g_bottomleft = {0.0f, 1.0f};
        constexpr s_v2<t_f32> g_bottomcenter = {0.5f, 1.0f};
        constexpr s_v2<t_f32> g_bottomright = {1.0f, 1.0f};
    }

    // For simplicity, RGBA is the only format we work with.

    struct s_texture_data_view {
        s_v2<t_s32> size_in_pxs;
        c_array<const t_u8> rgba_px_data;

        s_texture_data_view() = default;
        s_texture_data_view(const s_v2<t_s32> size_in_pxs, const c_array<const t_u8> rgba_px_data)
            : size_in_pxs(size_in_pxs), rgba_px_data(rgba_px_data) {}
    };

    struct s_texture_data {
        s_v2<t_s32> size_in_pxs;
        c_array<t_u8> rgba_px_data;

        s_texture_data() = default;
        s_texture_data(const s_v2<t_s32> size_in_pxs, const c_array<t_u8> rgba_px_data)
            : size_in_pxs(size_in_pxs), rgba_px_data(rgba_px_data) {}

        operator s_texture_data_view() const {
            return {size_in_pxs, c_array<const t_u8>(rgba_px_data)};
        }
    };

    [[nodiscard]] t_b8 LoadTextureFromRaw(const s_str_view file_path, c_mem_arena& mem_arena, s_texture_data& o_tex_data);
    [[nodiscard]] t_b8 LoadTextureFromPacked(const s_str_view file_path, c_mem_arena& mem_arena, s_texture_data& o_tex_data);

    [[nodiscard]] t_b8 PackTexture(const s_texture_data_view& tex_data, const s_str_view file_path, c_mem_arena& temp_mem_arena);
}
