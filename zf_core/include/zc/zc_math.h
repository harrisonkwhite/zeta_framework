#pragma once

#include <cmath>
#include <zc/zc_mem.h>

namespace zf {
    template<c_floating_point tp_type> constexpr tp_type Pi();
    template<> constexpr t_f32 Pi<t_f32>() { return 3.14159265358979323846f; }
    template<> constexpr t_f64 Pi<t_f64>() { return 3.141592653589793238462643383279502884; }

    template<c_floating_point tp_type> constexpr tp_type Tau();
    template<> constexpr t_f32 Tau<t_f32>() { return 6.28318530717958647692f; }
    template<> constexpr t_f64 Tau<t_f64>() { return 6.283185307179586476925286766559005768; }

    template<c_floating_point tp_type>
    constexpr tp_type DegsToRads(const tp_type degs) {
        return degs * (Pi<tp_type>() / 180);
    }

    template<c_floating_point tp_type>
    constexpr tp_type RadsToDegs(const tp_type rads) {
        return rads * (180 / Pi<tp_type>());
    }

    template<c_numeric tp_type>
    static constexpr tp_type Min(const tp_type& a, const tp_type& b) {
        return a <= b ? a : b;
    }

    template<c_numeric tp_type>
    static constexpr tp_type Max(const tp_type& a, const tp_type& b) {
        return a >= b ? a : b;
    }

    template<c_numeric tp_type>
    constexpr tp_type Abs(const tp_type n) {
        return n < 0 ? -n : n;
    }

    template<c_numeric tp_type>
    constexpr t_s32 Sign(const tp_type n) {
        if (n > 0) {
            return 1;
        } else if (n < 0) {
            return -1;
        }

        return 0;
    }

    template<c_numeric tp_type>
    constexpr tp_type Clamp(const tp_type n, const tp_type min, const tp_type max) {
        ZF_ASSERT(min <= max);
        return n < 0 ? -n : n;
    }

    template<c_integral tp_type>
    constexpr tp_type WrapUpper(const tp_type val, const tp_type max_excl) {
        return ((val % max_excl) + max_excl) % max_excl;
    }

    template<c_integral tp_type>
    constexpr tp_type Wrap(const tp_type val, const tp_type min, const tp_type max_excl) {
        return min + WrapUpper(val - min, max_excl - min);
    }

    template<c_integral tp_type>
    constexpr tp_type DigitAt(const tp_type n, const t_u32 index) {
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

    template<c_integral tp_type>
    constexpr t_s32 DigitCnt(const tp_type n) {
        if (n < 0) {
            return DigitCnt(-n);
        }

        if (n < 10) {
            return 1;
        }

        return 1 + DigitCnt(n / 10);
    }

    template<c_floating_point tp_type>
    constexpr t_b8 NearlyEqual(const tp_type val, const tp_type targ, const tp_type tol = 1e-5) {
        ZF_ASSERT(tol >= 0);
        return val >= targ - tol && val <= targ + tol;
    }

    template<c_numeric tp_type>
    struct s_v2 {
        tp_type x;
        tp_type y;

        constexpr s_v2() = default;
        constexpr s_v2(const tp_type x, const tp_type y) : x(x), y(y) {}

        constexpr t_b8 operator==(const s_v2<tp_type>& other) const {
            return x == other.x && y == other.y;
        }

        constexpr t_b8 operator!=(const s_v2<tp_type>& other) const {
            return !(*this == other);
        }

        constexpr s_v2<tp_type> operator+(const s_v2<tp_type>& other) const {
            return {x + other.x, y + other.y};
        }

        constexpr s_v2<tp_type> operator-(const s_v2<tp_type>& other) const {
            return {x - other.x, y - other.y};
        }

        s_v2<tp_type>& operator+=(const s_v2<tp_type>& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        s_v2<tp_type>& operator-=(const s_v2<tp_type>& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr s_v2<tp_type> operator*(const tp_type scalar) const requires c_floating_point<tp_type> {
            return {x * scalar, y * scalar};
        }

        constexpr s_v2<tp_type> operator/(const tp_type scalar) const requires c_floating_point<tp_type> {
            return {x / scalar, y / scalar};
        }

        s_v2<tp_type>& operator*=(const tp_type scalar) requires c_floating_point<tp_type> {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        s_v2<tp_type>& operator/=(const tp_type scalar) requires c_floating_point<tp_type> {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        template<c_numeric tp_other_type>
        constexpr explicit operator s_v2<tp_other_type>() const {
            return {static_cast<tp_other_type>(x), static_cast<tp_other_type>(y)};
        }
    };

    template<c_floating_point tp_type>
    constexpr s_v2<tp_type> operator*(const tp_type scalar, const s_v2<tp_type>& v) {
        return {v.x * scalar, v.y * scalar};
    }

    template<c_numeric tp_type>
    s_v2<tp_type> CompwiseProd(const s_v2<tp_type> a, const s_v2<tp_type> b) {
        return {a.x * b.x, a.y * b.y};
    }

    template<c_numeric tp_type>
    tp_type DotProd(const s_v2<tp_type> a, const s_v2<tp_type> b) {
        return (a.x * b.x) + (a.y * b.y);
    }

    template<c_floating_point tp_type>
    tp_type CalcMag(const s_v2<tp_type> v) {
        return sqrt((v.x * v.x) + (v.y * v.y));
    }

    template<c_floating_point tp_type>
    s_v2<tp_type> CalcNormalOrZero(const s_v2<tp_type> v) {
        const tp_type mag = CalcMag(v);

        if (mag == 0) {
            return {};
        }

        return {v.x / mag, v.y / mag};
    }

    template<c_floating_point tp_type>
    tp_type CalcDist(const s_v2<tp_type> a, const s_v2<tp_type> b) {
        return CalcMag(s_v2<tp_type>(b.x - a.x, b.y - a.y));
    }

    template<c_floating_point tp_type>
    s_v2<tp_type> CalcDir(const s_v2<tp_type> a, const s_v2<tp_type> b) {
        return CalcNormalOrZero({b.x - a.x, b.y - a.y});
    }

    template<c_floating_point tp_type>
    tp_type CalcDirInRads(const s_v2<tp_type> a, const s_v2<tp_type> b) {
        return atan2(-(b.y - a.y), b.x - a.x);
    }

    template<c_numeric tp_type>
    struct s_rect {
        tp_type x;
        tp_type y;
        tp_type width;
        tp_type height;

        constexpr s_rect() = default;
        constexpr s_rect(const tp_type x, const tp_type y, const tp_type width, const tp_type height)
            : x(x), y(y), width(width), height(height) {}
        constexpr s_rect(const s_v2<tp_type> pos, const s_v2<tp_type> size)
            : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

        constexpr t_b8 operator==(const s_rect<tp_type>& other) const {
            return x == other.x && y == other.y;
        }

        constexpr t_b8 operator!=(const s_rect<tp_type>& other) const {
            return !(*this == other);
        }

        template<c_numeric tp_other_type>
        constexpr explicit operator s_rect<tp_other_type>() const {
            return {
                static_cast<tp_other_type>(x),
                static_cast<tp_other_type>(y),
                static_cast<tp_other_type>(width),
                static_cast<tp_other_type>(height)
            };
        }
    };

    template<c_numeric tp_type> s_v2<tp_type> RectPos(const s_rect<tp_type> rect) { return {rect.x, rect.y}; }
    template<c_numeric tp_type> s_v2<tp_type> RectSize(const s_rect<tp_type> rect) { return {rect.width, rect.height}; }
    template<c_numeric tp_type> tp_type RectLeft(const s_rect<tp_type> rect) { return rect.x; }
    template<c_numeric tp_type> tp_type RectTop(const s_rect<tp_type> rect) { return rect.y; }
    template<c_numeric tp_type> tp_type RectRight(const s_rect<tp_type> rect) { return rect.x + rect.width; }
    template<c_numeric tp_type> tp_type RectBottom(const s_rect<tp_type> rect) { return rect.y + rect.height; }

    template<c_numeric tp_type>
    t_b8 DoesRectContainPoint(const s_rect<tp_type> rect, const s_v2<tp_type> pt) {
        return pt.x > RectLeft(rect) && pt.y > RectTop(rect) && pt.x < RectRight(rect) && pt.y < RectBottom(rect);
    }

    template<c_numeric tp_type>
    t_b8 DoRectsIntersect(const s_rect<tp_type> a, const s_rect<tp_type> b) {
        return RectLeft(a) < RectRight(b) && RectTop(a) < RectBottom(b) && RectRight(a) > RectLeft(b) && RectBottom(a) > RectTop(b);
    }

    // Generate a rectangle encompassing all of the provided rectangles. At least a single rectangle must be provided.
    template<c_numeric tp_type>
    s_rect<tp_type> CalcSpanningRect(const s_array_rdonly<s_rect<tp_type>> rects) {
        ZF_ASSERT(!rects.IsEmpty());

        tp_type min_left = rects[0].x;
        tp_type min_top = rects[0].y;
        tp_type max_right = rects[0].Right();
        tp_type max_bottom = rects[0].Bottom();

        for (t_s32 i = 1; i < rects.Len(); i++) {
            min_left = Min(rects[i].x, min_left);
            min_top = Min(rects[i].y, min_top);
            max_right = Max(rects[i].Right(), max_right);
            max_bottom = Max(rects[i].Bottom(), max_bottom);
        }

        return {
            min_left,
            min_top,
            max_right - min_left,
            max_bottom - min_top
        };
    }

    template<c_numeric tp_type>
    struct s_range {
        tp_type begin;
        tp_type end;
    };

    struct s_matrix_4x4 {
        s_static_array<s_static_array<t_f32, 4>, 4> elems;
    };

    constexpr s_matrix_4x4 MakeIdentityMatrix4x4() {
        s_matrix_4x4 mat = {};

        mat.elems[0][0] = 1.0f;
        mat.elems[1][1] = 1.0f;
        mat.elems[2][2] = 1.0f;
        mat.elems[3][3] = 1.0f;

        return mat;
    }

    inline s_matrix_4x4 MakeOrthographicMatrix4x4(const t_f32 left, const t_f32 right, const t_f32 bottom, const t_f32 top, const t_f32 z_near, const t_f32 z_far) {
        ZF_ASSERT(right > left);
        ZF_ASSERT(top < bottom);
        ZF_ASSERT(z_far > z_near);
        ZF_ASSERT(z_near < z_far);

        s_matrix_4x4 mat = {};
        mat.elems[0][0] = 2.0f / (right - left);
        mat.elems[1][1] = 2.0f / (top - bottom);
        mat.elems[2][2] = -2.0f / (z_far - z_near);
        mat.elems[3][0] = -(right + left) / (right - left);
        mat.elems[3][1] = -(top + bottom) / (top - bottom);
        mat.elems[3][2] = -(z_far + z_near) / (z_far - z_near);
        mat.elems[3][3] = 1.0f;
        return mat;
    }

    template<c_floating_point tp_type>
    constexpr tp_type Lerp(const tp_type a, const tp_type b, const tp_type t) {
        return a + ((b - a) * t);
    }

    template<c_floating_point tp_type>
    constexpr s_v2<tp_type> Lerp(const s_v2<tp_type> a, const s_v2<tp_type> b, const tp_type t) {
        return a + ((b - a) * t);
    }

    template<c_floating_point tp_type>
    inline s_v2<tp_type> LenDir(const tp_type len, const tp_type dir) {
        return s_v2<tp_type>(cos(dir), -sin(dir)) * len;
    }
}
