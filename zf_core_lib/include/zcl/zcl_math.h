#pragma once

#include <cmath>
#include <zcl/zcl_mem.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    constexpr t_f32 g_math_pi = 3.14159265358979323846f;
    constexpr t_f32 g_math_tau = 6.28318530717958647692f;

    struct t_v2 {
        t_f32 x;
        t_f32 y;

        constexpr t_v2 operator+(const t_v2 &other) const { return {x + other.x, y + other.y}; }
        constexpr t_v2 operator-(const t_v2 &other) const { return {x - other.x, y - other.y}; }
        constexpr t_v2 operator*(const t_f32 scalar) const { return {x * scalar, y * scalar}; }
        constexpr t_v2 operator/(const t_f32 scalar) const { return {x / scalar, y / scalar}; }

        constexpr t_v2 &operator+=(const t_v2 &other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr t_v2 &operator-=(const t_v2 &other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr t_v2 &operator*=(const t_f32 scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        constexpr t_v2 &operator/=(const t_f32 scalar) {
            x /= scalar;
            y /= scalar;
            return *this;
        }
    };

    constexpr t_v2 operator*(const t_f32 scalar, const t_v2 v) {
        return {v.x * scalar, v.y * scalar};
    }

    struct t_v2_i {
        t_i32 x;
        t_i32 y;

        constexpr t_b8 operator==(const t_v2_i &other) const { return x == other.x && y == other.y; }
        constexpr t_b8 operator!=(const t_v2_i &other) const { return !(*this == other); }

        constexpr t_v2_i operator+(const t_v2_i &other) const { return {x + other.x, y + other.y}; }
        constexpr t_v2_i operator-(const t_v2_i &other) const { return {x - other.x, y - other.y}; }

        constexpr t_v2_i &operator+=(const t_v2_i &other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr t_v2_i &operator-=(const t_v2_i &other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }
    };

    struct t_v3 {
        t_f32 x;
        t_f32 y;
        t_f32 z;
    };

    struct t_v4 {
        t_f32 x;
        t_f32 y;
        t_f32 z;
        t_f32 w;
    };

    struct t_rect_f {
        t_f32 x;
        t_f32 y;
        t_f32 width;
        t_f32 height;
    };

    struct t_rect_i {
        t_i32 x;
        t_i32 y;
        t_i32 width;
        t_i32 height;
    };

    struct t_mat4x4 {
        t_static_array<t_static_array<t_f32, 4>, 4> elems;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    constexpr t_f32 f_math_degs_to_rads(const t_f32 degs) {
        return degs * (g_math_pi / 180.0f);
    }

    constexpr t_f32 f_math_rads_to_degs(const t_f32 rads) {
        return rads * (180.0f / g_math_pi);
    }

    template <c_integral tp_type>
    t_i32 f_math_calc_digit_cnt(const tp_type n) {
        if (n < 0) {
            return f_math_calc_digit_cnt(-n);
        }

        if (n < 10) {
            return 1;
        }

        return 1 + f_math_calc_digit_cnt(n / 10);
    }

    // Determines the digit at the given index, where the indexes are from the least significant digit to the most.
    template <c_integral tp_type>
    tp_type f_math_determine_digit_at(const tp_type n, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < f_math_calc_digit_cnt(n));

        if (n < 0) {
            return f_math_determine_digit_at(-n, index);
        }

        if (index == 0) {
            return n % 10;
        }

        if (n < 10) {
            return 0;
        }

        return f_math_determine_digit_at(n / 10, index - 1);
    }

    inline t_b8 f_math_is_nearly_equal(const t_f32 val, const t_f32 targ, const t_f32 tol = 1e-5f) {
        ZF_ASSERT(tol >= 0);
        return val >= targ - tol && val <= targ + tol;
    }

    inline t_v2 f_math_create_v2(const t_f32 x, const t_f32 y) {
        return {x, y};
    }

    inline t_v2_i f_math_create_v2_i(const t_i32 x, const t_i32 y) {
        return {x, y};
    }

    inline t_v2 f_math_convert_to_v2(const t_v2_i v) {
        return {static_cast<t_f32>(v.x), static_cast<t_f32>(v.y)};
    }

    inline t_rect_f f_math_create_f(const t_f32 x, const t_f32 y, const t_f32 width, const t_f32 height) {
        return {x, y, width, height};
    }

    inline t_rect_f f_math_create_rect_f(const t_v2 pos, const t_v2 size) {
        return {pos.x, pos.y, size.x, size.y};
    }

    inline t_rect_f f_math_create_rect_f(const t_v2 pos, const t_v2 size, const t_v2 origin) {
        return {pos.x - (size.x * origin.x), pos.y - (size.y * origin.y), size.x, size.y};
    }

    inline t_rect_i f_math_create_rect_i(const t_i32 x, const t_i32 y, const t_i32 width, const t_i32 height) {
        return {x, y, width, height};
    }

    inline t_rect_i f_math_create_rect_i(const t_v2_i pos, const t_v2_i size) {
        return {pos.x, pos.y, size.x, size.y};
    }

    inline t_v2 f_math_get_rect_pos(const t_rect_f rect) { return {rect.x, rect.y}; }
    inline t_v2_i f_math_get_rect_pos(const t_rect_i rect) { return {rect.x, rect.y}; }

    inline t_v2 f_math_get_rect_size(const t_rect_f rect) { return {rect.width, rect.height}; }
    inline t_v2_i f_math_get_rect_size(const t_rect_i rect) { return {rect.width, rect.height}; }

    inline t_v2 f_math_get_rect_center(const t_rect_f rect) { return {rect.x + (rect.width / 2.0f), rect.y + (rect.height / 2.0f)}; }

    inline t_f32 f_math_get_rect_left(const t_rect_f rect) { return rect.x; }
    inline t_i32 f_math_get_rect_left(const t_rect_i rect) { return rect.x; }

    inline t_f32 f_math_get_rect_top(const t_rect_f rect) { return rect.y; }
    inline t_i32 f_math_get_rect_top(const t_rect_i rect) { return rect.y; }

    inline t_f32 f_math_get_rect_right(const t_rect_f rect) { return rect.x + rect.width; }
    inline t_i32 f_math_get_rect_right(const t_rect_i rect) { return rect.x + rect.width; }

    inline t_f32 f_math_get_rect_bottom(const t_rect_f rect) { return rect.y + rect.height; }
    inline t_i32 f_math_get_rect_bottom(const t_rect_i rect) { return rect.y + rect.height; }

    inline t_v2 f_math_get_rect_topleft(const t_rect_f rect) { return {f_math_get_rect_left(rect), f_math_get_rect_top(rect)}; }
    inline t_v2_i f_math_get_rect_topleft(const t_rect_i rect) { return {f_math_get_rect_left(rect), f_math_get_rect_top(rect)}; }

    inline t_v2 f_math_get_rect_topright(const t_rect_f rect) { return {f_math_get_rect_right(rect), f_math_get_rect_top(rect)}; }
    inline t_v2_i f_math_get_rect_topright(const t_rect_i rect) { return {f_math_get_rect_right(rect), f_math_get_rect_top(rect)}; }

    inline t_v2 f_math_get_rect_bottomleft(const t_rect_f rect) { return {f_math_get_rect_left(rect), f_math_get_rect_bottom(rect)}; }
    inline t_v2_i f_math_get_rect_bottomleft(const t_rect_i rect) { return {f_math_get_rect_left(rect), f_math_get_rect_bottom(rect)}; }

    inline t_v2 f_math_get_rect_bottomright(const t_rect_f rect) { return {f_math_get_rect_right(rect), f_math_get_rect_bottom(rect)}; }
    inline t_v2_i f_math_get_rect_bottomright(const t_rect_i rect) { return {f_math_get_rect_right(rect), f_math_get_rect_bottom(rect)}; }

    inline t_f32 f_math_get_rect_area(const t_rect_f rect) { return rect.width * rect.height; }
    inline t_i32 f_math_get_rect_area(const t_rect_i rect) { return rect.width * rect.height; }

    inline t_b8 f_math_are_rects_equal(const t_rect_i a, const t_rect_i b) {
        return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
    }

    inline t_rect_f f_math_convert_to_rect_f(const t_rect_i rect) {
        return {static_cast<t_f32>(rect.x), static_cast<t_f32>(rect.y), static_cast<t_f32>(rect.width), static_cast<t_f32>(rect.height)};
    }

    inline t_rect_i t_math_convert_to_rect_i(const t_rect_f rect) {
        return {static_cast<t_i32>(rect.x), static_cast<t_i32>(rect.y), static_cast<t_i32>(rect.width), static_cast<t_i32>(rect.height)};
    }

    inline t_b8 f_math_do_rects_inters(const t_rect_f a, const t_rect_f b) {
        return f_math_get_rect_left(a) < f_math_get_rect_right(b) && f_math_get_rect_top(a) < f_math_get_rect_bottom(b) && f_math_get_rect_right(a) > f_math_get_rect_left(b) && f_math_get_rect_bottom(a) > f_math_get_rect_top(b);
    }

    inline t_b8 f_math_do_rects_inters(const t_rect_i a, const t_rect_i b) {
        return f_math_get_rect_left(a) < f_math_get_rect_right(b) && f_math_get_rect_top(a) < f_math_get_rect_bottom(b) && f_math_get_rect_right(a) > f_math_get_rect_left(b) && f_math_get_rect_bottom(a) > f_math_get_rect_top(b);
    }

    // @todo: Make a constant.
    inline t_mat4x4 f_math_create_identity_matrix() {
        t_mat4x4 mat = {};
        mat.elems[0][0] = 1.0f;
        mat.elems[1][1] = 1.0f;
        mat.elems[2][2] = 1.0f;
        mat.elems[3][3] = 1.0f;

        return mat;
    }

    inline t_f32 f_math_lerp(const t_f32 a, const t_f32 b, const t_f32 t) { return a + ((b - a) * t); }
    inline t_v2 f_math_lerp(const t_v2 a, const t_v2 b, const t_f32 t) { return a + ((b - a) * t); }

    inline t_v2 f_math_compwise_prod(const t_v2 a, const t_v2 b) { return {a.x * b.x, a.y * b.y}; }
    inline t_v2_i f_math_compwise_prod(const t_v2_i a, const t_v2_i b) { return {a.x * b.x, a.y * b.y}; }

    inline t_f32 f_math_dot_prod(const t_v2 a, const t_v2 b) { return (a.x * b.x) + (a.y * b.y); }
    inline t_i32 f_math_dot_prod(const t_v2_i a, const t_v2_i b) { return (a.x * b.x) + (a.y * b.y); }

    inline t_f32 f_math_calc_mag(const t_v2 v) {
        return sqrt((v.x * v.x) + (v.y * v.y));
    }

    // Returns {} if a divide by 0 is attempted.
    inline t_v2 f_math_calc_normal(const t_v2 v) {
        const t_f32 mag = f_math_calc_mag(v);

        if (mag == 0.0f) {
            return {};
        }

        return {v.x / mag, v.y / mag};
    }

    inline t_f32 f_math_calc_dist(const t_v2 a, const t_v2 b) {
        return f_math_calc_mag(b - a);
    }

    inline t_v2 f_math_calc_dir(const t_v2 a, const t_v2 b) {
        return f_math_calc_normal(b - a);
    }

    // Returns 0 if the horizontal and vertical differences of the vectors are 0.
    inline t_f32 f_math_calc_dir_in_rads(const t_v2 a, const t_v2 b) {
        const t_f32 rise = -(b.y - a.y);
        const t_f32 run = b.x - a.x;

        if (rise == 0.0f && run == 0.0f) {
            return 0.0f;
        }

        return atan2(rise, run);
    }

    inline t_v2 f_math_calc_len_dir(const t_f32 len, const t_f32 dir) {
        return t_v2(cos(dir), -sin(dir)) * len;
    }

    inline t_b8 f_math_is_pt_in_rect(const t_v2 pt, const t_rect_f rect) {
        return pt.x > f_math_get_rect_left(rect) && pt.y > f_math_get_rect_top(rect) && pt.x < f_math_get_rect_right(rect) && pt.y < f_math_get_rect_bottom(rect);
    }

    inline t_b8 f_math_is_pt_in_rect(const t_v2_i pt, const t_rect_i rect) {
        return pt.x > f_math_get_rect_left(rect) && pt.y > f_math_get_rect_top(rect) && pt.x < f_math_get_rect_right(rect) && pt.y < f_math_get_rect_bottom(rect);
    }

    inline t_v2 f_math_clamp_within_container(const t_v2 pt, const t_rect_f container) {
        return {f_clamp(pt.x, f_math_get_rect_left(container), f_math_get_rect_right(container)), f_clamp(pt.y, f_math_get_rect_top(container), f_math_get_rect_bottom(container))};
    }

    inline t_v2_i f_math_clamp_within_container(const t_v2_i pt, const t_rect_i container) {
        return {f_clamp(pt.x, f_math_get_rect_left(container), f_math_get_rect_right(container)), f_clamp(pt.y, f_math_get_rect_top(container), f_math_get_rect_bottom(container))};
    }

    inline t_rect_f f_math_clamp_within_container(const t_rect_f rect, const t_rect_f container) {
        const t_v2 tl = {f_max(rect.x, container.x), f_max(rect.y, container.y)};
        return {tl.x, tl.y, f_max(f_min(f_math_get_rect_right(rect), f_math_get_rect_right(container)) - tl.x, 0.0f), f_max(f_min(f_math_get_rect_bottom(rect), f_math_get_rect_bottom(container)) - tl.y, 0.0f)};
    }

    inline t_rect_i f_math_clamp_within_container(const t_rect_i rect, const t_rect_i container) {
        const t_v2_i tl = {f_max(rect.x, container.x), f_max(rect.y, container.y)};
        return {tl.x, tl.y, f_max(f_min(f_math_get_rect_right(rect), f_math_get_rect_right(container)) - tl.x, 0), f_max(f_min(f_math_get_rect_bottom(rect), f_math_get_rect_bottom(container)) - tl.y, 0)};
    }

    // Returns a value between 0 and 1 indicating what percentage of the rectangle is within the container.
    inline t_f32 f_math_calc_perc_of_occupance(const t_rect_f rect, const t_rect_f container) {
        ZF_ASSERT(container.width > 0 && container.height > 0);

        const auto subrect = f_math_clamp_within_container(rect, container);
        return f_clamp(f_math_get_rect_area(subrect) / f_math_get_rect_area(container), 0.0f, 1.0f);
    }

    // Returns a value between 0 and 1 indicating what percentage of the rectangle is within the container.
    inline t_f32 f_math_calc_perc_of_occupance(const t_rect_i rect, const t_rect_i container) {
        ZF_ASSERT(container.width > 0 && container.height > 0);

        const auto subrect = f_math_clamp_within_container(rect, container);
        return f_clamp(static_cast<t_f32>(f_math_get_rect_area(subrect)) / static_cast<t_f32>(f_math_get_rect_area(container)), 0.0f, 1.0f);
    }

    t_rect_f f_math_calc_spanning_rect(const t_array_mut<t_rect_f> rects);
    t_rect_i f_math_calc_spanning_rect(const t_array_mut<t_rect_i> rects);

    // ============================================================
}
