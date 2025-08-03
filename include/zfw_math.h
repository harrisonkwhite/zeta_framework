#ifndef ZFW_MATH_H
#define ZFW_MATH_H

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <cu.h>

typedef struct {
    float x;
    float y;
} zfw_s_vec_2d;

typedef struct {
    t_s32 x;
    t_s32 y;
} zfw_s_vec_2d_s32;

typedef union {
    struct {
        float x;
        float y;
        float z;
    };

    struct {
        float r;
        float g;
        float b;
    };
} zfw_u_vec_3d;

typedef union {
    struct {
        float x;
        float y;
        float z;
        float w;
    };

    struct {
        float r;
        float g;
        float b;
        float a;
    };
} zfw_u_vec_4d;

typedef enum {
    zfw_ek_cardinal_dir_right,
    zfw_ek_cardinal_dir_left,
    zfw_ek_cardinal_dir_down,
    zfw_ek_cardinal_dir_up
} zfw_e_cardinal_dir;

static const zfw_s_vec_2d_s32 zfw_g_cardinal_dir_vecs[] = {
    [zfw_ek_cardinal_dir_right] = {1, 0},
    [zfw_ek_cardinal_dir_left] = {-1, 0},
    [zfw_ek_cardinal_dir_down] = {0, 1},
    [zfw_ek_cardinal_dir_up] = {0, -1}
};

typedef struct {
    float x;
    float y;
    float width;
    float height;
} zfw_s_rect;

typedef struct {
    t_s32 x;
    t_s32 y;
    t_s32 width;
    t_s32 height;
} zfw_s_rect_s32;

typedef struct {
    float left;
    float top;
    float right;
    float bottom;
} zfw_s_rect_edges;

typedef struct {
    t_s32 left;
    t_s32 top;
    t_s32 right;
    t_s32 bottom;
} zfw_s_rect_edges_s32;

typedef struct {
    float elems[4][4];
} zfw_s_matrix_4x4;

typedef struct {
    const zfw_s_vec_2d* pts;
    int cnt;
} zfw_s_poly;

zfw_s_rect ZFW_GenSpanningRect(const zfw_s_rect* const rects, const int cnt);

zfw_s_poly ZFW_PushQuadPoly(s_mem_arena* const mem_arena, const zfw_s_vec_2d pos, const zfw_s_vec_2d size, const zfw_s_vec_2d origin);
zfw_s_poly ZFW_PushQuadPolyRotated(s_mem_arena* const mem_arena, const zfw_s_vec_2d pos, const zfw_s_vec_2d size, const zfw_s_vec_2d origin, const float rot);
bool ZFW_DoPolysInters(const zfw_s_poly a, const zfw_s_poly b);
bool ZFW_DoesPolyIntersWithRect(const zfw_s_poly poly, const zfw_s_rect rect);
zfw_s_rect_edges ZFW_PolySpan(const zfw_s_poly poly);

static inline zfw_s_vec_2d ZFW_Vec2DSum(const zfw_s_vec_2d a, const zfw_s_vec_2d b) {
    return (zfw_s_vec_2d){a.x + b.x, a.y + b.y};
}

static inline zfw_s_vec_2d_s32 ZFW_Vec2DS32Sum(const zfw_s_vec_2d_s32 a, const zfw_s_vec_2d_s32 b) {
    return (zfw_s_vec_2d_s32){a.x + b.x, a.y + b.y};
}

static inline zfw_s_vec_2d ZFW_Vec2DScaled(const zfw_s_vec_2d vec, const float scalar) {
    return (zfw_s_vec_2d){vec.x * scalar, vec.y * scalar};
}

static inline float ZFW_Mag(const zfw_s_vec_2d vec) {
    return sqrtf(vec.x * vec.x + vec.y * vec.y);
}

static inline float ZFW_Dot(const zfw_s_vec_2d a, const zfw_s_vec_2d b) {
    return (a.x * b.x) + (a.y * b.y);
}

static inline zfw_s_vec_2d ZFW_NormalOrZero(const zfw_s_vec_2d vec) {
    const float mag = ZFW_Mag(vec);

    if (mag == 0.0f) {
        return (zfw_s_vec_2d){0};
    }

    return (zfw_s_vec_2d){vec.x / mag, vec.y / mag};
}

static inline zfw_s_vec_2d ZFW_Vec2DDir(const zfw_s_vec_2d a, const zfw_s_vec_2d b) {
    return ZFW_NormalOrZero((zfw_s_vec_2d){b.x - a.x, b.y - a.y});
}

static inline float ZFW_Dist(const zfw_s_vec_2d a, const zfw_s_vec_2d b) {
    zfw_s_vec_2d d = {a.x - b.x, a.y - b.y};
    return ZFW_Mag(d);
}

static inline float ZFW_Dir(const zfw_s_vec_2d vec) {
    return atan2f(-vec.y, vec.x);
}

static inline float ZFW_DirAToB(const zfw_s_vec_2d a, const zfw_s_vec_2d b) {
    return ZFW_Dir((zfw_s_vec_2d){b.x - a.x, b.y - a.y});
}

static inline zfw_s_vec_2d ZFW_LenDir(const float len, const float dir) {
    return (zfw_s_vec_2d){cosf(dir) * len, -sinf(dir) * len};
}

static inline zfw_s_vec_2d ZFW_RectPos(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.x, rect.y};
}

static inline zfw_s_vec_2d_s32 ZFW_RectS32Pos(const zfw_s_rect_s32 rect) {
    return (zfw_s_vec_2d_s32){rect.x, rect.y};
}

static inline zfw_s_vec_2d ZFW_RectSize(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.width, rect.height};
}

static inline zfw_s_vec_2d_s32 ZFW_RectS32Size(const zfw_s_rect_s32 rect) {
    return (zfw_s_vec_2d_s32){rect.width, rect.height};
}

static inline zfw_s_vec_2d ZFW_RectCenter(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.x + (rect.width / 2.0f), rect.y + (rect.height / 2.0f)};
}

static inline zfw_s_rect ZFW_RectTranslated(const zfw_s_rect rect, const zfw_s_vec_2d trans) {
    return (zfw_s_rect){rect.x + trans.x, rect.y + trans.y, rect.width, rect.height};
}

static inline zfw_s_rect_s32 ZFW_RectTranslatedS32(const zfw_s_rect_s32 rect, const zfw_s_vec_2d_s32 trans) {
    return (zfw_s_rect_s32){rect.x + trans.x, rect.y + trans.y, rect.width, rect.height};
}

static inline bool ZFW_IsPointInRect(const zfw_s_vec_2d pt, const zfw_s_rect rect) {
    return pt.x >= rect.x && pt.y >= rect.y && pt.x < rect.x + rect.width && pt.y < rect.y + rect.height;
}

static inline bool ZFW_DoRectsInters(const zfw_s_rect a, const zfw_s_rect b) {
    return a.x < b.x + b.width && a.y < b.y + b.height && a.x + a.width > b.x && a.y + a.height > b.y;
}

static inline zfw_s_rect_edges ZFW_RectEdgesClamped(const zfw_s_rect_edges edges, const zfw_s_rect_edges clamp_edges) {
    return (zfw_s_rect_edges){
        MAX(edges.left, clamp_edges.left),
        MAX(edges.top, clamp_edges.top),
        MIN(edges.right, clamp_edges.right),
        MIN(edges.bottom, clamp_edges.bottom)
    };
}

static inline zfw_s_rect_edges_s32 ZFW_RectEdgesS32Clamped(const zfw_s_rect_edges_s32 edges, const zfw_s_rect_edges_s32 clamp_edges) {
    return (zfw_s_rect_edges_s32){
        MAX(edges.left, clamp_edges.left),
        MAX(edges.top, clamp_edges.top),
        MIN(edges.right, clamp_edges.right),
        MIN(edges.bottom, clamp_edges.bottom)
    };
}

static inline bool ZFW_IsRangeValid(const zfw_s_rect_edges range, const zfw_s_vec_2d size) {
    assert(size.x > 0.0f && size.y > 0.0f);

    return range.left >= 0.0f && range.left < size.x
        && range.right >= 0.0f && range.right <= size.x
        && range.top >= 0.0f && range.top < size.y
        && range.bottom >= 0.0f && range.bottom <= size.y
        && range.left <= range.right
        && range.top <= range.bottom;
}

static inline bool ZFW_IsRangeS32Valid(const zfw_s_rect_edges_s32 range, const zfw_s_vec_2d_s32 size) {
    assert(size.x > 0 && size.y > 0);

    return range.left >= 0 && range.left < size.x
        && range.right >= 0 && range.right <= size.x
        && range.top >= 0 && range.top < size.y
        && range.bottom >= 0 && range.bottom <= size.y
        && range.left <= range.right
        && range.top <= range.bottom;
}

static inline zfw_s_matrix_4x4 ZFW_IdentityMatrix4x4() {
    zfw_s_matrix_4x4 mat = {0};
    mat.elems[0][0] = 1.0f;
    mat.elems[1][1] = 1.0f;
    mat.elems[2][2] = 1.0f;
    mat.elems[3][3] = 1.0f;

    return mat;
}

static inline zfw_s_matrix_4x4 ZFW_OrthographicMatrix(const float left, const float right, const float bottom, const float top, const float near, const float far) {
    assert(right > left);
    assert(top < bottom);
    assert(far > near);
    assert(near < far);

    zfw_s_matrix_4x4 mat = {0};
    mat.elems[0][0] = 2.0f / (right - left);
    mat.elems[1][1] = 2.0f / (top - bottom);
    mat.elems[2][2] = -2.0f / (far - near);
    mat.elems[3][0] = -(right + left) / (right - left);
    mat.elems[3][1] = -(top + bottom) / (top - bottom);
    mat.elems[3][2] = -(far + near) / (far - near);
    mat.elems[3][3] = 1.0f;

    return mat;
}

static inline void ZFW_TranslateMatrix4x4(zfw_s_matrix_4x4* const mat, const zfw_s_vec_2d trans) {
    assert(mat);

    mat->elems[3][0] += trans.x;
    mat->elems[3][1] += trans.y;
}

static inline void ZFW_ScaleMatrix4x4(zfw_s_matrix_4x4* const mat, const float scalar) {
    assert(mat);

    mat->elems[0][0] *= scalar;
    mat->elems[1][1] *= scalar;
    mat->elems[2][2] *= scalar;
}

static inline bool ZFW_IsPolyValid(const zfw_s_poly poly) {
    return poly.pts && poly.cnt > 0;
}

#endif
