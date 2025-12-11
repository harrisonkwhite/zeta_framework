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

    constexpr t_b8 IsNearlyEqual(const t_f32 val, const t_f32 targ, const t_f32 tol = 1e-5f) {
        ZF_ASSERT(tol >= 0);
        return val >= targ - tol && val <= targ + tol;
    }

    // ============================================================
    // @section: Vectors
    // ============================================================
    struct s_v2 {
        t_f32 x = 0.0f;
        t_f32 y = 0.0f;

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

        inline t_f32 CalcMag() {
            return sqrt((x * x) + (y * y));
        }

        inline s_v2 CalcNormalOrZero() {
            const t_f32 mag = CalcMag();

            if (mag == 0.0f) {
                return {};
            }

            return {x / mag, y / mag};
        }
    };

    constexpr s_v2 operator*(const t_f32 scalar, const s_v2 v) {
        return {v.x * scalar, v.y * scalar};
    }

    struct s_v2_i {
        t_i32 x = 0;
        t_i32 y = 0;

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

    struct s_v3 {
        t_f32 x = 0.0f;
        t_f32 y = 0.0f;
        t_f32 z = 0.0f;

        constexpr s_v3() = default;
        constexpr s_v3(const t_f32 x, const t_f32 y, const t_f32 z) : x(x), y(y), z(z) {}
    };

    struct s_v4 {
        t_f32 x = 0.0f;
        t_f32 y = 0.0f;
        t_f32 z = 0.0f;
        t_f32 w = 0.0f;

        constexpr s_v4() = default;
        constexpr s_v4(const t_f32 x, const t_f32 y, const t_f32 z, const t_f32 w) : x(x), y(y), z(z), w(w) {}
    };

    // ============================================================
    // @section: Rectangles
    // ============================================================
    struct s_rect_i;

    struct s_rect_f {
        t_f32 x = 0.0f;
        t_f32 y = 0.0f;
        t_f32 width = 0.0f;
        t_f32 height = 0.0f;

        constexpr s_rect_f() = default;
        constexpr s_rect_f(const t_f32 x, const t_f32 y, const t_f32 width, const t_f32 height) : x(x), y(y), width(width), height(height) {}
        constexpr s_rect_f(const s_v2 pos, const s_v2 size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

        constexpr s_v2 Pos() const {
            return {x, y};
        }

        constexpr s_v2 Size() const {
            return {width, height};
        }

        constexpr s_v2 Center() const {
            return {x + (width / 2.0f), y + (height / 2.0f)};
        }

        constexpr t_f32 Left() const {
            return x;
        }

        constexpr t_f32 Top() const {
            return y;
        }

        constexpr t_f32 Right() const {
            return x + width;
        }

        constexpr t_f32 Bottom() const {
            return y + height;
        }

        constexpr t_f32 Area() const {
            return width * height;
        }

        explicit constexpr operator s_rect_i() const;
        constexpr s_rect_i ToRectI() const;

        constexpr t_b8 ContainsPoint(const s_v2 pt) const {
            return pt.x > Left() && pt.y > Top() && pt.x < Right() && pt.y < Bottom();
        }

        constexpr t_b8 IntersWith(const s_rect_f other) const {
            return Left() < other.Right() && Top() < other.Bottom() && Right() > other.Left() && Bottom() > other.Top();
        }

        constexpr s_rect_f Clamped(const s_rect_f container) const {
            const s_v2 tl = {ZF_MAX(x, container.x), ZF_MAX(y, container.y)};
            return {tl.x, tl.y, ZF_MIN(Right(), container.Right()) - tl.x, ZF_MIN(Bottom(), container.Bottom()) - tl.y};
        }

        constexpr t_f32 CalcOccupancyPerc(const s_rect_f container) const {
            const auto subrect = Clamped(container);
            return Clamp(subrect.Area() / container.Area(), 0.0f, 1.0f);
        }
    };

    struct s_rect_i {
        t_i32 x = 0;
        t_i32 y = 0;
        t_i32 width = 0;
        t_i32 height = 0;

        constexpr s_rect_i() = default;
        constexpr s_rect_i(const t_i32 x, const t_i32 y, const t_i32 width, const t_i32 height) : x(x), y(y), width(width), height(height) {}
        constexpr s_rect_i(const s_v2_i pos, const s_v2_i size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

        constexpr s_v2_i Pos() const {
            return {x, y};
        }

        constexpr s_v2_i Size() const {
            return {width, height};
        }

        constexpr t_i32 Left() const {
            return x;
        }

        constexpr t_i32 Top() const {
            return y;
        }

        constexpr t_i32 Right() const {
            return x + width;
        }

        constexpr t_i32 Bottom() const {
            return y + height;
        }

        constexpr t_i32 Area() const {
            return width * height;
        }

        constexpr t_b8 operator==(const s_rect_i other) const {
            return x == other.x && y == other.y && width == other.width && height == other.height;
        }

        constexpr t_b8 operator!=(const s_rect_i other) const {
            return !(*this == other);
        }

        explicit constexpr operator s_rect_f() const {
            return {static_cast<t_f32>(x), static_cast<t_f32>(y), static_cast<t_f32>(width), static_cast<t_f32>(height)};
        }

        constexpr s_rect_f ToRectF() const {
            return static_cast<s_rect_f>(*this);
        }

        constexpr t_b8 ContainsPoint(const s_v2_i pt) const {
            return pt.x > Left() && pt.y > Top() && pt.x < Right() && pt.y < Bottom();
        }

        constexpr t_b8 IntersWith(const s_rect_i other) const {
            return Left() < other.Right() && Top() < other.Bottom() && Right() > other.Left() && Bottom() > other.Top();
        }

        constexpr s_rect_i Clamped(const s_rect_i container) const {
            const s_v2_i tl = {ZF_MAX(x, container.x), ZF_MAX(y, container.y)};
            return {tl.x, tl.y, ZF_MIN(Right(), container.Right()) - tl.x, ZF_MIN(Bottom(), container.Bottom()) - tl.y};
        }

        constexpr t_f32 CalcOccupancyPerc(const s_rect_i container) const {
            const auto subrect = Clamped(container);
            return Clamp(static_cast<t_f32>(subrect.Area()) / static_cast<t_f32>(container.Area()), 0.0f, 1.0f);
        }
    };

    constexpr s_rect_f::operator s_rect_i() const {
        return {static_cast<t_i32>(x), static_cast<t_i32>(y), static_cast<t_i32>(width), static_cast<t_i32>(height)};
    }

    constexpr s_rect_i s_rect_f::ToRectI() const {
        return static_cast<s_rect_i>(*this);
    }

    s_rect_f CalcSpanningRect(const s_array<s_rect_f> rects);
    s_rect_i CalcSpanningRect(const s_array<s_rect_i> rects);

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

    constexpr t_f32 Lerp(const t_f32 a, const t_f32 b, const t_f32 t) {
        return a + ((b - a) * t);
    }

    constexpr s_v2 Lerp(const s_v2 a, const s_v2 b, const t_f32 t) {
        return a + ((b - a) * t);
    }

    constexpr s_v2 CompwiseProd(const s_v2 a, const s_v2 b) {
        return {a.x * b.x, a.y * b.y};
    }

    constexpr t_f32 DotProd(const s_v2 a, const s_v2 b) {
        return (a.x * b.x) + (a.y * b.y);
    }

    inline t_f32 CalcDist(const s_v2 a, const s_v2 b) {
        return s_v2(b.x - a.x, b.y - a.y).CalcMag();
    }

    inline s_v2 CalcDir(const s_v2 a, const s_v2 b) {
        return s_v2(b.x - a.x, b.y - a.y).CalcNormalOrZero();
    }

    inline t_f32 CalcDirInRads(const s_v2 a, const s_v2 b) {
        return atan2(-(b.y - a.y), b.x - a.x);
    }

    inline s_v2 CalcLenDir(const t_f32 len, const t_f32 dir) {
        return s_v2(cos(dir), -sin(dir)) * len;
    }

    constexpr s_v2 ClampPointInRect(const s_v2 pt, const s_rect_f rect) {
        return {Clamp(pt.x, rect.Left(), rect.Right()), Clamp(pt.y, rect.Top(), rect.Bottom())};
    }

    constexpr s_v2_i ClampPointInRect(const s_v2_i pt, const s_rect_i rect) {
        return {Clamp(pt.x, rect.Left(), rect.Right()), Clamp(pt.y, rect.Top(), rect.Bottom())};
    }
}
