#pragma once

#include <cmath>
#include <zc/mem/mem.h>
#include <zc/mem/arrays.h>
#include <zc/type_traits.h>

namespace zf {
    constexpr float g_pi = 3.14159265358979323846f;
    constexpr float g_tau = 6.28318530717958647692f;

    template<co_numeric tp_type>
    static constexpr tp_type Min(const tp_type& a, const tp_type& b) {
        return a <= b ? a : b;
    }

    template<co_numeric tp_type>
    static constexpr tp_type Max(const tp_type& a, const tp_type& b) {
        return a >= b ? a : b;
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

    struct s_v2 {
        float x = 0.0f;
        float y = 0.0f;

        constexpr s_v2() = default;
        constexpr s_v2(const float x, const float y) : x(x), y(y) {}

        float Dot(const s_v2 other) const {
            return (x * other.x) + (y * other.y);
        }

        float Mag() const {
            return sqrtf((x * x) + (y * y));
        }

        s_v2 NormalizedOrZero() const {
            const float mag = Mag();

            if (mag == 0.0f) {
                return {};
            }

            return {x / mag, y / mag};
        }

        constexpr bool operator==(const s_v2& other) const {
            return x == other.x && y == other.y;
        }

        constexpr bool operator!=(const s_v2& other) const {
            return !(*this == other);
        }

        constexpr s_v2 operator-() const {
            return {-x, -y};
        }

        constexpr s_v2 operator+(const s_v2& other) const {
            return {x + other.x, y + other.y};
        }

        constexpr s_v2 operator-(const s_v2& other) const {
            return {x - other.x, y - other.y};
        }

        s_v2& operator+=(const s_v2& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        s_v2& operator-=(const s_v2& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr s_v2 operator*(const float scalar) const {
            return {x * scalar, y * scalar};
        }

        constexpr s_v2 operator/(const float scalar) const {
            return {x / scalar, y / scalar};
        }

        s_v2& operator*=(const float scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        s_v2& operator/=(const float scalar) {
            x /= scalar;
            y /= scalar;
            return *this;
        }
    };

    constexpr s_v2 operator*(float scalar, const s_v2& v) {
        return {v.x * scalar, v.y * scalar};
    }

    struct s_v2_int {
        int x = 0;
        int y = 0;

        constexpr s_v2_int() = default;
        constexpr s_v2_int(const int x, const int y) : x(x), y(y) {}

        constexpr bool operator==(const s_v2_int& other) const {
            return x == other.x && y == other.y;
        }

        constexpr bool operator!=(const s_v2_int& other) const {
            return !(*this == other);
        }

        operator s_v2() const {
            return {static_cast<float>(x), static_cast<float>(y)};
        }
    };

    struct s_v3 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        constexpr s_v3() = default;
        constexpr s_v3(const float x, const float y, const float z) : x(x), y(y), z(z) {}

        s_v2 XY() {
            return {x, y};
        }

        constexpr bool operator==(const s_v3& other) const {
            return x == other.x && y == other.y && z == other.z;
        }

        constexpr bool operator!=(const s_v3& other) const {
            return !(*this == other);
        }
    };

    struct s_v4 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 0.0f;

        constexpr s_v4() = default;
        constexpr s_v4(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}
        constexpr s_v4(const s_v3 v3, const float w) : x(v3.x), y(v3.y), z(v3.z), w(w) {}

        s_v3 XYZ() {
            return {x, y, z};
        }

        constexpr bool operator==(const s_v4& other) const {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        constexpr bool operator!=(const s_v4& other) const {
            return !(*this == other);
        }
    };

    struct s_rect {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;

        s_rect() = default;
        s_rect(const float x, const float y, const float width, const float height) : x(x), y(y), width(width), height(height) {}
        s_rect(const s_v2 pos, const s_v2 size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}
        s_rect(const s_v2 pos, const s_v2 size, const s_v2 origin) : x(pos.x - (size.x * origin.x)), y(pos.y - (size.y * origin.y)), width(size.x), height(size.y) {}

        s_v2 Pos() const {
            return {x, y};
        }

        s_v2 Size() const {
            return {width, height};
        }

        float Right() const {
            return x + width;
        }

        float Bottom() const {
            return y + height;
        }
    };

    // Generate a rectangle encompassing all of the provided rectangles.
    // At least a single rectangle must be provided.
    inline s_rect CalcSpanningRect(const c_array<const s_rect> rects) {
        ZF_ASSERT(!rects.IsEmpty());

        float min_left = rects[0].x;
        float min_top = rects[0].y;
        float max_right = rects[0].Right();
        float max_bottom = rects[0].Bottom();

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

    struct s_rect_int {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;

        s_rect_int() = default;
        s_rect_int(const int x, const int y, const int width, const int height) : x(x), y(y), width(width), height(height) {}
        s_rect_int(const s_v2_int pos, const s_v2_int size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

        s_v2_int Pos() const {
            return {x, y};
        }

        s_v2_int Size() const {
            return {width, height};
        }

        int Right() const {
            return x + width;
        }

        int Bottom() const {
            return y + height;
        }

        operator s_rect() const {
            return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)};
        }
    };

    template<co_numeric tp_type>
    class c_rect_edges {
    public:
        c_rect_edges() = default;

        c_rect_edges(const tp_type left, const tp_type top, const tp_type right, const tp_type bottom) : m_left(left), m_top(top), m_right(right), m_bottom(bottom) {
            ZF_ASSERT(left <= right);
            ZF_ASSERT(top <= bottom);
        }

        c_rect_edges(const s_v2 topleft, const s_v2 bottomright) : m_left(topleft.x), m_top(topleft.y), m_right(bottomright.x), m_bottom(bottomright.y) {
            ZF_ASSERT(topleft.x <= bottomright.x && topleft.y <= bottomright.y);
        }

        tp_type Left() const { return m_left; }
        tp_type Top() const { return m_top; }
        tp_type Right() const { return m_right; }
        tp_type Bottom() const { return m_bottom; }

        void SetLeft(const tp_type left) {
            ZF_ASSERT(left <= m_right);
            m_left = left;
        }

        void SetRight(const tp_type right) {
            ZF_ASSERT(right >= m_left);
            m_right = right;
        }

        void SetTop(const tp_type top) {
            ZF_ASSERT(top <= m_bottom);
            m_top = top;
        }

        void SetBottom(const tp_type bottom) {
            ZF_ASSERT(bottom >= m_top);
            m_bottom = bottom;
        }

    private:
        tp_type m_left = 0.0f;
        tp_type m_top = 0.0f;
        tp_type m_right = 0.0f;
        tp_type m_bottom = 0.0f;
    };

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

        void Translate(const s_v2 trans) {
            elems[3][0] += trans.x;
            elems[3][1] += trans.y;
        }

        void Scale(const float scalar) {
            elems[0][0] *= scalar;
            elems[1][1] *= scalar;
            elems[2][2] *= scalar;
        }
    };

    constexpr float Lerp(const float a, const float b, const float t) {
        return a + ((b - a) * t);
    }

    constexpr s_v2 Lerp(const s_v2 a, const s_v2 b, const float t) {
        return {Lerp(a.x, b.x, t), Lerp(a.y, b.y, t)};
    }

    inline s_v2 LenDir(const float len, const float dir) {
        return {cosf(dir) * len, -sinf(dir) * len};
    }
}
