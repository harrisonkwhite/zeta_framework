#pragma once

#include <cmath>
#include <zcl/zcl_mem.h>

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
    constexpr t_i32 CalcDigitCnt(const tp_type n) {
        if (n < 0) {
            return CalcDigitCnt(-n);
        }

        if (n < 10) {
            return 1;
        }

        return 1 + CalcDigitCnt(n / 10);
    }

    // Gives the digit at the given index, where the indexes are from the least significant digit to the most.
    template <c_integral tp_type>
    constexpr tp_type FindDigitAt(const tp_type n, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < CalcDigitCnt(n));

        if (n < 0) {
            return FindDigitAt(-n, index);
        }

        if (index == 0) {
            return n % 10;
        }

        if (n < 10) {
            return 0;
        }

        return FindDigitAt(n / 10, index - 1);
    }

    constexpr t_b8 IsNearlyEqual(const t_f32 val, const t_f32 targ, const t_f32 tol = 1e-5f) {
        ZF_ASSERT(tol >= 0);
        return val >= targ - tol && val <= targ + tol;
    }

    // ============================================================
    // @section: Vectors
    // ============================================================
    struct s_v2_i;

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

        explicit constexpr operator s_v2_i() const;
        constexpr s_v2_i ToV2I() const;

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

    // ============================================================
    // @section: Rectangles
    // ============================================================
    struct s_rect_i;

    struct s_rect_f {
    public:
        t_f32 x = 0.0f;
        t_f32 y = 0.0f;

        constexpr s_rect_f() = default;

        constexpr s_rect_f(const t_f32 x, const t_f32 y, const t_f32 width, const t_f32 height) : x(x), y(y), m_width(width), m_height(height) {
            ZF_ASSERT(width >= 0.0f && height >= 0.0f);
        }

        constexpr s_rect_f(const s_v2 pos, const s_v2 size) : x(pos.x), y(pos.y), m_width(size.x), m_height(size.y) {
            ZF_ASSERT(size.x >= 0.0f && size.y >= 0.0f);
        }

        constexpr t_f32 Width() const { return m_width; }
        constexpr t_f32 Height() const { return m_height; }

        constexpr s_v2 Pos() const { return {x, y}; }
        constexpr s_v2 Size() const { return {m_width, m_height}; }

        constexpr t_f32 Left() const { return x; }
        constexpr t_f32 Top() const { return y; }
        constexpr t_f32 Right() const { return x + m_width; }
        constexpr t_f32 Bottom() const { return y + m_height; }

        constexpr s_v2 TopLeft() const { return {Left(), Top()}; }
        constexpr s_v2 TopCenter() const { return {x + (m_width / 2.0f), Top()}; }
        constexpr s_v2 TopRight() const { return {Right(), Top()}; }
        constexpr s_v2 CenterLeft() const { return {Left(), y + (m_height / 2.0f)}; }
        constexpr s_v2 Center() const { return {x + (m_width / 2.0f), y + (m_height / 2.0f)}; }
        constexpr s_v2 CenterRight() const { return {Right(), y + (m_height / 2.0f)}; }
        constexpr s_v2 BottomLeft() const { return {Left(), Bottom()}; }
        constexpr s_v2 BottomCenter() const { return {x + (m_width / 2.0f), Bottom()}; }
        constexpr s_v2 BottomRight() const { return {Right(), Bottom()}; }

        constexpr t_f32 Area() const { return m_width * m_height; }

        explicit constexpr operator s_rect_i() const;
        constexpr s_rect_i ToRectI() const;

        constexpr t_b8 ContainsPoint(const s_v2 pt) const {
            return pt.x > Left() && pt.y > Top() && pt.x < Right() && pt.y < Bottom();
        }

        constexpr t_b8 IntersWith(const s_rect_f other) const {
            return Left() < other.Right() && Top() < other.Bottom() && Right() > other.Left() && Bottom() > other.Top();
        }

    private:
        t_f32 m_width = 0.0f;
        t_f32 m_height = 0.0f;
    };

    struct s_rect_i {
    public:
        t_i32 x = 0;
        t_i32 y = 0;

        constexpr s_rect_i() = default;

        constexpr s_rect_i(const t_i32 x, const t_i32 y, const t_i32 width, const t_i32 height) : x(x), y(y), m_width(width), m_height(height) {
            ZF_ASSERT(width >= 0 && height >= 0);
        }

        constexpr s_rect_i(const s_v2_i pos, const s_v2_i size) : x(pos.x), y(pos.y), m_width(size.x), m_height(size.y) {
            ZF_ASSERT(size.x >= 0 && size.y >= 0);
        }

        constexpr t_i32 Width() const { return m_width; }
        constexpr t_i32 Height() const { return m_height; }

        constexpr s_v2_i Pos() const { return {x, y}; }
        constexpr s_v2_i Size() const { return {m_width, m_height}; }

        constexpr t_i32 Left() const { return x; }
        constexpr t_i32 Top() const { return y; }
        constexpr t_i32 Right() const { return x + m_width; }
        constexpr t_i32 Bottom() const { return y + m_height; }

        constexpr s_v2_i TopLeft() const { return {Left(), Top()}; }
        constexpr s_v2_i TopRight() const { return {Right(), Top()}; }
        constexpr s_v2_i BottomLeft() const { return {Left(), Bottom()}; }
        constexpr s_v2_i BottomRight() const { return {Right(), Bottom()}; }

        constexpr t_i32 Area() const { return m_width * m_height; }

        constexpr t_b8 operator==(const s_rect_i other) const {
            return x == other.x && y == other.y && m_width == other.m_width && m_height == other.m_height;
        }

        constexpr t_b8 operator!=(const s_rect_i other) const {
            return !(*this == other);
        }

        explicit constexpr operator s_rect_f() const {
            return {static_cast<t_f32>(x), static_cast<t_f32>(y), static_cast<t_f32>(m_width), static_cast<t_f32>(m_height)};
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

    private:
        t_i32 m_width = 0;
        t_i32 m_height = 0;
    };

    constexpr s_rect_f::operator s_rect_i() const {
        return {static_cast<t_i32>(x), static_cast<t_i32>(y), static_cast<t_i32>(m_width), static_cast<t_i32>(m_height)};
    }

    constexpr s_rect_i s_rect_f::ToRectI() const {
        return static_cast<s_rect_i>(*this);
    }

    s_rect_f CalcSpanningRect(const s_array<s_rect_f> rects);
    s_rect_i CalcSpanningRect(const s_array<s_rect_i> rects);

    // ============================================================

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

    constexpr s_v2 ClampedWithinContainer(const s_v2 pt, const s_rect_f rect) {
        return {Clamp(pt.x, rect.Left(), rect.Right()), Clamp(pt.y, rect.Top(), rect.Bottom())};
    }

    constexpr s_v2_i ClampedWithinContainer(const s_v2_i pt, const s_rect_i rect) {
        return {Clamp(pt.x, rect.Left(), rect.Right()), Clamp(pt.y, rect.Top(), rect.Bottom())};
    }

    constexpr s_rect_f ClampedWithinContainer(const s_rect_f rect, const s_rect_f container) {
        const s_v2 tl = {ZF_MAX(rect.x, container.x), ZF_MAX(rect.y, container.y)};
        return {tl.x, tl.y, ZF_MIN(rect.Right(), container.Right()) - tl.x, ZF_MIN(rect.Bottom(), container.Bottom()) - tl.y};
    }

    constexpr s_rect_i ClampedWithinContainer(const s_rect_i rect, const s_rect_i container) {
        const s_v2_i tl = {ZF_MAX(rect.x, container.x), ZF_MAX(rect.y, container.y)};
        return {tl.x, tl.y, ZF_MIN(rect.Right(), container.Right()) - tl.x, ZF_MIN(rect.Bottom(), container.Bottom()) - tl.y};
    }

    // Returns a value between 0 and 1 indicating what percentage of the rectangle is within the container.
    constexpr t_f32 CalcPercOfOccupance(const s_rect_f rect, const s_rect_f container) {
        const auto subrect = ClampedWithinContainer(rect, container);
        return Clamp(subrect.Area() / container.Area(), 0.0f, 1.0f);
    }

    // Returns a value between 0 and 1 indicating what percentage of the rectangle is within the container.
    constexpr t_f32 CalcPercOfOccupance(const s_rect_i rect, const s_rect_i container) {
        const auto subrect = ClampedWithinContainer(rect, container);
        return Clamp(static_cast<t_f32>(subrect.Area()) / static_cast<t_f32>(container.Area()), 0.0f, 1.0f);
    }
}
