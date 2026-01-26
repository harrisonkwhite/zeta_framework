#pragma once

#include <zcl/zcl_basic.h>

namespace zcl {
    // ============================================================
    // @section: Types and Constants
    // ============================================================

    // Putting all of these types and things up here since the math functions are so interrelated.

    constexpr t_f32 k_pi = 3.14159265358979323846f;
    constexpr t_f32 k_tau = 6.28318530717958647692f;

    constexpr zcl::t_f32 k_tolerance_default = 1e-5f;

    struct t_v2 {
        t_f32 x;
        t_f32 y;

        constexpr t_v2 operator-() const { return {-x, -y}; }

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

        constexpr t_v2_i operator-() const { return {-x, -y}; }

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

        constexpr t_v3 operator-() const { return {-x, -y, -z}; }

        constexpr t_v3 operator+(const t_v3 &other) const { return {x + other.x, y + other.y, z + other.z}; }
        constexpr t_v3 operator-(const t_v3 &other) const { return {x - other.x, y - other.y, z - other.z}; }
        constexpr t_v3 operator*(const t_f32 scalar) const { return {x * scalar, y * scalar, z * scalar}; }
        constexpr t_v3 operator/(const t_f32 divisor) const { return {x / divisor, y / divisor, z / divisor}; }

        constexpr t_v3 &operator+=(const t_v3 &other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        constexpr t_v3 &operator-=(const t_v3 &other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        constexpr t_v3 &operator*=(const t_f32 scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        constexpr t_v3 &operator/=(const t_f32 divisor) {
            x /= divisor;
            y /= divisor;
            z /= divisor;
            return *this;
        }
    };

    struct t_v4 {
        t_f32 x;
        t_f32 y;
        t_f32 z;
        t_f32 w;

        constexpr t_v4 operator-() const { return {-x, -y, -z, -w}; }

        constexpr t_v4 operator+(const t_v4 &other) const { return {x + other.x, y + other.y, z + other.z, w + other.w}; }
        constexpr t_v4 operator-(const t_v4 &other) const { return {x - other.x, y - other.y, z - other.z, w - other.w}; }
        constexpr t_v4 operator*(const t_f32 scalar) const { return {x * scalar, y * scalar, z * scalar, w * scalar}; }
        constexpr t_v4 operator/(const t_f32 divisor) const { return {x / divisor, y / divisor, z / divisor, w / divisor}; }

        constexpr t_v4 &operator+=(const t_v4 &other) {
            x += other.x;
            y += other.y;
            z += other.y;
            w += other.y;
            return *this;
        }

        constexpr t_v4 &operator-=(const t_v4 &other) {
            x -= other.x;
            y -= other.y;
            z -= other.y;
            w -= other.y;
            return *this;
        }

        constexpr t_v4 &operator*=(const t_f32 scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;
            return *this;
        }

        constexpr t_v4 &operator/=(const t_f32 divisor) {
            x /= divisor;
            y /= divisor;
            z /= divisor;
            w /= divisor;
            return *this;
        }
    };

    struct t_range {
        t_f32 min;
        t_f32 max;
    };

    struct t_range_excl_lower {
        t_f32 min;
        t_f32 max;
    };

    struct t_range_excl_upper {
        t_f32 min;
        t_f32 max;
    };

    struct t_range_excl_both {
        t_f32 min;
        t_f32 max;
    };

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

    struct t_mat4x4 {
        t_static_array<t_static_array<t_f32, 4>, 4> elems;
    };

    struct t_poly_rdonly {
        t_array_rdonly<t_v2> pts;
    };

    struct t_poly_mut {
        t_array_mut<t_v2> pts;

        operator t_poly_rdonly() const {
            return {.pts = pts};
        }
    };

    // ============================================================


    constexpr t_v2 AsV2F(const t_v2_i v) {
        return {static_cast<t_f32>(v.x), static_cast<t_f32>(v.y)};
    }

    constexpr t_v2_i AsV2I(const t_v2 v) {
        return {static_cast<t_i32>(v.x), static_cast<t_i32>(v.y)};
    }

    constexpr t_rect_f AsRectF(const t_rect_i rect) {
        return {static_cast<t_f32>(rect.x), static_cast<t_f32>(rect.y), static_cast<t_f32>(rect.width), static_cast<t_f32>(rect.height)};
    }

    constexpr t_rect_i AsRectI(const t_rect_f rect) {
        return {static_cast<t_i32>(rect.x), static_cast<t_i32>(rect.y), static_cast<t_i32>(rect.width), static_cast<t_i32>(rect.height)};
    }

    constexpr t_b8 CheckNearlyEqual(const t_f32 val, const t_f32 targ, const t_f32 tol = k_tolerance_default) {
        ZCL_ASSERT(!CheckNaN(val));
        ZCL_ASSERT(!CheckNaN(targ));
        ZCL_ASSERT(tol >= 0.0f);

        return val >= targ - tol && val <= targ + tol;
    }

    constexpr t_b8 CheckNearlyEqual(const t_v2 val, const t_v2 targ, const t_f32 tol = k_tolerance_default) {
        return CheckNearlyEqual(val.x, targ.x, tol) && CheckNearlyEqual(val.y, targ.y, tol);
    }

    constexpr t_b8 CheckNearlyEqual(const t_v3 val, const t_v3 targ, const t_f32 tol = k_tolerance_default) {
        return CheckNearlyEqual(val.x, targ.x, tol)
            && CheckNearlyEqual(val.y, targ.y, tol)
            && CheckNearlyEqual(val.z, targ.z, tol);
    }

    constexpr t_b8 CheckNearlyEqual(const t_v4 val, const t_v4 targ, const t_f32 tol = k_tolerance_default) {
        return CheckNearlyEqual(val.x, targ.x, tol)
            && CheckNearlyEqual(val.y, targ.y, tol)
            && CheckNearlyEqual(val.z, targ.z, tol)
            && CheckNearlyEqual(val.w, targ.w, tol);
    }

    constexpr t_f32 Snap(const t_f32 val, const t_f32 targ, const t_f32 tol = k_tolerance_default) {
        return CheckNearlyEqual(val, targ, tol) ? targ : val;
    }

    constexpr t_f32 Lerp(const t_f32 a, const t_f32 b, const t_f32 t) {
        return a + ((b - a) * t);
    }

    constexpr t_v2 Lerp(const t_v2 a, const t_v2 b, const t_f32 t) {
        return a + ((b - a) * t);
    }

    constexpr t_f32 DegsToRads(const t_f32 degs) {
        return degs * (k_pi / 180.0f);
    }

    constexpr t_f32 RadsToDegs(const t_f32 rads) {
        return rads * (180.0f / k_pi);
    }

    template <c_integral tp_type>
    constexpr t_i32 CalcDigitCount(const tp_type n) {
        if (n < 0) {
            return CalcDigitCount(-n);
        }

        if (n < 10) {
            return 1;
        }

        return 1 + CalcDigitCount(n / 10);
    }

    template <c_integral tp_type>
    constexpr tp_type CalcDigitAt(const tp_type n, const t_i32 index) {
        ZCL_ASSERT(index >= 0 && index < CalcDigitCount(n));

        if (n < 0) {
            return CalcDigitAt(-n, index);
        }

        if (index == 0) {
            return n % 10;
        }

        if (n < 10) {
            return 0;
        }

        return CalcDigitAt(n / 10, index - 1);
    }

    constexpr t_range RangeCreate(const t_f32 min, const t_f32 max) {
        ZCL_ASSERT(!CheckNaN(min) && !CheckNaN(max));
        ZCL_ASSERT(min <= max);

        return {min, max};
    }

    constexpr t_range RangeCreateExclLower(const t_f32 min, const t_f32 max) {
        ZCL_ASSERT(!CheckNaN(min) && !CheckNaN(max));
        ZCL_ASSERT(min <= max);

        return {min, max};
    }

    constexpr t_range RangeCreateExclUpper(const t_f32 min, const t_f32 max) {
        ZCL_ASSERT(!CheckNaN(min) && !CheckNaN(max));
        ZCL_ASSERT(min <= max);

        return {min, max};
    }

    constexpr t_range RangeCreateExclBoth(const t_f32 min, const t_f32 max) {
        ZCL_ASSERT(!CheckNaN(min) && !CheckNaN(max));
        ZCL_ASSERT(min <= max);

        return {min, max};
    }

    constexpr t_b8 RangeCheckEmpty(const t_range range) { return false; }
    constexpr t_b8 RangeCheckEmpty(const t_range_excl_lower range) { return CheckNearlyEqual(range.min, range.max); }
    constexpr t_b8 RangeCheckEmpty(const t_range_excl_upper range) { return CheckNearlyEqual(range.min, range.max); }
    constexpr t_b8 RangeCheckEmpty(const t_range_excl_both range) { return CheckNearlyEqual(range.min, range.max); }

    constexpr t_b8 RangeValueCheckWithin(const t_range range, const t_f32 val) {
        ZCL_ASSERT(!CheckNaN(val));
        return val >= range.min && val <= range.max;
    }

    constexpr t_b8 RangeValueCheckWithin(const t_range_excl_lower range, const t_f32 val) {
        ZCL_ASSERT(!CheckNaN(val));
        return val > range.min && val <= range.max;
    }

    constexpr t_b8 RangeValueCheckWithin(const t_range_excl_upper range, const t_f32 val) {
        ZCL_ASSERT(!CheckNaN(val));
        return val >= range.min && val < range.max;
    }

    constexpr t_b8 RangeValueCheckWithin(const t_range_excl_both range, const t_f32 val) {
        ZCL_ASSERT(!CheckNaN(val));
        return val > range.min && val < range.max;
    }

    constexpr t_f32 RangeValueSnapToBounds(const t_range range, const t_f32 val, const t_f32 tol = k_tolerance_default) {
        ZCL_ASSERT(!CheckNaN(val));

        if (val < range.min && CheckNearlyEqual(val, range.min, tol)) {
            return range.min;
        }

        if (val > range.max && CheckNearlyEqual(val, range.max, tol)) {
            return range.max;
        }

        return val;
    }

    // Checks if the value - snapped to the range bounds - is within the range.
    constexpr t_b8 RangeValueCheckWithinSnapped(const t_range range, const t_f32 val, const t_f32 tol = k_tolerance_default) {
        return RangeValueCheckWithin(range, RangeValueSnapToBounds(range, val, tol));
    }

    constexpr t_rect_f RectCreateF(const t_f32 x, const t_f32 y, const t_f32 width, const t_f32 height) {
        ZCL_ASSERT(!CheckNaN(x) && !CheckNaN(y) && !CheckNaN(width) && !CheckNaN(height));
        ZCL_ASSERT(width >= 0.0f && height >= 0.0f);

        return {x, y, width, height};
    }

    constexpr t_rect_f RectCreateF(const t_v2 pos, const t_v2 size) {
        return RectCreateF(pos.x, pos.y, size.x, size.y);
    }

    constexpr t_rect_i RectCreateI(const t_i32 x, const t_i32 y, const t_i32 width, const t_i32 height) {
        ZCL_ASSERT(width >= 0 && height >= 0);
        return {x, y, width, height};
    }

    constexpr t_rect_i RectCreateI(const t_v2_i pos, const t_v2_i size) {
        return RectCreateI(pos.x, pos.y, size.x, size.y);
    }

    constexpr t_v2 RectGetPos(const t_rect_f rect) { return {rect.x, rect.y}; }
    constexpr t_v2_i RectGetPos(const t_rect_i rect) { return {rect.x, rect.y}; }

    constexpr t_v2 RectGetSize(const t_rect_f rect) { return {rect.width, rect.height}; }
    constexpr t_v2_i RectGetSize(const t_rect_i rect) { return {rect.width, rect.height}; }

    constexpr t_v2 RectGetCenter(const t_rect_f rect) { return {rect.x + (rect.width / 2.0f), rect.y + (rect.height / 2.0f)}; }

    constexpr t_f32 RectGetLeft(const t_rect_f rect) { return rect.x; }
    constexpr t_i32 RectGetLeft(const t_rect_i rect) { return rect.x; }

    constexpr t_f32 RectGetTop(const t_rect_f rect) { return rect.y; }
    constexpr t_i32 RectGetTop(const t_rect_i rect) { return rect.y; }

    constexpr t_f32 RectGetRight(const t_rect_f rect) { return rect.x + rect.width; }
    constexpr t_i32 RectGetRight(const t_rect_i rect) { return rect.x + rect.width; }

    constexpr t_f32 RectGetBottom(const t_rect_f rect) { return rect.y + rect.height; }
    constexpr t_i32 RectGetBottom(const t_rect_i rect) { return rect.y + rect.height; }

    constexpr t_v2 RectGetTopLeft(const t_rect_f rect) { return {RectGetLeft(rect), RectGetTop(rect)}; }
    constexpr t_v2_i RectGetTopLeft(const t_rect_i rect) { return {RectGetLeft(rect), RectGetTop(rect)}; }

    constexpr t_v2 RectGetTopRight(const t_rect_f rect) { return {RectGetRight(rect), RectGetTop(rect)}; }
    constexpr t_v2_i RectGetTopRight(const t_rect_i rect) { return {RectGetRight(rect), RectGetTop(rect)}; }

    constexpr t_v2 RectGetBottomLeft(const t_rect_f rect) { return {RectGetLeft(rect), RectGetBottom(rect)}; }
    constexpr t_v2_i RectGetBottomLeft(const t_rect_i rect) { return {RectGetLeft(rect), RectGetBottom(rect)}; }

    constexpr t_v2 RectGetBottomRight(const t_rect_f rect) { return {RectGetRight(rect), RectGetBottom(rect)}; }
    constexpr t_v2_i RectGetBottomRight(const t_rect_i rect) { return {RectGetRight(rect), RectGetBottom(rect)}; }

    constexpr t_f32 RectGetArea(const t_rect_f rect) { return rect.width * rect.height; }
    constexpr t_i32 RectGetArea(const t_rect_i rect) { return rect.width * rect.height; }

    constexpr t_mat4x4 MatrixCreateIdentity() {
        t_mat4x4 result = {};
        result.elems[0][0] = 1.0f;
        result.elems[1][1] = 1.0f;
        result.elems[2][2] = 1.0f;
        result.elems[3][3] = 1.0f;

        return result;
    }

    constexpr t_mat4x4 MatrixCreateTranslated(const t_v2 offs) {
        t_mat4x4 result = MatrixCreateIdentity();
        result.elems[3][0] = offs.x;
        result.elems[3][1] = offs.y;

        return result;
    }

    t_mat4x4 MatrixCreateRotated(const t_f32 rot);

    constexpr t_mat4x4 MatrixCreateScaled(const t_v2 scalar) {
        t_mat4x4 result = MatrixCreateIdentity();
        result.elems[0][0] *= scalar.x;
        result.elems[1][1] *= scalar.y;

        return result;
    }

    constexpr t_mat4x4 MatrixAdd(const t_mat4x4 &a, const t_mat4x4 &b) {
        t_mat4x4 result;

        for (t_i32 i = 0; i < 4; i++) {
            for (t_i32 j = 0; j < 4; j++) {
                result.elems[i][j] = a.elems[i][j] + b.elems[i][j];
            }
        }

        return result;
    }

    constexpr t_mat4x4 MatrixSubtract(const t_mat4x4 &a, const t_mat4x4 &b) {
        t_mat4x4 result;

        for (t_i32 i = 0; i < 4; i++) {
            for (t_i32 j = 0; j < 4; j++) {
                result.elems[i][j] = a.elems[i][j] - b.elems[i][j];
            }
        }

        return result;
    }

    constexpr t_mat4x4 MatrixMultiply(const t_mat4x4 &a, const t_mat4x4 &b) {
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

    // Points are guaranteed to be in this order: top-left, top-right, bottom-right, bottom-left.
    t_poly_mut PolyCreateQuad(const t_v2 pos, const t_v2 size, const t_v2 origin, t_arena *const arena);

    // Points are guaranteed to be in this order: top-left, top-right, bottom-right, bottom-left.
    t_poly_mut PolyCreateQuadRotated(const t_v2 pos, const t_v2 size, const t_v2 origin, const t_f32 rot, t_arena *const arena);

    constexpr t_f32 CalcDotProd(const t_v2 a, const t_v2 b) { return (a.x * b.x) + (a.y * b.y); }
    constexpr t_i32 CalcDotProd(const t_v2_i a, const t_v2_i b) { return (a.x * b.x) + (a.y * b.y); }

    constexpr t_f32 CalcCrossProd(const t_v2 a, const t_v2 b) { return (a.x * b.y) - (a.y * b.x); }
    constexpr t_i32 CalcCrossProd(const t_v2_i a, const t_v2_i b) { return (a.x * b.y) - (a.y * b.x); }

    constexpr t_v2 CalcCompwiseProd(const t_v2 a, const t_v2 b) { return {a.x * b.x, a.y * b.y}; }
    constexpr t_v2_i CalcCompwiseProd(const t_v2_i a, const t_v2_i b) { return {a.x * b.x, a.y * b.y}; }

    t_f32 CalcMag(const t_v2 v);

    // Returns {} if a divide by 0 is attempted.
    inline t_v2 CalcNormal(const t_v2 v) {
        const t_f32 mag = CalcMag(v);

        if (mag == 0.0f) {
            return {};
        }

        return {v.x / mag, v.y / mag};
    }

    inline t_f32 CalcDist(const t_v2 a, const t_v2 b) {
        return CalcMag(b - a);
    }

    inline t_v2 CalcDir(const t_v2 a, const t_v2 b) {
        return CalcNormal(b - a);
    }

    // Returns 0 if the horizontal and vertical differences of the vectors are 0.
    t_f32 CalcDirRads(const t_v2 a, const t_v2 b);

    t_v2 CalcLengthDir(const t_f32 len, const t_f32 dir);

    constexpr t_b8 CheckPointInRect(const t_v2 pt, const t_rect_f rect) {
        return pt.x > RectGetLeft(rect) && pt.y > RectGetTop(rect) && pt.x < RectGetRight(rect) && pt.y < RectGetBottom(rect);
    }

    constexpr t_b8 CheckPointInRect(const t_v2_i pt, const t_rect_i rect) {
        return pt.x > RectGetLeft(rect) && pt.y > RectGetTop(rect) && pt.x < RectGetRight(rect) && pt.y < RectGetBottom(rect);
    }

    constexpr t_v2 ClampWithinContainer(const t_v2 pt, const t_rect_f container) {
        return {Clamp(pt.x, RectGetLeft(container), RectGetRight(container)), Clamp(pt.y, RectGetTop(container), RectGetBottom(container))};
    }

    constexpr t_v2_i ClampWithinContainer(const t_v2_i pt, const t_rect_i container) {
        return {Clamp(pt.x, RectGetLeft(container), RectGetRight(container)), Clamp(pt.y, RectGetTop(container), RectGetBottom(container))};
    }

    constexpr t_rect_f ClampWithinContainer(const t_rect_f rect, const t_rect_f container) {
        const t_v2 tl = {CalcMax(rect.x, container.x), CalcMax(rect.y, container.y)};
        return {tl.x, tl.y, CalcMax(CalcMin(RectGetRight(rect), RectGetRight(container)) - tl.x, 0.0f), CalcMax(CalcMin(RectGetBottom(rect), RectGetBottom(container)) - tl.y, 0.0f)};
    }

    constexpr t_rect_i ClampWithinContainer(const t_rect_i rect, const t_rect_i container) {
        const t_v2_i tl = {CalcMax(rect.x, container.x), CalcMax(rect.y, container.y)};
        return {tl.x, tl.y, CalcMax(CalcMin(RectGetRight(rect), RectGetRight(container)) - tl.x, 0), CalcMax(CalcMin(RectGetBottom(rect), RectGetBottom(container)) - tl.y, 0)};
    }

    // Returns a value between 0 and 1 indicating what percentage of the rectangle is within the container.
    constexpr t_f32 CalcPercOfOccupance(const t_rect_f rect, const t_rect_f container) {
        ZCL_ASSERT(container.width > 0 && container.height > 0);

        const auto subrect = ClampWithinContainer(rect, container);
        return Clamp(RectGetArea(subrect) / RectGetArea(container), 0.0f, 1.0f);
    }

    // Returns a value between 0 and 1 indicating what percentage of the rectangle is within the container.
    constexpr t_f32 CalcPercOfOccupance(const t_rect_i rect, const t_rect_i container) {
        ZCL_ASSERT(container.width > 0 && container.height > 0);

        const auto subrect = ClampWithinContainer(rect, container);
        return Clamp(static_cast<t_f32>(RectGetArea(subrect)) / static_cast<t_f32>(RectGetArea(container)), 0.0f, 1.0f);
    }

    constexpr t_b8 CheckEqual(const t_rect_i a, const t_rect_i b) {
        return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
    }

    t_rect_f CalcSpanningRect(const t_array_mut<t_v2> pts);
    t_rect_i CalcSpanningRect(const t_array_mut<t_v2_i> pts);
    t_rect_f CalcSpanningRect(const t_array_mut<t_rect_f> rects);
    t_rect_i CalcSpanningRect(const t_array_mut<t_rect_i> rects);
    t_rect_f CalcSpanningRect(const t_poly_rdonly poly);

    t_poly_mut CalcSpanningPoly(const t_rect_f a, const t_rect_f b, t_arena *const arena);

    t_b8 CheckInters(const t_poly_rdonly poly_a, const t_poly_rdonly poly_b);

    t_b8 CheckInters(const t_poly_rdonly poly, const t_rect_f rect);

    inline t_b8 CheckInters(const t_rect_f rect, const t_poly_rdonly poly) {
        return CheckInters(poly, rect);
    }

    t_b8 CheckInters(const t_poly_rdonly poly, const t_v2 segment_begin, const t_v2 segment_end);

    inline t_b8 CheckInters(const t_v2 segment_begin, const t_v2 segment_end, const t_poly_rdonly poly) {
        return CheckInters(poly, segment_begin, segment_end);
    }

    constexpr t_b8 CheckInters(const t_rect_f a, const t_rect_f b) {
        return RectGetLeft(a) < RectGetRight(b) && RectGetTop(a) < RectGetBottom(b) && RectGetRight(a) > RectGetLeft(b) && RectGetBottom(a) > RectGetTop(b);
    }

    constexpr t_b8 CheckInters(const t_rect_i a, const t_rect_i b) {
        return RectGetLeft(a) < RectGetRight(b) && RectGetTop(a) < RectGetBottom(b) && RectGetRight(a) > RectGetLeft(b) && RectGetBottom(a) > RectGetTop(b);
    }

    t_b8 CheckInters(const t_v2 seg_a_begin, const t_v2 seg_a_end, const t_v2 seg_b_begin, const t_v2 seg_b_end, const t_f32 tol = k_tolerance_default);

    t_b8 CheckPointOnSegment(const t_v2 seg_begin, const t_v2 seg_end, const t_v2 pt, const t_f32 tol = k_tolerance_default);
    t_b8 CheckCross(const t_v2 seg_a_begin, const t_v2 seg_a_end, const t_v2 seg_b_begin, const t_v2 seg_b_end, const t_f32 tol = k_tolerance_default);
}
