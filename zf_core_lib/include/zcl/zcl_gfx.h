#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_math.h>
#include <zcl/zcl_ds.h>

namespace zf {
    // ============================================================
    // @section: Colours
    // ============================================================
    struct s_color_rgba32f {
    public:
        constexpr s_color_rgba32f() = default;

        constexpr s_color_rgba32f(const t_f32 r, const t_f32 g, const t_f32 b, const t_f32 a) : m_r(r), m_g(g), m_b(b), m_a(a) {
            ZF_ASSERT(r >= 0.0f && r <= 1.0f);
            ZF_ASSERT(g >= 0.0f && g <= 1.0f);
            ZF_ASSERT(b >= 0.0f && b <= 1.0f);
            ZF_ASSERT(a >= 0.0f && a <= 1.0f);
        }

        constexpr operator s_v4() const {
            return {m_r, m_g, m_b, m_a};
        }

        constexpr t_f32 R() const {
            return m_r;
        }

        constexpr t_f32 G() const {
            return m_g;
        }

        constexpr t_f32 B() const {
            return m_b;
        }

        constexpr t_f32 A() const {
            return m_a;
        }

    private:
        t_f32 m_r = 0.0f;
        t_f32 m_g = 0.0f;
        t_f32 m_b = 0.0f;
        t_f32 m_a = 0.0f;
    };

    struct s_color_rgb24f {
    public:
        constexpr s_color_rgb24f() = default;

        constexpr s_color_rgb24f(const t_f32 r, const t_f32 g, const t_f32 b) : m_r(r), m_g(g), m_b(b) {
            ZF_ASSERT(r >= 0.0f && r <= 1.0f);
            ZF_ASSERT(g >= 0.0f && g <= 1.0f);
            ZF_ASSERT(b >= 0.0f && b <= 1.0f);
        }

        constexpr operator s_color_rgba32f() const {
            return {m_r, m_g, m_b, 1.0f};
        }

        constexpr operator s_v3() const {
            return {m_r, m_g, m_b};
        }

        constexpr t_f32 R() const {
            return m_r;
        }

        constexpr t_f32 G() const {
            return m_g;
        }

        constexpr t_f32 B() const {
            return m_b;
        }

    private:
        t_f32 m_r = 0.0f;
        t_f32 m_g = 0.0f;
        t_f32 m_b = 0.0f;
    };

    struct s_color_rgba8 {
        t_u8 r = 0;
        t_u8 g = 0;
        t_u8 b = 0;
        t_u8 a = 0;

        constexpr operator s_color_rgba32f() const {
            return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
        }
    };

    constexpr s_color_rgba32f ColorsMixed(const s_color_rgba32f a, const s_color_rgba32f b, const t_f32 amount) {
        ZF_ASSERT(amount >= 0.0f && amount <= 1.0f);

        return {
            Lerp(a.R(), b.R(), amount),
            Lerp(a.G(), b.G(), amount),
            Lerp(a.B(), b.B(), amount),
            Lerp(a.A(), b.A(), amount),
        };
    }

    constexpr t_f32 ColorLuminance(const s_color_rgba32f col) {
        return (0.2126f * col.R()) + (0.7152f * col.G()) + (0.0722f * col.B());
    }

    constexpr s_color_rgba32f ColorAsGrayscale(const s_color_rgba32f col) {
        const t_f32 lum = ColorLuminance(col);
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
        constexpr s_color_rgb24f g_black = {0.0f, 0.0f, 0.0f};
        constexpr s_color_rgb24f g_dark_gray = {0.25f, 0.25f, 0.25f};
        constexpr s_color_rgb24f g_gray = {0.5f, 0.5f, 0.5f};
        constexpr s_color_rgb24f g_light_gray = {0.75f, 0.75f, 0.75f};
        constexpr s_color_rgb24f g_white = {1.0f, 1.0f, 1.0f};
        constexpr s_color_rgb24f g_red = {1.0f, 0.0f, 0.0f};
        constexpr s_color_rgb24f g_orange = {1.0f, 0.5f, 0.0f};
        constexpr s_color_rgb24f g_yellow = {1.0f, 1.0f, 0.0f};
        constexpr s_color_rgb24f g_lime = {0.75f, 1.0f, 0.0f};
        constexpr s_color_rgb24f g_green = {0.0f, 1.0f, 0.0f};
        constexpr s_color_rgb24f g_teal = {0.0f, 0.5f, 0.5f};
        constexpr s_color_rgb24f g_cyan = {0.0f, 1.0f, 1.0f};
        constexpr s_color_rgb24f g_blue = {0.0f, 0.0f, 1.0f};
        constexpr s_color_rgb24f g_purple = {0.5f, 0.0f, 0.5f};
        constexpr s_color_rgb24f g_magenta = {1.0f, 0.0f, 1.0f};
        constexpr s_color_rgb24f g_pink = {1.0f, 0.75f, 0.8f};
        constexpr s_color_rgb24f g_brown = {0.6f, 0.3f, 0.0f};
    }

    // ============================================================
    // @section: Textures
    // ============================================================
    namespace origins {
        constexpr s_v2 g_topleft = {0.0f, 0.0f};
        constexpr s_v2 g_topcenter = {0.5f, 0.0f};
        constexpr s_v2 g_topright = {1.0f, 0.0f};
        constexpr s_v2 g_centerleft = {0.0f, 0.5f};
        constexpr s_v2 g_center = {0.5f, 0.5f};
        constexpr s_v2 g_centerright = {1.0f, 0.5f};
        constexpr s_v2 g_bottomleft = {0.0f, 1.0f};
        constexpr s_v2 g_bottomcenter = {0.5f, 1.0f};
        constexpr s_v2 g_bottomright = {1.0f, 1.0f};
    }

    constexpr t_b8 IsOriginValid(const s_v2 origin) {
        return origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f;
    }

    struct s_texture_data_rdonly {
    public:
        constexpr s_texture_data_rdonly() = default;

        constexpr s_texture_data_rdonly(const s_v2_i size_in_pxs, const s_array_rdonly<t_u8> rgba_px_data) : m_size_in_pxs(size_in_pxs), m_rgba_px_data(rgba_px_data) {
            ZF_ASSERT(rgba_px_data.Len() == 4 * size_in_pxs.x * size_in_pxs.y);
        }

        constexpr s_v2_i SizeInPixels() const {
            return m_size_in_pxs;
        }

        constexpr s_array_rdonly<t_u8> RGBAPixelData() const {
            return m_rgba_px_data;
        }

    private:
        s_v2_i m_size_in_pxs = {};
        s_array_rdonly<t_u8> m_rgba_px_data = {};
    };

    struct s_texture_data {
    public:
        constexpr s_texture_data() = default;

        constexpr s_texture_data(const s_v2_i size_in_pxs, const s_array<t_u8> &rgba_px_data) : m_size_in_pxs(size_in_pxs), m_rgba_px_data(rgba_px_data) {
            ZF_ASSERT(rgba_px_data.Len() == 4 * size_in_pxs.x * size_in_pxs.y);
        }

        constexpr operator s_texture_data_rdonly() const {
            return {m_size_in_pxs, m_rgba_px_data};
        }

        constexpr s_v2_i SizeInPixels() const {
            return m_size_in_pxs;
        }

        constexpr s_array_rdonly<t_u8> RGBAPixelData() const {
            return m_rgba_px_data;
        }

    private:
        s_v2_i m_size_in_pxs = {};
        s_array<t_u8> m_rgba_px_data = {};
    };

    [[nodiscard]] t_b8 LoadTextureFromRaw(const s_str_rdonly file_path, s_mem_arena &tex_data_mem_arena, s_mem_arena &temp_mem_arena, s_texture_data &o_tex_data);

    [[nodiscard]] t_b8 PackTexture(const s_str_rdonly file_path, const s_texture_data tex_data, s_mem_arena &temp_mem_arena);
    [[nodiscard]] t_b8 UnpackTexture(const s_str_rdonly file_path, s_mem_arena &tex_data_mem_arena, s_mem_arena &temp_mem_arena, s_texture_data &o_tex_data);

    inline s_rect_f CalcTextureCoords(const s_rect_i src_rect, const s_v2_i tex_size) {
        return {
            static_cast<t_f32>(src_rect.x) / static_cast<t_f32>(tex_size.x),
            static_cast<t_f32>(src_rect.y) / static_cast<t_f32>(tex_size.y),
            static_cast<t_f32>(src_rect.width) / static_cast<t_f32>(tex_size.x),
            static_cast<t_f32>(src_rect.height) / static_cast<t_f32>(tex_size.y),
        };
    }
}
