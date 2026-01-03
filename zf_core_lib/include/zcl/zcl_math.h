#pragma once

#include <cmath>
#include <zcl/zcl_mem.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    constexpr t_f32 g_pi = 3.14159265358979323846f;
    constexpr t_f32 g_tau = 6.28318530717958647692f;

    struct s_v2 {
        t_f32 x;
        t_f32 y;

        s_v2 operator+(const s_v2 &other) const { return {x + other.x, y + other.y}; }
        s_v2 operator-(const s_v2 &other) const { return {x - other.x, y - other.y}; }
        s_v2 operator*(const t_f32 scalar) const { return {x * scalar, y * scalar}; }
        s_v2 operator/(const t_f32 scalar) const { return {x / scalar, y / scalar}; }

        s_v2 &operator+=(const s_v2 &other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        s_v2 &operator-=(const s_v2 &other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        s_v2 &operator*=(const t_f32 scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        s_v2 &operator/=(const t_f32 scalar) {
            x /= scalar;
            y /= scalar;
            return *this;
        }
    };

    inline s_v2 operator*(const t_f32 scalar, const s_v2 v) {
        return {v.x * scalar, v.y * scalar};
    }

    struct s_v2_i {
        t_i32 x;
        t_i32 y;

        t_b8 operator==(const s_v2_i &other) const { return x == other.x && y == other.y; }
        t_b8 operator!=(const s_v2_i &other) const { return !(*this == other); }
        s_v2_i operator+(const s_v2_i &other) const { return {x + other.x, y + other.y}; }
        s_v2_i operator-(const s_v2_i &other) const { return {x - other.x, y - other.y}; }

        s_v2_i &operator+=(const s_v2_i &other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        s_v2_i &operator-=(const s_v2_i &other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }
    };

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

    struct s_rect_f {
        t_f32 x;
        t_f32 y;
        t_f32 width;
        t_f32 height;
    };

    struct s_rect_i {
        t_i32 x;
        t_i32 y;
        t_i32 width;
        t_i32 height;
    };

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
    t_i32 CalcDigitCount(const tp_type n) {
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
    tp_type DetermineDigitAt(const tp_type n, const t_i32 index) {
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

    inline t_b8 IsNearlyEqual(const t_f32 val, const t_f32 targ, const t_f32 tol = 1e-5f) {
        ZF_ASSERT(tol >= 0);
        return val >= targ - tol && val <= targ + tol;
    }

    inline s_v2 CreateV2(const t_f32 x, const t_f32 y) {
        return {x, y};
    }

    inline s_v2_i CreateV2I(const t_i32 x, const t_i32 y) {
        return {x, y};
    }

    inline s_v2 ToV2(const s_v2_i v) {
        return {static_cast<t_f32>(v.x), static_cast<t_f32>(v.y)};
    }

    inline s_rect_f CreateRectF(const t_f32 x, const t_f32 y, const t_f32 width, const t_f32 height) {
        return {x, y, width, height};
    }

    inline s_rect_f CreateRectF(const s_v2 pos, const s_v2 size) {
        return {pos.x, pos.y, size.x, size.y};
    }

    inline s_rect_f CreateRectF(const s_v2 pos, const s_v2 size, const s_v2 origin) {
        return {pos.x - (size.x * origin.x), pos.y - (size.y * origin.y), size.x, size.y};
    }

    inline s_rect_i CreateRectI(const t_i32 x, const t_i32 y, const t_i32 width, const t_i32 height) {
        return {x, y, width, height};
    }

    inline s_rect_i CreateRectI(const s_v2_i pos, const s_v2_i size) {
        return {pos.x, pos.y, size.x, size.y};
    }

    inline s_v2 Pos(const s_rect_f rect) { return {rect.x, rect.y}; }
    inline s_v2_i Pos(const s_rect_i rect) { return {rect.x, rect.y}; }

    inline s_v2 Size(const s_rect_f rect) { return {rect.width, rect.height}; }
    inline s_v2_i Size(const s_rect_i rect) { return {rect.width, rect.height}; }

    inline s_v2 Center(const s_rect_f rect) { return {rect.x + (rect.width / 2.0f), rect.y + (rect.height / 2.0f)}; }

    inline t_f32 Left(const s_rect_f rect) { return rect.x; }
    inline t_i32 Left(const s_rect_i rect) { return rect.x; }

    inline t_f32 Top(const s_rect_f rect) { return rect.y; }
    inline t_i32 Top(const s_rect_i rect) { return rect.y; }

    inline t_f32 Right(const s_rect_f rect) { return rect.x + rect.width; }
    inline t_i32 Right(const s_rect_i rect) { return rect.x + rect.width; }

    inline t_f32 Bottom(const s_rect_f rect) { return rect.y + rect.height; }
    inline t_i32 Bottom(const s_rect_i rect) { return rect.y + rect.height; }

    inline s_v2 TopLeft(const s_rect_f rect) { return {Left(rect), Top(rect)}; }
    inline s_v2_i TopLeft(const s_rect_i rect) { return {Left(rect), Top(rect)}; }

    inline s_v2 TopRight(const s_rect_f rect) { return {Right(rect), Top(rect)}; }
    inline s_v2_i TopRight(const s_rect_i rect) { return {Right(rect), Top(rect)}; }

    inline s_v2 BottomLeft(const s_rect_f rect) { return {Left(rect), Bottom(rect)}; }
    inline s_v2_i BottomLeft(const s_rect_i rect) { return {Left(rect), Bottom(rect)}; }

    inline s_v2 BottomRight(const s_rect_f rect) { return {Right(rect), Bottom(rect)}; }
    inline s_v2_i BottomRight(const s_rect_i rect) { return {Right(rect), Bottom(rect)}; }

    inline t_f32 Area(const s_rect_f rect) { return rect.width * rect.height; }
    inline t_i32 Area(const s_rect_i rect) { return rect.width * rect.height; }

    inline t_b8 AreEqual(const s_rect_i a, const s_rect_i b) {
        return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
    }

    inline s_rect_f ToRectF(const s_rect_i rect) {
        return {static_cast<t_f32>(rect.x), static_cast<t_f32>(rect.y), static_cast<t_f32>(rect.width), static_cast<t_f32>(rect.height)};
    }

    inline s_rect_i ToRectI(const s_rect_f rect) {
        return {static_cast<t_i32>(rect.x), static_cast<t_i32>(rect.y), static_cast<t_i32>(rect.width), static_cast<t_i32>(rect.height)};
    }

    inline t_b8 DoRectsInters(const s_rect_f a, const s_rect_f b) {
        return Left(a) < Right(b) && Top(a) < Bottom(b) && Right(a) > Left(b) && Bottom(a) > Top(b);
    }

    inline t_b8 DoRectsInters(const s_rect_i a, const s_rect_i b) {
        return Left(a) < Right(b) && Top(a) < Bottom(b) && Right(a) > Left(b) && Bottom(a) > Top(b);
    }

    // @todo: Make a constant.
    inline s_mat4x4 IdentityMatrix() {
        s_mat4x4 mat = {};
        mat.elems[0][0] = 1.0f;
        mat.elems[1][1] = 1.0f;
        mat.elems[2][2] = 1.0f;
        mat.elems[3][3] = 1.0f;

        return mat;
    }

    inline t_f32 Lerp(const t_f32 a, const t_f32 b, const t_f32 t) { return a + ((b - a) * t); }
    inline s_v2 Lerp(const s_v2 a, const s_v2 b, const t_f32 t) { return a + ((b - a) * t); }

    inline s_v2 CompwiseProd(const s_v2 a, const s_v2 b) { return {a.x * b.x, a.y * b.y}; }
    inline s_v2_i CompwiseProd(const s_v2_i a, const s_v2_i b) { return {a.x * b.x, a.y * b.y}; }

    inline t_f32 DotProd(const s_v2 a, const s_v2 b) { return (a.x * b.x) + (a.y * b.y); }
    inline t_i32 DotProd(const s_v2_i a, const s_v2_i b) { return (a.x * b.x) + (a.y * b.y); }

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
        return s_v2(cos(dir), -sin(dir)) * len;
    }

    inline t_b8 IsPointInRect(const s_v2 pt, const s_rect_f rect) {
        return pt.x > Left(rect) && pt.y > Top(rect) && pt.x < Right(rect) && pt.y < Bottom(rect);
    }

    inline t_b8 IsPointInRect(const s_v2_i pt, const s_rect_i rect) {
        return pt.x > Left(rect) && pt.y > Top(rect) && pt.x < Right(rect) && pt.y < Bottom(rect);
    }

    inline s_v2 ClampedWithinContainer(const s_v2 pt, const s_rect_f container) {
        return {Clamp(pt.x, Left(container), Right(container)), Clamp(pt.y, Top(container), Bottom(container))};
    }

    inline s_v2_i ClampedWithinContainer(const s_v2_i pt, const s_rect_i container) {
        return {Clamp(pt.x, Left(container), Right(container)), Clamp(pt.y, Top(container), Bottom(container))};
    }

    inline s_rect_f ClampedWithinContainer(const s_rect_f rect, const s_rect_f container) {
        const s_v2 tl = {ZF_MAX(rect.x, container.x), ZF_MAX(rect.y, container.y)};
        return {tl.x, tl.y, ZF_MAX(ZF_MIN(Right(rect), Right(container)) - tl.x, 0), ZF_MAX(ZF_MIN(Bottom(rect), Bottom(container)) - tl.y, 0)};
    }

    inline s_rect_i ClampedWithinContainer(const s_rect_i rect, const s_rect_i container) {
        const s_v2_i tl = {ZF_MAX(rect.x, container.x), ZF_MAX(rect.y, container.y)};
        return {tl.x, tl.y, ZF_MAX(ZF_MIN(Right(rect), Right(container)) - tl.x, 0), ZF_MAX(ZF_MIN(Bottom(rect), Bottom(container)) - tl.y, 0)};
    }

    // Returns a value between 0 and 1 indicating what percentage of the rectangle is within the container.
    inline t_f32 CalcPercOfOccupance(const s_rect_f rect, const s_rect_f container) {
        ZF_ASSERT(container.width > 0 && container.height > 0);

        const auto subrect = ClampedWithinContainer(rect, container);
        return Clamp(Area(subrect) / Area(container), 0.0f, 1.0f);
    }

    // Returns a value between 0 and 1 indicating what percentage of the rectangle is within the container.
    inline t_f32 CalcPercOfOccupance(const s_rect_i rect, const s_rect_i container) {
        ZF_ASSERT(container.width > 0 && container.height > 0);

        const auto subrect = ClampedWithinContainer(rect, container);
        return Clamp(static_cast<t_f32>(Area(subrect)) / static_cast<t_f32>(Area(container)), 0.0f, 1.0f);
    }

    s_rect_f CalcSpanningRect(const s_array_mut<s_rect_f> rects);
    s_rect_i CalcSpanningRect(const s_array_mut<s_rect_i> rects);

    // ============================================================
}
