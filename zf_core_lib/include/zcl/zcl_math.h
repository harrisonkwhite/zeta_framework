#pragma once

#include <cmath>
#include <zcl/zcl_mem.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    constexpr t_f32 g_pi = 3.14159265358979323846f;
    constexpr t_f32 g_tau = 6.28318530717958647692f;

    struct s_v2_i;

    struct s_v2 {
        t_f32 x;
        t_f32 y;

        constexpr s_v2() = default;
        constexpr s_v2(const t_f32 x, const t_f32 y) : x(x), y(y) {}

        constexpr s_v2 operator+(const s_v2 other) const { return {x + other.x, y + other.y}; }
        constexpr s_v2 operator-(const s_v2 other) const { return {x - other.x, y - other.y}; }
        constexpr s_v2 operator*(const t_f32 scalar) const { return {x * scalar, y * scalar}; }
        constexpr s_v2 operator/(const t_f32 scalar) const { return {x / scalar, y / scalar}; }

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

        explicit constexpr operator s_v2_i() const;
        constexpr s_v2_i ToV2I() const;
    };

    constexpr s_v2 operator*(const t_f32 scalar, const s_v2 v) {
        return {v.x * scalar, v.y * scalar};
    }

    struct s_v2_i {
        t_i32 x;
        t_i32 y;

        constexpr s_v2_i() = default;
        constexpr s_v2_i(const t_i32 x, const t_i32 y) : x(x), y(y) {}

        constexpr t_b8 operator==(const s_v2_i &other) const { return x == other.x && y == other.y; }
        constexpr t_b8 operator!=(const s_v2_i &other) const { return !(*this == other); }
        constexpr s_v2_i operator+(const s_v2_i other) const { return {x + other.x, y + other.y}; }
        constexpr s_v2_i operator-(const s_v2_i other) const { return {x - other.x, y - other.y}; }

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

        explicit constexpr operator s_v2() const {
            return {static_cast<t_f32>(x), static_cast<t_f32>(y)};
        }

        constexpr s_v2 ToV2() const {
            return static_cast<s_v2>(*this);
        }
    };

    constexpr s_v2::operator s_v2_i() const {
        return {static_cast<t_i32>(x), static_cast<t_i32>(y)};
    }

    constexpr s_v2_i s_v2::ToV2I() const {
        return static_cast<s_v2_i>(*this);
    }

    struct s_v3 {
        t_f32 x;
        t_f32 y;
        t_f32 z;

        constexpr s_v3() = default;
        constexpr s_v3(const t_f32 x, const t_f32 y, const t_f32 z) : x(x), y(y), z(z) {}
    };

    struct s_v4 {
        t_f32 x;
        t_f32 y;
        t_f32 z;
        t_f32 w;

        constexpr s_v4() = default;
        constexpr s_v4(const t_f32 x, const t_f32 y, const t_f32 z, const t_f32 w) : x(x), y(y), z(z), w(w) {}
    };

    struct s_rect_i;

    struct s_rect_f {
        t_f32 x;
        t_f32 y;
        t_f32 width;
        t_f32 height;

        constexpr s_rect_f() = default;
        constexpr s_rect_f(const t_f32 x, const t_f32 y, const t_f32 width, const t_f32 height) : x(x), y(y), width(width), height(height) {}
        constexpr s_rect_f(const s_v2 pos, const s_v2 size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

        constexpr s_v2 Pos() const { return {x, y}; }
        constexpr s_v2 Size() const { return {width, height}; }

        constexpr t_f32 Left() const { return x; }
        constexpr t_f32 Top() const { return y; }
        constexpr t_f32 Right() const { return x + width; }
        constexpr t_f32 Bottom() const { return y + height; }

        constexpr s_v2 TopLeft() const { return {Left(), Top()}; }
        constexpr s_v2 TopCenter() const { return {x + (width / 2.0f), Top()}; }
        constexpr s_v2 TopRight() const { return {Right(), Top()}; }
        constexpr s_v2 CenterLeft() const { return {Left(), y + (height / 2.0f)}; }
        constexpr s_v2 Center() const { return {x + (width / 2.0f), y + (height / 2.0f)}; }
        constexpr s_v2 CenterRight() const { return {Right(), y + (height / 2.0f)}; }
        constexpr s_v2 BottomLeft() const { return {Left(), Bottom()}; }
        constexpr s_v2 BottomCenter() const { return {x + (width / 2.0f), Bottom()}; }
        constexpr s_v2 BottomRight() const { return {Right(), Bottom()}; }

        constexpr t_f32 Area() const { return width * height; }

        explicit constexpr operator s_rect_i() const;
        constexpr s_rect_i ToRectI() const;
    };

    struct s_rect_i {
        t_i32 x;
        t_i32 y;
        t_i32 width;
        t_i32 height;

        constexpr s_rect_i() = default;
        constexpr s_rect_i(const t_i32 x, const t_i32 y, const t_i32 width, const t_i32 height) : x(x), y(y), width(width), height(height) {}
        constexpr s_rect_i(const s_v2_i pos, const s_v2_i size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

        constexpr s_v2_i Pos() const { return {x, y}; }
        constexpr s_v2_i Size() const { return {width, height}; }

        constexpr t_i32 Left() const { return x; }
        constexpr t_i32 Top() const { return y; }
        constexpr t_i32 Right() const { return x + width; }
        constexpr t_i32 Bottom() const { return y + height; }

        constexpr s_v2_i TopLeft() const { return {Left(), Top()}; }
        constexpr s_v2_i TopRight() const { return {Right(), Top()}; }
        constexpr s_v2_i BottomLeft() const { return {Left(), Bottom()}; }
        constexpr s_v2_i BottomRight() const { return {Right(), Bottom()}; }

        constexpr t_i32 Area() const { return width * height; }

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
    };

    constexpr s_rect_f::operator s_rect_i() const {
        return {static_cast<t_i32>(x), static_cast<t_i32>(y), static_cast<t_i32>(width), static_cast<t_i32>(height)};
    }

    constexpr s_rect_i s_rect_f::ToRectI() const {
        return static_cast<s_rect_i>(*this);
    }

    struct s_mat4x4 {
        s_static_array<s_static_array<t_f32, 4>, 4> elems;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    constexpr t_f32 DegsToRads(const t_f32 degs) {
        return degs * (g_pi / 180.0f);
    }

    constexpr t_f32 RadsToDegs(const t_f32 rads) {
        return rads * (180.0f / g_pi);
    }

    template <co_integral tp_type>
    constexpr t_i32 CalcDigitCount(const tp_type n) {
        if (n < 0) {
            return CalcDigitCount(-n);
        }

        if (n < 10) {
            return 1;
        }

        return 1 + CalcDigitCount(n / 10);
    }

    // Determines the digit at the given index, where the indexes are from the least significant digit to the most.
    template <co_integral tp_type>
    constexpr tp_type DetermineDigitAt(const tp_type n, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < CalcDigitCount(n));

        if (n < 0) {
            return DetermineDigitAt(-n, index);
        }

        if (index == 0) {
            return n % 10;
        }

        if (n < 10) {
            return 0;
        }

        return DetermineDigitAt(n / 10, index - 1);
    }

    constexpr t_b8 IsNearlyEqual(const t_f32 val, const t_f32 targ, const t_f32 tol = 1e-5f) {
        ZF_ASSERT(tol >= 0);
        return val >= targ - tol && val <= targ + tol;
    }

    constexpr t_b8 DoRectsInters(const s_rect_i a, const s_rect_i b) {
        return a.Left() < b.Right() && a.Top() < b.Bottom() && a.Right() > b.Left() && a.Bottom() > b.Top();
    }

    constexpr t_b8 DoRectsInters(const s_rect_f a, const s_rect_f b) {
        return a.Left() < b.Right() && a.Top() < b.Bottom() && a.Right() > b.Left() && a.Bottom() > b.Top();
    }

    constexpr s_mat4x4 IdentityMatrix() {
        s_mat4x4 mat = {};
        mat.elems[0][0] = 1.0f;
        mat.elems[1][1] = 1.0f;
        mat.elems[2][2] = 1.0f;
        mat.elems[3][3] = 1.0f;

        return mat;
    }

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

    inline t_f32 CalcMag(const s_v2 v) {
        return sqrt((v.x * v.x) + (v.y * v.y));
    }

    // Returns {} if a divide by 0 is attempted.
    inline s_v2 CalcNormal(const s_v2 v) {
        const t_f32 mag = CalcMag(v);

        if (mag == 0.0f) {
            return {};
        }

        return {v.x / mag, v.y / mag};
    }

    inline t_f32 CalcDist(const s_v2 a, const s_v2 b) {
        return CalcMag(b - a);
    }

    inline s_v2 CalcDir(const s_v2 a, const s_v2 b) {
        return CalcNormal(b - a);
    }

    // Returns 0 if the horizontal and vertical differences of the vectors are 0.
    inline t_f32 CalcDirInRads(const s_v2 a, const s_v2 b) {
        const t_f32 rise = -(b.y - a.y);
        const t_f32 run = b.x - a.x;

        if (rise == 0.0f && run == 0.0f) {
            return 0.0f;
        }

        return atan2(rise, run);
    }

    inline s_v2 CalcLenDir(const t_f32 len, const t_f32 dir) {
        return s_v2{cos(dir), -sin(dir)} * len;
    }

    constexpr t_b8 IsPointInRect(const s_v2 pt, const s_rect_f rect) {
        return pt.x > rect.Left() && pt.y > rect.Top() && pt.x < rect.Right() && pt.y < rect.Bottom();
    }

    constexpr t_b8 IsPointInRect(const s_v2_i pt, const s_rect_i rect) {
        return pt.x > rect.Left() && pt.y > rect.Top() && pt.x < rect.Right() && pt.y < rect.Bottom();
    }

    constexpr s_v2 ClampedWithinContainer(const s_v2 pt, const s_rect_f container) {
        return {Clamp(pt.x, container.Left(), container.Right()), Clamp(pt.y, container.Top(), container.Bottom())};
    }

    constexpr s_v2_i ClampedWithinContainer(const s_v2_i pt, const s_rect_i container) {
        return {Clamp(pt.x, container.Left(), container.Right()), Clamp(pt.y, container.Top(), container.Bottom())};
    }

    constexpr s_rect_f ClampedWithinContainer(const s_rect_f rect, const s_rect_f container) {
        const s_v2 tl = {ZF_MAX(rect.x, container.x), ZF_MAX(rect.y, container.y)};
        return {tl.x, tl.y, ZF_MAX(ZF_MIN(rect.Right(), container.Right()) - tl.x, 0), ZF_MAX(ZF_MIN(rect.Bottom(), container.Bottom()) - tl.y, 0)};
    }

    constexpr s_rect_i ClampedWithinContainer(const s_rect_i rect, const s_rect_i container) {
        const s_v2_i tl = {ZF_MAX(rect.x, container.x), ZF_MAX(rect.y, container.y)};
        return {tl.x, tl.y, ZF_MAX(ZF_MIN(rect.Right(), container.Right()) - tl.x, 0), ZF_MAX(ZF_MIN(rect.Bottom(), container.Bottom()) - tl.y, 0)};
    }

    // Returns a value between 0 and 1 indicating what percentage of the rectangle is within the container.
    constexpr t_f32 CalcPercOfOccupance(const s_rect_f rect, const s_rect_f container) {
        ZF_ASSERT(container.width > 0 && container.height > 0);

        const auto subrect = ClampedWithinContainer(rect, container);
        return Clamp(subrect.Area() / container.Area(), 0.0f, 1.0f);
    }

    // Returns a value between 0 and 1 indicating what percentage of the rectangle is within the container.
    constexpr t_f32 CalcPercOfOccupance(const s_rect_i rect, const s_rect_i container) {
        ZF_ASSERT(container.width > 0 && container.height > 0);

        const auto subrect = ClampedWithinContainer(rect, container);
        return Clamp(static_cast<t_f32>(subrect.Area()) / static_cast<t_f32>(container.Area()), 0.0f, 1.0f);
    }

    s_rect_f CalcSpanningRect(const s_array_mut<s_rect_f> rects);
    s_rect_i CalcSpanningRect(const s_array_mut<s_rect_i> rects);

    // ============================================================
}
