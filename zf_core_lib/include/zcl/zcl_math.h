#pragma once

#include <cmath>
#include <zcl/zcl_mem.h>

namespace zcl::math {
    constexpr t_f32 k_pi = 3.14159265358979323846f;
    constexpr t_f32 k_tau = 6.28318530717958647692f;

    constexpr t_f32 degs_to_rads(const t_f32 degs) {
        return degs * (k_pi / 180.0f);
    }

    constexpr t_f32 rads_to_degs(const t_f32 rads) {
        return rads * (180.0f / k_pi);
    }

    template <c_integral tp_type>
    constexpr t_i32 calc_digit_cnt(const tp_type n) {
        if (n < 0) {
            return calc_digit_cnt(-n);
        }

        if (n < 10) {
            return 1;
        }

        return 1 + calc_digit_cnt(n / 10);
    }

    // Determines the digit at the given index, where the indexes are from the least significant digit to the most.
    template <c_integral tp_type>
    constexpr tp_type calc_digit_at(const tp_type n, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < calc_digit_cnt(n));

        if (n < 0) {
            return calc_digit_at(-n, index);
        }

        if (index == 0) {
            return n % 10;
        }

        if (n < 10) {
            return 0;
        }

        return calc_digit_at(n / 10, index - 1);
    }

    constexpr t_b8 check_nearly_equal(const t_f32 val, const t_f32 targ, const t_f32 tol = 1e-5f) {
        ZF_ASSERT(tol >= 0);
        return val >= targ - tol && val <= targ + tol;
    }


    // ============================================================
    // @section: Vectors

    struct t_v2 {
        t_f32 x;
        t_f32 y;

        constexpr t_v2 operator+(const t_v2 &other) const { return {x + other.x, y + other.y}; }
        constexpr t_v2 operator-(const t_v2 &other) const { return {x - other.x, y - other.y}; }
        constexpr t_v2 operator*(const t_f32 scalar) const { return {x * scalar, y * scalar}; }
        constexpr t_v2 operator/(const t_f32 divisor) const { return {x / divisor, y / divisor}; }

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

        constexpr t_v2 &operator/=(const t_f32 divisor) {
            x /= divisor;
            y /= divisor;
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
        constexpr t_v2_i operator*(const t_i32 scalar) const { return {x * scalar, y * scalar}; }
        constexpr t_v2_i operator/(const t_i32 divisor) const { return {x / divisor, y / divisor}; }

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

        constexpr t_v2_i &operator*=(const t_i32 scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        constexpr t_v2_i &operator/=(const t_i32 divisor) {
            x /= divisor;
            y /= divisor;
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

    constexpr t_v2 v2_i_to_f(const t_v2_i v) {
        return {static_cast<t_f32>(v.x), static_cast<t_f32>(v.y)};
    }

    constexpr t_v2_i v2_f_to_i(const t_v2 v) {
        return {static_cast<t_i32>(v.x), static_cast<t_i32>(v.y)};
    }

    constexpr t_f32 v2_calc_dot_prod(const t_v2 a, const t_v2 b) { return (a.x * b.x) + (a.y * b.y); }
    constexpr t_i32 v2_calc_dot_prod(const t_v2_i a, const t_v2_i b) { return (a.x * b.x) + (a.y * b.y); }

    constexpr t_f32 v2_calc_cross_prod(const t_v2 a, const t_v2 b) { return (a.x * b.y) - (a.y * b.x); }
    constexpr t_i32 v2_calc_cross_prod(const t_v2_i a, const t_v2_i b) { return (a.x * b.y) - (a.y * b.x); }

    constexpr t_v2 v2_calc_compwise_prod(const t_v2 a, const t_v2 b) { return {a.x * b.x, a.y * b.y}; }
    constexpr t_v2_i v2_calc_compwise_prod(const t_v2_i a, const t_v2_i b) { return {a.x * b.x, a.y * b.y}; }

    inline t_f32 v2_calc_mag(const t_v2 v) {
        return sqrt((v.x * v.x) + (v.y * v.y));
    }

    // Returns {} if a divide by 0 is attempted.
    inline t_v2 v2_calc_normal(const t_v2 v) {
        const t_f32 mag = v2_calc_mag(v);

        if (mag == 0.0f) {
            return {};
        }

        return {v.x / mag, v.y / mag};
    }

    // ============================================================


    // ============================================================
    // @section: Rectangles

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

    constexpr t_rect_f rect_create_f32(const t_f32 x, const t_f32 y, const t_f32 width, const t_f32 height) {
        ZF_ASSERT(width >= 0.0f && height >= 0.0f);
        return {x, y, width, height};
    }

    constexpr t_rect_f rect_create_f32(const t_v2 pos, const t_v2 size) {
        ZF_ASSERT(size.x >= 0.0f && size.y >= 0.0f);
        return {pos.x, pos.y, size.x, size.y};
    }

    constexpr t_rect_i rect_create_i32(const t_i32 x, const t_i32 y, const t_i32 width, const t_i32 height) {
        ZF_ASSERT(width >= 0 && height >= 0);
        return {x, y, width, height};
    }

    constexpr t_rect_i rect_create_i32(const t_v2_i pos, const t_v2_i size) {
        ZF_ASSERT(size.x >= 0 && size.y >= 0);
        return {pos.x, pos.y, size.x, size.y};
    }

    constexpr t_v2 rect_get_pos(const t_rect_f rect) { return {rect.x, rect.y}; }
    constexpr t_v2_i rect_get_pos(const t_rect_i rect) { return {rect.x, rect.y}; }

    constexpr t_v2 rect_get_size(const t_rect_f rect) { return {rect.width, rect.height}; }
    constexpr t_v2_i rect_get_size(const t_rect_i rect) { return {rect.width, rect.height}; }

    constexpr t_v2 rect_get_center(const t_rect_f rect) { return {rect.x + (rect.width / 2.0f), rect.y + (rect.height / 2.0f)}; }

    constexpr t_f32 rect_get_left(const t_rect_f rect) { return rect.x; }
    constexpr t_i32 rect_get_left(const t_rect_i rect) { return rect.x; }

    constexpr t_f32 rect_get_top(const t_rect_f rect) { return rect.y; }
    constexpr t_i32 rect_get_top(const t_rect_i rect) { return rect.y; }

    constexpr t_f32 rect_get_right(const t_rect_f rect) { return rect.x + rect.width; }
    constexpr t_i32 rect_get_right(const t_rect_i rect) { return rect.x + rect.width; }

    constexpr t_f32 rect_get_bottom(const t_rect_f rect) { return rect.y + rect.height; }
    constexpr t_i32 rect_get_bottom(const t_rect_i rect) { return rect.y + rect.height; }

    constexpr t_v2 rect_get_topleft(const t_rect_f rect) { return {rect_get_left(rect), rect_get_top(rect)}; }
    constexpr t_v2_i rect_get_topleft(const t_rect_i rect) { return {rect_get_left(rect), rect_get_top(rect)}; }

    constexpr t_v2 rect_get_topright(const t_rect_f rect) { return {rect_get_right(rect), rect_get_top(rect)}; }
    constexpr t_v2_i rect_get_topright(const t_rect_i rect) { return {rect_get_right(rect), rect_get_top(rect)}; }

    constexpr t_v2 rect_get_bottomleft(const t_rect_f rect) { return {rect_get_left(rect), rect_get_bottom(rect)}; }
    constexpr t_v2_i rect_get_bottomleft(const t_rect_i rect) { return {rect_get_left(rect), rect_get_bottom(rect)}; }

    constexpr t_v2 rect_get_bottomright(const t_rect_f rect) { return {rect_get_right(rect), rect_get_bottom(rect)}; }
    constexpr t_v2_i rect_get_bottomright(const t_rect_i rect) { return {rect_get_right(rect), rect_get_bottom(rect)}; }

    constexpr t_f32 rect_get_area(const t_rect_f rect) { return rect.width * rect.height; }
    constexpr t_i32 rect_get_area(const t_rect_i rect) { return rect.width * rect.height; }

    constexpr t_rect_f rect_i_to_f(const t_rect_i rect) {
        return {static_cast<t_f32>(rect.x), static_cast<t_f32>(rect.y), static_cast<t_f32>(rect.width), static_cast<t_f32>(rect.height)};
    }

    constexpr t_rect_i rect_f_to_i(const t_rect_f rect) {
        return {static_cast<t_i32>(rect.x), static_cast<t_i32>(rect.y), static_cast<t_i32>(rect.width), static_cast<t_i32>(rect.height)};
    }

    constexpr t_b8 rects_check_equal(const t_rect_i a, const t_rect_i b) {
        return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
    }

    constexpr t_b8 rects_check_inters(const t_rect_f a, const t_rect_f b) {
        return rect_get_left(a) < rect_get_right(b) && rect_get_top(a) < rect_get_bottom(b) && rect_get_right(a) > rect_get_left(b) && rect_get_bottom(a) > rect_get_top(b);
    }

    constexpr t_b8 rects_check_inters(const t_rect_i a, const t_rect_i b) {
        return rect_get_left(a) < rect_get_right(b) && rect_get_top(a) < rect_get_bottom(b) && rect_get_right(a) > rect_get_left(b) && rect_get_bottom(a) > rect_get_top(b);
    }

    t_rect_f rects_calc_span(const t_array_mut<t_rect_f> rects);
    t_rect_i rects_calc_span(const t_array_mut<t_rect_i> rects);

    // ============================================================


    // ============================================================
    // @section: Matrices

    struct t_mat4x4 {
        t_static_array<t_static_array<t_f32, 4>, 4> elems;
    };

    constexpr t_mat4x4 matrix_create_identity() {
        t_mat4x4 result = {};
        result.elems[0][0] = 1.0f;
        result.elems[1][1] = 1.0f;
        result.elems[2][2] = 1.0f;
        result.elems[3][3] = 1.0f;

        return result;
    }

    constexpr t_mat4x4 matrix_create_translated(const t_v2 offs) {
        t_mat4x4 result = matrix_create_identity();
        result.elems[0][3] = offs.x;
        result.elems[1][3] = offs.y;

        return result;
    }

    inline t_mat4x4 matrix_create_rotated(const t_f32 rot) {
        t_mat4x4 result = matrix_create_identity();
        result.elems[0][0] = cos(rot);
        result.elems[0][1] = -sin(rot);
        result.elems[1][0] = sin(rot);
        result.elems[1][1] = cos(rot);

        return result;
    }

    constexpr t_mat4x4 matrix_create_scaled(const t_v2 scalar) {
        t_mat4x4 result = matrix_create_identity();
        result.elems[0][0] *= scalar.x;
        result.elems[1][1] *= scalar.y;

        return result;
    }

    constexpr t_mat4x4 matrix_add(const t_mat4x4 &a, const t_mat4x4 &b) {
        t_mat4x4 result;

        for (t_i32 i = 0; i < 4; i++) {
            for (t_i32 j = 0; j < 4; j++) {
                result.elems[i][j] = a.elems[i][j] + b.elems[i][j];
            }
        }

        return result;
    }

    constexpr t_mat4x4 matrix_subtract(const t_mat4x4 &a, const t_mat4x4 &b) {
        t_mat4x4 result;

        for (t_i32 i = 0; i < 4; i++) {
            for (t_i32 j = 0; j < 4; j++) {
                result.elems[i][j] = a.elems[i][j] - b.elems[i][j];
            }
        }

        return result;
    }

    constexpr t_mat4x4 matrix_multiply(const t_mat4x4 &a, const t_mat4x4 &b) {
        t_mat4x4 result = {};

        for (t_i32 i = 0; i < 4; i++) {
            for (t_i32 j = 0; j < 4; j++) {
                for (t_i32 k = 0; k < 4; k++) {
                    result.elems[i][j] += (a.elems[i][k] * b.elems[k][j]);
                }
            }
        }

        return result;
    }

    // ============================================================


    // ============================================================
    // @section: Polygons

    struct t_poly_rdonly {
        t_array_rdonly<t_v2> pts;
    };

    struct t_poly_mut {
        t_array_mut<t_v2> pts;

        operator t_poly_rdonly() const {
            return {.pts = pts};
        }
    };

    // Points are guaranteed to be in this order: top-left, top-right, bottom-right, bottom-left.
    t_poly_mut poly_create_quad(const t_v2 pos, const t_v2 size, const t_v2 origin, mem::t_arena *const arena);

    // Points are guaranteed to be in this order: top-left, top-right, bottom-right, bottom-left.
    t_poly_mut poly_create_quad_rotated(const t_v2 pos, const t_v2 size, const t_v2 origin, const t_f32 rot, mem::t_arena *const arena);

    t_b8 polys_check_inters(const t_poly_rdonly a, const t_poly_rdonly b);
    t_b8 poly_check_inters_with_rect(const t_poly_rdonly poly, const t_rect_f rect);

    t_rect_f poly_calc_span(const t_poly_rdonly poly);

    // ============================================================


    constexpr t_f32 lerp(const t_f32 a, const t_f32 b, const t_f32 t) { return a + ((b - a) * t); }
    constexpr t_v2 lerp(const t_v2 a, const t_v2 b, const t_f32 t) { return a + ((b - a) * t); }

    inline t_f32 calc_dist(const t_v2 a, const t_v2 b) {
        return v2_calc_mag(b - a);
    }

    inline t_v2 calc_dir(const t_v2 a, const t_v2 b) {
        return v2_calc_normal(b - a);
    }

    // Returns 0 if the horizontal and vertical differences of the vectors are 0.
    inline t_f32 calc_dir_in_rads(const t_v2 a, const t_v2 b) {
        const t_f32 rise = b.y - a.y;
        const t_f32 run = b.x - a.x;

        if (rise == 0.0f && run == 0.0f) {
            return 0.0f;
        }

        return atan2(rise, run);
    }

    inline t_v2 calc_lengthdir(const t_f32 len, const t_f32 dir) {
        return t_v2{cos(dir), sin(dir)} * len;
    }

    constexpr t_b8 check_pt_in_rect(const t_v2 pt, const t_rect_f rect) {
        return pt.x > rect_get_left(rect) && pt.y > rect_get_top(rect) && pt.x < rect_get_right(rect) && pt.y < rect_get_bottom(rect);
    }

    constexpr t_b8 check_pt_in_rect(const t_v2_i pt, const t_rect_i rect) {
        return pt.x > rect_get_left(rect) && pt.y > rect_get_top(rect) && pt.x < rect_get_right(rect) && pt.y < rect_get_bottom(rect);
    }

    constexpr t_v2 clamp_within_container(const t_v2 pt, const t_rect_f container) {
        return {clamp(pt.x, rect_get_left(container), rect_get_right(container)), clamp(pt.y, rect_get_top(container), rect_get_bottom(container))};
    }

    constexpr t_v2_i clamp_within_container(const t_v2_i pt, const t_rect_i container) {
        return {clamp(pt.x, rect_get_left(container), rect_get_right(container)), clamp(pt.y, rect_get_top(container), rect_get_bottom(container))};
    }

    constexpr t_rect_f clamp_within_container(const t_rect_f rect, const t_rect_f container) {
        const t_v2 tl = {max(rect.x, container.x), max(rect.y, container.y)};
        return {tl.x, tl.y, max(min(rect_get_right(rect), rect_get_right(container)) - tl.x, 0.0f), max(min(rect_get_bottom(rect), rect_get_bottom(container)) - tl.y, 0.0f)};
    }

    constexpr t_rect_i clamp_within_container(const t_rect_i rect, const t_rect_i container) {
        const t_v2_i tl = {max(rect.x, container.x), max(rect.y, container.y)};
        return {tl.x, tl.y, max(min(rect_get_right(rect), rect_get_right(container)) - tl.x, 0), max(min(rect_get_bottom(rect), rect_get_bottom(container)) - tl.y, 0)};
    }

    // Returns a value between 0 and 1 indicating what percentage of the rectangle is within the container.
    constexpr t_f32 calc_perc_of_occupance(const t_rect_f rect, const t_rect_f container) {
        ZF_ASSERT(container.width > 0 && container.height > 0);

        const auto subrect = clamp_within_container(rect, container);
        return clamp(rect_get_area(subrect) / rect_get_area(container), 0.0f, 1.0f);
    }

    // Returns a value between 0 and 1 indicating what percentage of the rectangle is within the container.
    constexpr t_f32 calc_perc_of_occupance(const t_rect_i rect, const t_rect_i container) {
        ZF_ASSERT(container.width > 0 && container.height > 0);

        const auto subrect = clamp_within_container(rect, container);
        return clamp(static_cast<t_f32>(rect_get_area(subrect)) / static_cast<t_f32>(rect_get_area(container)), 0.0f, 1.0f);
    }
}
