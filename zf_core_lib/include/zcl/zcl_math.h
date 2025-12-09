#pragma once

#include <zcl/zcl_mem.h>

#include <cmath>

namespace zf {
    constexpr t_f32 g_pi = 3.14159265358979323846f;
    constexpr t_f32 g_tau = 6.28318530717958647692f;

    constexpr t_f32 DegsToRads(const t_f32 degs) {
        return degs * (g_pi / 180.0f);
    }

    constexpr t_f32 RadsToDegs(const t_f32 rads) {
        return rads * (180.0f / g_pi);
    }

    template <c_integral tp_type>
    constexpr t_len DigitCnt(const tp_type n) {
        if (n < 0) {
            return DigitCnt(-n);
        }

        if (n < 10) {
            return 1;
        }

        return 1 + DigitCnt(n / 10);
    }

    // Gives the digit at the given index, where the indexes are from the least
    // significant digit to the most.
    template <c_integral tp_type>
    constexpr tp_type DigitAt(const tp_type n, const t_len index) {
        ZF_ASSERT(index >= 0 && index < DigitCnt(n));

        if (n < 0) {
            return DigitAt(-n, index);
        }

        if (index == 0) {
            return n % 10;
        }

        if (n < 10) {
            return 0;
        }

        return DigitAt(n / 10, index - 1);
    }

    template <c_floating_point tp_type>
    constexpr t_b8 IsNearlyEqual(const tp_type val, const tp_type targ, const tp_type tol = 1e-5) {
        ZF_ASSERT(tol >= 0);
        return val >= targ - tol && val <= targ + tol;
    }

    // ============================================================
    // @section: Vectors
    // ============================================================
    struct s_v2 {
        t_f32 x;
        t_f32 y;

        constexpr s_v2() = default;
        constexpr s_v2(const t_f32 x, const t_f32 y) : x(x), y(y) {}

        constexpr s_v2 operator+(const s_v2 other) const {
            return {x + other.x, y + other.y};
        }

        constexpr s_v2 operator-(const s_v2 other) const {
            return {x - other.x, y - other.y};
        }

        constexpr s_v2 operator*(const t_f32 scalar) const {
            return {x * scalar, y * scalar};
        }

        constexpr s_v2 operator/(const t_f32 scalar) const {
            return {x / scalar, y / scalar};
        }

        constexpr s_v2 &operator+=(const s_v2 other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr s_v2 &operator-=(const s_v2 other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr s_v2 &operator*=(const t_f32 scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        constexpr s_v2 &operator/=(const t_f32 scalar) {
            x /= scalar;
            y /= scalar;
            return *this;
        }
    };

    constexpr s_v2 operator*(const t_f32 scalar, const s_v2 v) {
        return {v.x * scalar, v.y * scalar};
    }

    struct s_v2_i {
        t_i32 x;
        t_i32 y;

        constexpr s_v2_i() = default;
        constexpr s_v2_i(const t_i32 x, const t_i32 y) : x(x), y(y) {}

        constexpr s_v2_i operator+(const s_v2_i other) const {
            return {x + other.x, y + other.y};
        }

        constexpr s_v2_i operator-(const s_v2_i other) const {
            return {x - other.x, y - other.y};
        }

        constexpr s_v2_i &operator+=(const s_v2_i other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr s_v2_i &operator-=(const s_v2_i other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr t_b8 operator==(const s_v2_i &other) const {
            return x == other.x && y == other.y;
        }

        constexpr t_b8 operator!=(const s_v2_i &other) const {
            return !(*this == other);
        }
    };

    constexpr s_v2 ToV2(const s_v2_i v) {
        return {static_cast<t_f32>(v.x), static_cast<t_f32>(v.y)};
    }

    constexpr s_v2_i ToV2I(const s_v2 v) {
        return {static_cast<t_i32>(v.x), static_cast<t_i32>(v.y)};
    }

    constexpr s_v2 CompwiseProd(const s_v2 a, const s_v2 b) {
        return {a.x * b.x, a.y * b.y};
    }

    constexpr t_f32 DotProd(const s_v2 a, const s_v2 b) {
        return (a.x * b.x) + (a.y * b.y);
    }

    inline t_f32 CalcMag(const s_v2 v) {
        return sqrt((v.x * v.x) + (v.y * v.y));
    }

    inline s_v2 CalcNormalOrZero(const s_v2 v) {
        const t_f32 mag = CalcMag(v);

        if (mag == 0.0f) {
            return {};
        }

        return {v.x / mag, v.y / mag};
    }

    inline t_f32 CalcDist(const s_v2 a, const s_v2 b) {
        return CalcMag({b.x - a.x, b.y - a.y});
    }

    inline s_v2 CalcDir(const s_v2 a, const s_v2 b) {
        return CalcNormalOrZero({b.x - a.x, b.y - a.y});
    }

    inline t_f32 CalcDirInRads(const s_v2 a, const s_v2 b) {
        return atan2(-(b.y - a.y), b.x - a.x);
    }

    inline s_v2 LenDir(const t_f32 len, const t_f32 dir) {
        return s_v2(cos(dir), -sin(dir)) * len;
    }

    struct s_v3 {
        t_f32 x;
        t_f32 y;
        t_f32 z;
    };

    struct s_v4 {
        t_f32 x;
        t_f32 y;
        t_f32 z;
        t_f32 w;
    };

    // ============================================================
    // @section: Rectangles
    // ============================================================
    struct s_rect_f {
        t_f32 x;
        t_f32 y;
        t_f32 width;
        t_f32 height;

        constexpr s_rect_f() = default;
        constexpr s_rect_f(const t_f32 x, const t_f32 y, const t_f32 width, const t_f32 height) : x(x), y(y), width(width), height(height) {}
        constexpr s_rect_f(const s_v2 pos, const s_v2 size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}
    };

    struct s_rect_i {
        t_i32 x;
        t_i32 y;
        t_i32 width;
        t_i32 height;

        constexpr s_rect_i() = default;
        constexpr s_rect_i(const t_i32 x, const t_i32 y, const t_i32 width, const t_i32 height) : x(x), y(y), width(width), height(height) {}
        constexpr s_rect_i(const s_v2_i pos, const s_v2_i size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

        constexpr t_b8 operator==(const s_rect_i other) const {
            return x == other.x && y == other.y && width == other.width && height == other.height;
        }

        constexpr t_b8 operator!=(const s_rect_i other) const {
            return !(*this == other);
        }
    };

    constexpr s_rect_f ToRectF(const s_rect_i rect) {
        return {static_cast<t_f32>(rect.x), static_cast<t_f32>(rect.y), static_cast<t_f32>(rect.width), static_cast<t_f32>(rect.height)};
    }

    constexpr s_rect_i ToRectI(const s_rect_f rect) {
        return {static_cast<t_i32>(rect.x), static_cast<t_i32>(rect.y), static_cast<t_i32>(rect.width), static_cast<t_i32>(rect.height)};
    }

    constexpr s_v2 RectPos(const s_rect_f rect) {
        return {rect.x, rect.y};
    }

    constexpr s_v2_i RectPos(const s_rect_i rect) {
        return {rect.x, rect.y};
    }

    constexpr s_v2 RectSize(const s_rect_f rect) {
        return {rect.width, rect.height};
    }

    constexpr s_v2 RectCenter(const s_rect_f rect) {
        return {RectPos(rect) + (RectSize(rect) / 2.0f)};
    }

    constexpr s_v2_i RectSize(const s_rect_i rect) {
        return {rect.width, rect.height};
    }

    constexpr t_f32 RectLeft(const s_rect_f rect) {
        return rect.x;
    }

    constexpr t_i32 RectLeft(const s_rect_i rect) {
        return rect.x;
    }

    constexpr t_f32 RectTop(const s_rect_f rect) {
        return rect.y;
    }

    constexpr t_i32 RectTop(const s_rect_i rect) {
        return rect.y;
    }

    constexpr t_f32 RectRight(const s_rect_f rect) {
        return rect.x + rect.width;
    }

    constexpr t_i32 RectRight(const s_rect_i rect) {
        return rect.x + rect.width;
    }

    constexpr t_f32 RectBottom(const s_rect_f rect) {
        return rect.y + rect.height;
    }

    constexpr t_i32 RectBottom(const s_rect_i rect) {
        return rect.y + rect.height;
    }

    constexpr t_f32 RectArea(const s_rect_f rect) {
        return rect.width * rect.height;
    }

    constexpr t_i32 RectArea(const s_rect_i rect) {
        return rect.width * rect.height;
    }

    constexpr s_rect_f RectClamped(const s_rect_f rect, const s_rect_f container) {
        const s_v2 tl = {ZF_MAX(rect.x, container.x), ZF_MAX(rect.y, container.y)};
        return {tl.x, tl.y, ZF_MIN(RectRight(rect), RectRight(container)) - tl.x, ZF_MIN(RectBottom(rect), RectBottom(container)) - tl.y};
    }

    constexpr s_rect_i RectClamped(const s_rect_i rect, const s_rect_i container) {
        const s_v2_i tl = {ZF_MAX(rect.x, container.x), ZF_MAX(rect.y, container.y)};
        return {tl.x, tl.y, ZF_MIN(RectRight(rect), RectRight(container)) - tl.x, ZF_MIN(RectBottom(rect), RectBottom(container)) - tl.y};
    }

    constexpr t_f32 CalcRectOccupancyPerc(const s_rect_f a, const s_rect_f b) {
        const auto subrect = RectClamped(a, b);
        return Clamp(static_cast<t_f32>(RectArea(subrect)) / RectArea(b), 0.0f, 1.0f);
    }

    constexpr t_i32 CalcRectOccupancyPerc(const s_rect_i a, const s_rect_i b) {
        const auto subrect = RectClamped(a, b);
        return Clamp(static_cast<t_i32>(RectArea(subrect)) / RectArea(b), 0, 1);
    }

    constexpr t_b8 DoesRectContainPoint(const s_rect_f rect, const s_v2 pt) {
        return pt.x > RectLeft(rect) && pt.y > RectTop(rect) && pt.x < RectRight(rect) && pt.y < RectBottom(rect);
    }

    constexpr t_b8 DoesRectContainPoint(const s_rect_i rect, const s_v2_i pt) {
        return pt.x > RectLeft(rect) && pt.y > RectTop(rect) && pt.x < RectRight(rect) && pt.y < RectBottom(rect);
    }

    constexpr t_b8 DoRectsIntersect(const s_rect_f a, const s_rect_f b) {
        return RectLeft(a) < RectRight(b) && RectTop(a) < RectBottom(b) && RectRight(a) > RectLeft(b) && RectBottom(a) > RectTop(b);
    }

    constexpr t_b8 DoRectsIntersect(const s_rect_i a, const s_rect_i b) {
        return RectLeft(a) < RectRight(b) && RectTop(a) < RectBottom(b) && RectRight(a) > RectLeft(b) && RectBottom(a) > RectTop(b);
    }

    constexpr s_v2 ClampPointInRect(const s_v2 pt, const s_rect_f rect) {
        return {Clamp(pt.x, RectLeft(rect), RectRight(rect)), Clamp(pt.y, RectTop(rect), RectBottom(rect))};
    }

    constexpr s_v2_i ClampPointInRect(const s_v2_i pt, const s_rect_i rect) {
        return {Clamp(pt.x, RectLeft(rect), RectRight(rect)), Clamp(pt.y, RectTop(rect), RectBottom(rect))};
    }

    // Generate a rectangle encompassing all of the provided rectangles. At least a single
    // rectangle must be provided.
    template <c_nonstatic_array tp_type>
    s_rect_f CalcSpanningRect(const tp_type rects) {
        static_assert(s_is_same<typename tp_type::t_elem, s_rect_f>::g_val || s_is_same<typename tp_type::t_elem, s_rect_i>::g_val);

        ZF_ASSERT(rects.len > 0);

        auto min_left = rects[0].x;
        auto min_top = rects[0].y;
        auto max_right = RectRight(rects[0]);
        auto max_bottom = RectBottom(rects[0]);

        for (t_len i = 1; i < rects.len; i++) {
            min_left = ZF_MIN(rects[i].x, min_left);
            min_top = ZF_MIN(rects[i].y, min_top);
            max_right = ZF_MAX(RectRight(rects[i]), max_right);
            max_bottom = ZF_MAX(RectBottom(rects[i]), max_bottom);
        }

        return {min_left, min_top, max_right - min_left, max_bottom - min_top};
    }

    // ============================================================
    // @section: Matrices
    // ============================================================
    struct s_mat4x4 {
        s_static_array<s_static_array<t_f32, 4>, 4> elems;
    };

    constexpr s_mat4x4 CreateIdentityMatrix() {
        s_mat4x4 mat = {};

        mat.elems[0][0] = 1.0f;
        mat.elems[1][1] = 1.0f;
        mat.elems[2][2] = 1.0f;
        mat.elems[3][3] = 1.0f;

        return mat;
    }

    constexpr s_mat4x4 CreateOrthographicMatrix(const t_f32 left, const t_f32 right, const t_f32 bottom, const t_f32 top, const t_f32 z_near, const t_f32 z_far) {
        ZF_ASSERT(right > left);
        ZF_ASSERT(top < bottom);
        ZF_ASSERT(z_far > z_near);
        ZF_ASSERT(z_near < z_far);

        s_mat4x4 mat = {};
        mat.elems[0][0] = 2.0f / (right - left);
        mat.elems[1][1] = 2.0f / (top - bottom);
        mat.elems[2][2] = -2.0f / (z_far - z_near);
        mat.elems[3][0] = -(right + left) / (right - left);
        mat.elems[3][1] = -(top + bottom) / (top - bottom);
        mat.elems[3][2] = -(z_far + z_near) / (z_far - z_near);
        mat.elems[3][3] = 1.0f;
        return mat;
    }

    // ============================================================

    template <c_floating_point tp_type>
    constexpr tp_type Lerp(const tp_type a, const tp_type b, const tp_type t) {
        return a + ((b - a) * t);
    }

    constexpr s_v2 Lerp(const s_v2 a, const s_v2 b, const t_f32 t) {
        return a + ((b - a) * t);
    }
}
