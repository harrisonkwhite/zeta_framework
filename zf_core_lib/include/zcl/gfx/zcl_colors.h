#pragma once

#include <zcl/zcl_math.h>
#include <zcl/ds/zcl_hash_maps.h>
#include <zcl/zcl_strs.h>

namespace zcl::gfx {
    struct t_color_rgba32f {
        t_f32 r;
        t_f32 g;
        t_f32 b;
        t_f32 a;

        constexpr operator t_v4() const { return {r, g, b, a}; }
    };

    struct t_color_rgba8 {
        t_u8 r;
        t_u8 g;
        t_u8 b;
        t_u8 a;
    };

    constexpr t_b8 color_check_normalized(const t_color_rgba32f col) {
        return col.r >= 0.0f && col.r <= 1.0f
            && col.g >= 0.0f && col.g <= 1.0f
            && col.b >= 0.0f && col.b <= 1.0f
            && col.a >= 0.0f && col.a <= 1.0f;
    }

    constexpr t_color_rgba32f color_create_rgba32f(const t_f32 r, const t_f32 g, const t_f32 b, const t_f32 a = 1.0f) {
        const t_color_rgba32f result = {r, g, b, a};
        ZF_ASSERT(color_check_normalized(result));
        return result;
    }

    constexpr t_color_rgba8 color_create_rgba8(const t_u8 r, const t_u8 g, const t_u8 b, const t_u8 a = 255) {
        return {r, g, b, a};
    }

    constexpr t_color_rgba32f k_color_transparent_black = color_create_rgba32f(0.0f, 0.0f, 0.0f);
    constexpr t_color_rgba32f k_color_black = color_create_rgba32f(0.0f, 0.0f, 0.0f);
    constexpr t_color_rgba32f k_color_dark_gray = color_create_rgba32f(0.25f, 0.25f, 0.25f);
    constexpr t_color_rgba32f k_color_gray = color_create_rgba32f(0.5f, 0.5f, 0.5f);
    constexpr t_color_rgba32f k_color_light_gray = color_create_rgba32f(0.75f, 0.75f, 0.75f);
    constexpr t_color_rgba32f k_color_white = color_create_rgba32f(1.0f, 1.0f, 1.0f);
    constexpr t_color_rgba32f k_color_red = color_create_rgba32f(1.0f, 0.0f, 0.0f);
    constexpr t_color_rgba32f k_color_orange = color_create_rgba32f(1.0f, 0.5f, 0.0f);
    constexpr t_color_rgba32f k_color_yellow = color_create_rgba32f(1.0f, 1.0f, 0.0f);
    constexpr t_color_rgba32f k_color_lime = color_create_rgba32f(0.75f, 1.0f, 0.0f);
    constexpr t_color_rgba32f k_color_green = color_create_rgba32f(0.0f, 1.0f, 0.0f);
    constexpr t_color_rgba32f k_color_teal = color_create_rgba32f(0.0f, 0.5f, 0.5f);
    constexpr t_color_rgba32f k_color_cyan = color_create_rgba32f(0.0f, 1.0f, 1.0f);
    constexpr t_color_rgba32f k_color_blue = color_create_rgba32f(0.0f, 0.0f, 1.0f);
    constexpr t_color_rgba32f k_color_purple = color_create_rgba32f(0.5f, 0.0f, 0.5f);
    constexpr t_color_rgba32f k_color_magenta = color_create_rgba32f(1.0f, 0.0f, 1.0f);
    constexpr t_color_rgba32f k_color_pink = color_create_rgba32f(1.0f, 0.75f, 0.8f);
    constexpr t_color_rgba32f k_color_brown = color_create_rgba32f(0.6f, 0.3f, 0.0f);

    constexpr t_color_rgba8 color_rgba32f_to_rgba8(const t_color_rgba32f col) {
        ZF_ASSERT(color_check_normalized(col));
        return color_create_rgba8(static_cast<t_u8>(255.0f * col.r), static_cast<t_u8>(255.0f * col.g), static_cast<t_u8>(255.0f * col.b), static_cast<t_u8>(255.0f * col.a));
    }

    constexpr t_color_rgba32f color_rgba8_to_rgba32f(const t_color_rgba8 col) {
        return color_create_rgba32f(static_cast<t_f32>(col.r) / 255.0f, static_cast<t_f32>(col.g) / 255.0f, static_cast<t_f32>(col.b) / 255.0f, static_cast<t_f32>(col.a) / 255.0f);
    }

    constexpr t_color_rgba32f color_get_mix(const t_color_rgba32f a, const t_color_rgba32f b, const t_f32 amount) {
        ZF_ASSERT(amount >= 0.0f && amount <= 1.0f);

        return {
            lerp(a.r, b.r, amount),
            lerp(a.g, b.g, amount),
            lerp(a.b, b.b, amount),
            lerp(a.a, b.a, amount),
        };
    }

    constexpr t_f32 color_calc_luminance(const t_color_rgba32f col) {
        return (0.2126f * col.r) + (0.7152f * col.g) + (0.0722f * col.b);
    }

    constexpr t_color_rgba32f color_to_grayscale(const t_color_rgba32f col) {
        const t_f32 lum = color_calc_luminance(col);
        return {lum, lum, lum, col.a};
    }

    constexpr t_u32 color_rgba8_to_hex(const t_color_rgba8 col) {
        t_u32 result = 0;
        result |= static_cast<t_u32>(col.r) << 24;
        result |= static_cast<t_u32>(col.g) << 16;
        result |= static_cast<t_u32>(col.b) << 8;
        result |= static_cast<t_u32>(col.a);

        return result;
    }

    constexpr t_color_rgba8 color_hex_to_rgba8(const t_u32 hex) {
        const auto r = static_cast<t_u8>((hex & 0xFF000000) >> 24);
        const auto g = static_cast<t_u8>((hex & 0x00FF0000) >> 16);
        const auto b = static_cast<t_u8>((hex & 0x0000FF00) >> 8);
        const auto a = static_cast<t_u8>(hex & 0x000000FF);

        return {r, g, b, a};
    }
}
