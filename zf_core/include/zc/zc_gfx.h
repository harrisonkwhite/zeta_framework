#pragma once

#include <zc/zc_io.h>
#include <zc/zc_math.h>

namespace zf {
    struct s_color_rgba32f {
        constexpr s_color_rgba32f() = default;

        constexpr s_color_rgba32f(const t_f32 r, const t_f32 g, const t_f32 b, const t_f32 a = 1.0f) : r(r), g(g), b(b), a(a) {
            ZF_ASSERT(r >= 0.0f && r <= 1.0f
                && g >= 0.0f && g <= 1.0f
                && b >= 0.0f && b <= 1.0f
                && a >= 0.0f && a <= 1.0f);
        }

        constexpr t_f32 R() const { return r; }
        constexpr t_f32 G() const { return g; }
        constexpr t_f32 B() const { return b; }
        constexpr t_f32 A() const { return a; }

    private:
        t_f32 r = 0.0f;
        t_f32 g = 0.0f;
        t_f32 b = 0.0f;
        t_f32 a = 0.0f;
    };

    struct s_color_rgba8 {
        t_u8 r = 0;
        t_u8 g = 0;
        t_u8 b = 0;
        t_u8 a = 0;

        constexpr s_color_rgba8() = default;
        constexpr s_color_rgba8(const t_u8 r, const t_u8 g, const t_u8 b, const t_u8 a) : r(r), g(g), b(b), a(a) {}

        constexpr operator s_color_rgba32f() {
            return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
        }
    };

    constexpr s_color_rgba32f MixColors(const s_color_rgba32f a, const s_color_rgba32f b, const t_f32 amount) {
        ZF_ASSERT(amount >= 0.0f && amount <= 1.0f);

        return {
            Lerp(a.R(), b.R(), amount),
            Lerp(a.G(), b.G(), amount),
            Lerp(a.B(), b.B(), amount),
            Lerp(a.A(), b.A(), amount)
        };
    }

    constexpr t_f32 Luminance(const s_color_rgba32f col) {
        return (0.2126f * col.R()) + (0.7152f * col.G()) + (0.0722f * col.B());
    }

    constexpr s_color_rgba32f Grayscale(const s_color_rgba32f col) {
        const t_f32 lum = Luminance(col);
        return {lum, lum, lum, col.A()};
    }

    constexpr s_color_rgba8 ColorFromHex(const t_u32 hex) {
        const auto r = static_cast<t_u8>((hex & 0xFF000000) >> 24);
        const auto g = static_cast<t_u8>((hex & 0x00FF0000) >> 16);
        const auto b = static_cast<t_u8>((hex & 0x0000FF00) >> 8);
        const auto a = static_cast<t_u8>(hex & 0x000000FF);

        return {r, g, b, a};
    }

    namespace colors {
        constexpr s_color_rgba32f g_black = {0.0f, 0.0f, 0.0f};
        constexpr s_color_rgba32f g_dark_gray = {0.25f, 0.25f, 0.25f};
        constexpr s_color_rgba32f g_gray = {0.5f, 0.5f, 0.5f};
        constexpr s_color_rgba32f g_light_gray = {0.75f, 0.75f, 0.75f};
        constexpr s_color_rgba32f g_white = {1.0f, 1.0f, 1.0f};
        constexpr s_color_rgba32f g_red = {1.0f, 0.0f, 0.0f};
        constexpr s_color_rgba32f g_orange = {1.0f, 0.5f, 0.0f};
        constexpr s_color_rgba32f g_yellow = {1.0f, 1.0f, 0.0f};
        constexpr s_color_rgba32f g_lime = {0.75f, 1.0f, 0.0f};
        constexpr s_color_rgba32f g_green = {0.0f, 1.0f, 0.0f};
        constexpr s_color_rgba32f g_teal = {0.0f, 0.5f, 0.5f};
        constexpr s_color_rgba32f g_cyan = {0.0f, 1.0f, 1.0f};
        constexpr s_color_rgba32f g_blue = {0.0f, 0.0f, 1.0f};
        constexpr s_color_rgba32f g_purple = {0.5f, 0.0f, 0.5f};
        constexpr s_color_rgba32f g_magenta = {1.0f, 0.0f, 1.0f};
        constexpr s_color_rgba32f g_pink = {1.0f, 0.75f, 0.8f};
        constexpr s_color_rgba32f g_brown = {0.6f, 0.3f, 0.0f};
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

    struct s_texture_data {
        s_v2<t_s32> size_in_pxs;
        s_array<t_u8> rgba_px_data;

        s_texture_data() = default;
        s_texture_data(const s_v2<t_s32> size_in_pxs, const s_array<t_u8> rgba_px_data)
            : size_in_pxs(size_in_pxs), rgba_px_data(rgba_px_data) {}
    };

    [[nodiscard]] t_b8 LoadTextureFromRaw(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_texture_data& o_tex_data);
    [[nodiscard]] t_b8 LoadTextureFromPacked(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_texture_data& o_tex_data);

    [[nodiscard]] t_b8 PackTexture(const s_texture_data& tex_data, const s_str_rdonly file_path, s_mem_arena& temp_mem_arena);
}
