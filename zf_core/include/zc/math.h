#pragma once

#include <cmath>
#include <zc/debug.h>
#include <zc/mem/mem.h>
#include <zc/mem/arrays.h>
#include <zc/type_traits.h>

namespace zf {
    template<co_floating_point tp_type> constexpr tp_type Pi();
    template<> constexpr float Pi<float>() { return 3.14159265358979323846f; }
    template<> constexpr double Pi<double>() { return 3.141592653589793238462643383279502884; }

    template<co_floating_point tp_type> constexpr tp_type Tau();
    template<> constexpr float Tau<float>() { return 6.28318530717958647692f; }
    template<> constexpr double Tau<double>() { return 6.283185307179586476925286766559005768; }

    template<co_floating_point tp_type>
    constexpr tp_type DegsToRads(const tp_type degs) {
        return degs * (Pi<tp_type>() / 180);
    }

    template<co_floating_point tp_type>
    constexpr tp_type RadsToDegs(const tp_type rads) {
        return rads * (180 / Pi<tp_type>());
    }

    template<co_numeric tp_type>
    static constexpr tp_type Min(const tp_type& a, const tp_type& b) {
        return a <= b ? a : b;
    }

    template<co_numeric tp_type>
    static constexpr tp_type Max(const tp_type& a, const tp_type& b) {
        return a >= b ? a : b;
    }

    template<co_numeric tp_type>
    constexpr tp_type Abs(const tp_type n) {
        return n < 0 ? -n : n;
    }

    template<co_numeric tp_type>
    constexpr int Sign(const tp_type n) {
        if (n > 0) {
            return 1;
        } else if (n < 0) {
            return -1;
        }

        return 0;
    }

    template<co_numeric tp_type>
    constexpr tp_type Clamp(const tp_type n, const tp_type min, const tp_type max) {
        ZF_ASSERT(min <= max);
        return n < 0 ? -n : n;
    }

    template<co_integral tp_type>
    constexpr tp_type WrapUpper(const tp_type val, const tp_type max_excl) {
        return ((val % max_excl) + max_excl) % max_excl;
    }

    template<co_integral tp_type>
    constexpr tp_type Wrap(const tp_type val, const tp_type min, const tp_type max_excl) {
        return min + WrapUpper(val - min, max_excl - min);
    }

    template<co_integral tp_type>
    constexpr tp_type DigitAt(const tp_type n, const unsigned int index) {
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

    template<co_integral tp_type>
    constexpr int DigitCnt(const tp_type n) {
        if (n < 0) {
            return DigitCnt(-n);
        }

        if (n < 10) {
            return 1;
        }

        return 1 + DigitCnt(n / 10);
    }

    template<co_floating_point tp_type>
    constexpr bool NearlyEqual(const tp_type val, const tp_type targ, const tp_type tol = 1e-5) {
        ZF_ASSERT(tol >= 0);
        return val >= targ - tol && val <= targ + tol;
    }

    template<co_numeric tp_type>
    struct s_v2 {
        tp_type x = 0;
        tp_type y = 0;

        tp_type Dot(const s_v2<tp_type> other) const {
            return (x * other.x) + (y * other.y);
        }

        constexpr bool operator==(const s_v2<tp_type>& other) const {
            return x == other.x && y == other.y;
        }

        constexpr bool operator!=(const s_v2<tp_type>& other) const {
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

        constexpr s_v2<tp_type> operator*(const tp_type scalar) const requires co_floating_point<tp_type> {
            return {x * scalar, y * scalar};
        }

        constexpr s_v2<tp_type> operator/(const tp_type scalar) const requires co_floating_point<tp_type> {
            return {x / scalar, y / scalar};
        }

        s_v2<tp_type>& operator*=(const tp_type scalar) requires co_floating_point<tp_type> {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        s_v2<tp_type>& operator/=(const tp_type scalar) requires co_floating_point<tp_type> {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        tp_type Mag() const requires co_floating_point<tp_type> {
            return sqrt((x * x) + (y * y));
        }

        s_v2<tp_type> NormalizedOrZero() const requires co_floating_point<tp_type> {
            const tp_type mag = Mag();

            if (mag == 0) {
                return {};
            }

            return {x / mag, y / mag};
        }

        inline s_v2<tp_type> DirTo(const s_v2<tp_type> other) requires co_floating_point<tp_type> {
            return (other - *this).NormalizedOrZero();
        }

        inline tp_type DirToInRads(const s_v2<tp_type> other) requires co_floating_point<tp_type> {
            return atan2(other.y - y, other.x - x);
        }
    };

    template<co_numeric tp_type>
    constexpr s_v2<tp_type> operator*(const tp_type scalar, const s_v2<tp_type>& v) {
        return {v.x * scalar, v.y * scalar};
    }

    template<co_numeric tp_type>
    struct s_v3 {
        tp_type x = 0;
        tp_type y = 0;
        tp_type z = 0;

        constexpr s_v3() = default;
        constexpr s_v3(const tp_type x, const tp_type y, const tp_type z) : x(x), y(y), z(z) {}

        s_v2<tp_type> XY() {
            return {x, y};
        }

        constexpr bool operator==(const s_v3& other) const {
            return x == other.x && y == other.y && z == other.z;
        }

        constexpr bool operator!=(const s_v3& other) const {
            return !(*this == other);
        }
    };

    template<co_numeric tp_type>
    struct s_v4 {
        tp_type x = 0;
        tp_type y = 0;
        tp_type z = 0;
        tp_type w = 0;

        constexpr s_v4() = default;
        constexpr s_v4(const tp_type x, const tp_type y, const tp_type z, const tp_type w) : x(x), y(y), z(z), w(w) {}
        constexpr s_v4(const s_v3<tp_type> v3, const tp_type w) : x(v3.x), y(v3.y), z(v3.z), w(w) {}

        s_v3<tp_type> XYZ() {
            return {x, y, z};
        }

        constexpr bool operator==(const s_v4& other) const {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        constexpr bool operator!=(const s_v4& other) const {
            return !(*this == other);
        }
    };

    template<co_numeric tp_type>
    struct s_rect {
        tp_type x = 0;
        tp_type y = 0;
        tp_type width = 0;
        tp_type height = 0;

        s_rect() = default;
        s_rect(const tp_type x, const tp_type y, const tp_type width, const tp_type height) : x(x), y(y), width(width), height(height) {}
        s_rect(const s_v2<tp_type> pos, const s_v2<tp_type> size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

        s_v2<tp_type> Pos() const {
            return {x, y};
        }

        s_v2<tp_type> Size() const {
            return {width, height};
        }

        tp_type Left() const {
            return x;
        }

        tp_type Right() const {
            return x + width;
        }

        tp_type Top() const {
            return y;
        }

        tp_type Bottom() const {
            return y + height;
        }

        constexpr bool operator==(const s_rect<tp_type>& other) const {
            return x == other.x && y == other.y;
        }

        constexpr bool operator!=(const s_rect<tp_type>& other) const {
            return !(*this == other);
        }

        bool Contains(const s_v2<tp_type> pt) const {
            return pt.x > Left() && pt.y > Top() && pt.x < Right() && pt.y < Bottom();
        }

        bool Intersects(const s_rect<tp_type> other) const {
            return Left() < other.Right() && Top() < other.Bottom() && Right() > other.Left() && Bottom() > other.Top();
        }

        bool Touches(const s_v2<tp_type> pt) const {
            return pt.x >= Left() && pt.y >= Top() && pt.x <= Right() && pt.y <= Bottom();
        }

        bool Touches(const s_rect<tp_type> other) const {
            return Left() <= other.Right() && Top() <= other.Bottom() && Right() >= other.Left() && Bottom() >= other.Top();
        }
    };

    // Generate a rectangle encompassing all of the provided rectangles.
    // At least a single rectangle must be provided.
    template<co_numeric tp_type>
    s_rect<tp_type> CalcSpanningRect(const c_array<const s_rect<tp_type>> rects) {
        ZF_ASSERT(!rects.IsEmpty());

        tp_type min_left = rects[0].x;
        tp_type min_top = rects[0].y;
        tp_type max_right = rects[0].Right();
        tp_type max_bottom = rects[0].Bottom();

        for (int i = 1; i < rects.Len(); i++) {
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

    struct s_matrix_4x4 {
        s_static_array<s_static_array<float, 4>, 4> elems;

        static s_matrix_4x4 Identity() {
            s_matrix_4x4 mat;

            mat.elems[0][0] = 1.0f;
            mat.elems[1][1] = 1.0f;
            mat.elems[2][2] = 1.0f;
            mat.elems[3][3] = 1.0f;

            return mat;
        }

        static s_matrix_4x4 Orthographic(const float left, const float right, const float bottom, const float top, const float z_near, const float z_far) {
            ZF_ASSERT(right > left);
            ZF_ASSERT(top < bottom);
            ZF_ASSERT(z_far > z_near);
            ZF_ASSERT(z_near < z_far);

            s_matrix_4x4 mat;
            mat.elems[0][0] = 2.0f / (right - left);
            mat.elems[1][1] = 2.0f / (top - bottom);
            mat.elems[2][2] = -2.0f / (z_far - z_near);
            mat.elems[3][0] = -(right + left) / (right - left);
            mat.elems[3][1] = -(top + bottom) / (top - bottom);
            mat.elems[3][2] = -(z_far + z_near) / (z_far - z_near);
            mat.elems[3][3] = 1.0f;
            return mat;
        }

        float* Raw() {
            return elems[0].buf_raw;
        }

        const float* Raw() const {
            return elems[0].buf_raw;
        }

        void Translate(const s_v2<float> trans) {
            elems[3][0] += trans.x;
            elems[3][1] += trans.y;
        }

        void Scale(const float scalar) {
            elems[0][0] *= scalar;
            elems[1][1] *= scalar;
            elems[2][2] *= scalar;
        }
    };

    template<co_floating_point tp_type>
    constexpr tp_type Lerp(const tp_type a, const tp_type b, const tp_type t) {
        return a + ((b - a) * t);
    }

    template<co_floating_point tp_type>
    constexpr tp_type Lerp(const s_v2<tp_type> a, const s_v2<tp_type> b, const tp_type t) {
        return a + ((b - a) * t);
    }

    template<co_floating_point tp_type>
    inline s_v2<tp_type> LenDir(const tp_type len, const tp_type dir) {
        return s_v2<tp_type>(cos(dir), -sin(dir)) * len;
    }

    template<co_numeric tp_type>
    tp_type FindClosest(const c_array<const tp_type> nums, const tp_type targ) {
        ZF_ASSERT(!nums.IsEmpty());

        int index_of_closest = 0;
        tp_type closest_diff = Abs(nums[0] - targ);

        for (int i = 1; i < nums.Len(); i++) {
            const tp_type diff = Abs(nums[i] - targ);

            if (diff < closest_diff) {
                index_of_closest = i;
                closest_diff = diff;
            }
        }

        return nums[index_of_closest];
    }
}
