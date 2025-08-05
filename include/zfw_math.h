#ifndef ZFW_MATH_H
#define ZFW_MATH_H

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <cu.h>

typedef enum {
    zfw_ek_cardinal_dir_right,
    zfw_ek_cardinal_dir_left,
    zfw_ek_cardinal_dir_down,
    zfw_ek_cardinal_dir_up
} zfw_e_cardinal_dir;

static const s_v2_int zfw_g_cardinal_dirs[] = {
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
    int x;
    int y;
    int width;
    int height;
} zfw_s_rect_int;

typedef struct {
    float left;
    float top;
    float right;
    float bottom;
} zfw_s_rect_edges;

typedef struct {
    int left;
    int top;
    int right;
    int bottom;
} zfw_s_rect_edges_int;

typedef struct {
    float elems[4][4];
} zfw_s_matrix_4x4;

typedef struct {
    const s_v2* pts;
    int cnt;
} zfw_s_poly;

zfw_s_rect ZFW_GenSpanningRect(const zfw_s_rect* const rects, const int cnt);

zfw_s_poly ZFW_PushQuadPoly(s_mem_arena* const mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin);
zfw_s_poly ZFW_PushQuadPolyRotated(s_mem_arena* const mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin, const float rot);
bool ZFW_DoPolysInters(const zfw_s_poly a, const zfw_s_poly b);
bool ZFW_DoesPolyIntersWithRect(const zfw_s_poly poly, const zfw_s_rect rect);
zfw_s_rect_edges ZFW_PolySpan(const zfw_s_poly poly);

static inline s_v2 ZFW_RectPos(const zfw_s_rect rect) {
    return (s_v2){rect.x, rect.y};
}

static inline s_v2_int ZFW_RectIntPos(const zfw_s_rect_int rect) {
    return (s_v2_int){rect.x, rect.y};
}

static inline s_v2 ZFW_RectSize(const zfw_s_rect rect) {
    return (s_v2){rect.width, rect.height};
}

static inline s_v2_int ZFW_RectIntSize(const zfw_s_rect_int rect) {
    return (s_v2_int){rect.width, rect.height};
}

static inline s_v2 ZFW_RectCenter(const zfw_s_rect rect) {
    return (s_v2){rect.x + (rect.width / 2.0f), rect.y + (rect.height / 2.0f)};
}

static inline zfw_s_rect ZFW_RectTranslated(const zfw_s_rect rect, const s_v2 trans) {
    return (zfw_s_rect){rect.x + trans.x, rect.y + trans.y, rect.width, rect.height};
}

static inline zfw_s_rect_int ZFW_RectTranslatedInt(const zfw_s_rect_int rect, const s_v2_int trans) {
    return (zfw_s_rect_int){rect.x + trans.x, rect.y + trans.y, rect.width, rect.height};
}

static inline bool ZFW_IsPointInRect(const s_v2 pt, const zfw_s_rect rect) {
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

static inline zfw_s_rect_edges_int ZFW_RectEdgesIntClamped(const zfw_s_rect_edges_int edges, const zfw_s_rect_edges_int clamp_edges) {
    return (zfw_s_rect_edges_int){
        MAX(edges.left, clamp_edges.left),
        MAX(edges.top, clamp_edges.top),
        MIN(edges.right, clamp_edges.right),
        MIN(edges.bottom, clamp_edges.bottom)
    };
}

static inline bool ZFW_IsRangeValid(const zfw_s_rect_edges range, const s_v2 size) {
    assert(size.x > 0.0f && size.y > 0.0f);

    return range.left >= 0.0f && range.left < size.x
        && range.right >= 0.0f && range.right <= size.x
        && range.top >= 0.0f && range.top < size.y
        && range.bottom >= 0.0f && range.bottom <= size.y
        && range.left <= range.right
        && range.top <= range.bottom;
}

static inline bool ZFW_IsRangeIntValid(const zfw_s_rect_edges_int range, const s_v2_int size) {
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

static inline void ZFW_TranslateMatrix4x4(zfw_s_matrix_4x4* const mat, const s_v2 trans) {
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
