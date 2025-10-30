#pragma once

#include <cmath>
#include <cassert>
#include "mem/zc_static_array.h"

namespace zf {
    constexpr float g_pi = 3.14159265358979323846f;
    constexpr float g_tau = 6.28318530717958647692f;

    constexpr int DigitAt(const int n, const unsigned int index) {
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

    constexpr int DigitCnt(const int n) {
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

        constexpr s_v2 operator*(float scalar) const {
            return {x * scalar, y * scalar};
        }

        constexpr s_v2 operator/(float scalar) const {
            return {x / scalar, y / scalar};
        }

        s_v2& operator*=(float scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        s_v2& operator/=(float scalar) {
            x /= scalar;
            y /= scalar;
            return *this;
        }
    };

    constexpr s_v2 operator*(float scalar, const s_v2& v) {
        return {v.x * scalar, v.y * scalar};
    }

    struct s_v2_s32 {
        t_s32 x = 0;
        t_s32 y = 0;

        constexpr s_v2_s32() = default;
        constexpr s_v2_s32(const t_s32 x, const t_s32 y) : x(x), y(y) {}

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

        bool operator==(const s_rect& other) const {
            return x == other.x && y == other.y && width == other.width && height == other.height;
        }
    };

    struct s_rect_s32 {
        t_s32 x = 0;
        t_s32 y = 0;
        t_s32 width = 0;
        t_s32 height = 0;

        s_rect_s32() = default;
        s_rect_s32(const t_s32 x, const t_s32 y, const t_s32 width, const t_s32 height) : x(x), y(y), width(width), height(height) {}
        s_rect_s32(const s_v2_s32 pos, const s_v2_s32 size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

        s_v2_s32 Pos() const {
            return {x, y};
        }

        s_v2_s32 Size() const {
            return {width, height};
        }

        t_s32 Right() const {
            return x + width;
        }

        t_s32 Bottom() const {
            return y + height;
        }

        operator s_rect() const {
            return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)};
        }

        bool operator==(const s_rect_s32& other) const {
            return x == other.x && y == other.y && width == other.width && height == other.height;
        }
    };

    struct s_rect_edges {
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;
    };

    struct s_rect_edges_s32 {
        t_s32 left = 0;
        t_s32 top = 0;
        t_s32 right = 0;
        t_s32 bottom = 0;
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

        static s_matrix_4x4 Orthographic(const float left, const float right, const float bottom, const float top, const float near, const float far) {
            assert(right > left);
            assert(top < bottom);
            assert(far > near);
            assert(near < far);

            s_matrix_4x4 mat;
            mat.elems[0][0] = 2.0f / (right - left);
            mat.elems[1][1] = 2.0f / (top - bottom);
            mat.elems[2][2] = -2.0f / (far - near);
            mat.elems[3][0] = -(right + left) / (right - left);
            mat.elems[3][1] = -(top + bottom) / (top - bottom);
            mat.elems[3][2] = -(far + near) / (far - near);
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

    struct s_poly {
        c_array<const s_v2> pts;
    };

    s_poly GenQuadPoly(c_mem_arena& mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin);
    s_poly GenQuadPolyRotated(c_mem_arena& mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin, const float rot);

    bool DoPolysInters(const s_poly a, const s_poly b);
    bool DoesPolyIntersWithRect(const s_poly poly, const s_rect rect);
    s_rect_edges PolySpan(const s_poly poly);

    static inline s_v2 LenDir(const float len, const float dir) {
        return {cosf(dir) * len, -sinf(dir) * len};
    }

    static inline float Lerp(const float a, const float b, const float t) {
        return a + ((b - a) * t);
    }

    static inline s_v2 Lerp(const s_v2 a, const s_v2 b, const float t) {
        return {Lerp(a.x, b.x, t), Lerp(a.y, b.y, t)};
    }
}
