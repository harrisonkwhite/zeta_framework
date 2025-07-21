#ifndef ZFW_MATH_H
#define ZFW_MATH_H

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include "zfw_utils.h"
#include "zfw_random.h"

#define ZFW_PI 3.14159265358979323846f

#define ZFW_MIN(x, y) ((x) <= (y) ? (x) : (y))
#define ZFW_MAX(x, y) ((x) >= (y) ? (x) : (y))

#define ZFW_ABS(x) (x) < 0 ? -(x) : (x)

#define ZFW_CLAMP(x, min, max) (((x) < (min)) ? (min) : ((x) > (max) ? (max) : (x)))
#define ZFW_SIGN(x) ((x) == 0 ? (x) : ((x) > 0 ? 1 : -1));

typedef enum {
    zfw_ek_cardinal_dir_right,
    zfw_ek_cardinal_dir_left,
    zfw_ek_cardinal_dir_down,
    zfw_ek_cardinal_dir_up
} zfw_e_cardinal_dir;

typedef float zfw_t_matrix_4x4[4][4];

typedef struct {
    float x;
    float y;
} zfw_s_vec_2d;

typedef struct {
    int x;
    int y;
} zfw_s_vec_2d_i;

typedef struct {
    float x;
    float y;
    float z;
} zfw_s_vec_3d;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} zfw_s_vec_4d;

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
} zfw_s_rect_i;

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
} zfw_s_rect_edges_i;

typedef struct {
    float min;
    float max;
} zfw_s_range_f;

typedef struct {
    zfw_s_vec_2d* pts;
    int cnt;
} zfw_s_poly;

extern const zfw_s_vec_2d zfw_g_cardinal_dir_vecs[];

zfw_s_rect ZFWGenSpanningRect(const zfw_s_rect* const rects, const int cnt);

void ZFWInitIdenMatrix4x4(zfw_t_matrix_4x4* const mat);
void ZFWTranslateMatrix4x4(zfw_t_matrix_4x4* const mat, const zfw_s_vec_2d trans);
void ZFWScaleMatrix4x4(zfw_t_matrix_4x4* const mat, const float scalar);
void ZFWInitOrthoMatrix4x4(zfw_t_matrix_4x4* const mat, const float left, const float right, const float bottom, const float top, const float near, const float far);

bool ZFWPushQuadPoly(zfw_s_poly* const poly, zfw_s_mem_arena* const mem_arena, const zfw_s_vec_2d pos, const zfw_s_vec_2d size, const zfw_s_vec_2d origin);
bool ZFWPushQuadPolyRotated(zfw_s_poly* const poly, zfw_s_mem_arena* const mem_arena, const zfw_s_vec_2d pos, const zfw_s_vec_2d size, const zfw_s_vec_2d origin, const float rot);
bool ZFWDoPolysInters(const zfw_s_poly a, const zfw_s_poly b);
bool ZFWDoesPolyIntersWithRect(const zfw_s_poly poly, const zfw_s_rect rect);
zfw_s_rect_edges PolySpan(const zfw_s_poly poly);

static inline int ZFWIndexFrom2D(const zfw_s_vec_2d_i pos, const int width) {
    assert(pos.x >= 0 && pos.x < width && pos.y >= 0);
    return (width * pos.y) + pos.x;
}

static inline float ZFWLerp(const float a, const float b, const float t) {
    assert(t >= 0.0f && t <= 1.0f);
    return a + ((b - a) * t);
}

static inline zfw_s_vec_2d ZFWLerpVec2D(const zfw_s_vec_2d a, const zfw_s_vec_2d b, const float t) {
    assert(t >= 0.0f && t <= 1.0f);
    return (zfw_s_vec_2d){ZFWLerp(a.x, b.x, t), ZFWLerp(a.y, b.y, t)};
}

static inline zfw_s_vec_2d ZFWVec2DSum(const zfw_s_vec_2d a, const zfw_s_vec_2d b) {
    return (zfw_s_vec_2d){a.x + b.x, a.y + b.y};
}

static inline zfw_s_vec_2d_i ZFWVec2DISum(const zfw_s_vec_2d_i a, const zfw_s_vec_2d_i b) {
    return (zfw_s_vec_2d_i){a.x + b.x, a.y + b.y};
}

static inline zfw_s_vec_2d ZFWVec2DDiff(const zfw_s_vec_2d a, const zfw_s_vec_2d b) {
    return (zfw_s_vec_2d){a.x - b.x, a.y - b.y};
}

static inline zfw_s_vec_2d_i ZFWVec2DIDiff(const zfw_s_vec_2d_i a, const zfw_s_vec_2d_i b) {
    return (zfw_s_vec_2d_i){a.x - b.x, a.y - b.y};
}

static inline zfw_s_vec_2d ZFWVec2DScaled(const zfw_s_vec_2d vec, const float scalar) {
    return (zfw_s_vec_2d){vec.x * scalar, vec.y * scalar};
}

static inline zfw_s_vec_2d_i ZFWVec2DIScaled(const zfw_s_vec_2d_i vec, const float scalar) {
    return (zfw_s_vec_2d_i){vec.x * scalar, vec.y * scalar};
}

static inline bool ZFWVec2DsEqual(const zfw_s_vec_2d a, const zfw_s_vec_2d b) {
    return a.x == b.x && a.y == b.y;
}

static inline bool ZFWVec2DIsEqual(const zfw_s_vec_2d_i a, const zfw_s_vec_2d_i b) {
    return a.x == b.x && a.y == b.y;
}

static inline float ZFWMag(const zfw_s_vec_2d vec) {
    return sqrtf(vec.x * vec.x + vec.y * vec.y);
}

static inline float ZFWDot(const zfw_s_vec_2d a, const zfw_s_vec_2d b) {
    return a.x * b.x + a.y * b.y;
}

static inline zfw_s_vec_2d ZFWNormalOrZero(const zfw_s_vec_2d vec) {
    const float mag = ZFWMag(vec);

    if (mag == 0.0f) {
        return (zfw_s_vec_2d){0};
    }

    return (zfw_s_vec_2d){vec.x / mag, vec.y / mag};
}

static inline zfw_s_vec_2d ZFWVec2DDir(const zfw_s_vec_2d a, const zfw_s_vec_2d b) {
    return ZFWNormalOrZero(ZFWVec2DDiff(b, a));
}

static inline float ZFWDist(const zfw_s_vec_2d a, const zfw_s_vec_2d b) {
    zfw_s_vec_2d d = {a.x - b.x, a.y - b.y};
    return ZFWMag(d);
}

static inline float ZFWDir(const zfw_s_vec_2d vec) {
    return atan2f(-vec.y, vec.x);
}

static inline float ZFWDirFrom(const zfw_s_vec_2d src, const zfw_s_vec_2d dest) {
    return ZFWDir(ZFWVec2DDiff(dest, src));
}

static inline zfw_s_vec_2d ZFWLenDir(float len, float dir) {
    return (zfw_s_vec_2d){cosf(dir)* len, -sinf(dir) * len};
}

static inline zfw_s_vec_2d ZFWRectPos(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.x, rect.y};
}

static inline zfw_s_vec_2d_i ZFWRectIPos(const zfw_s_rect_i rect) {
    return (zfw_s_vec_2d_i){rect.x, rect.y};
}

static inline zfw_s_vec_2d ZFWRectSize(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.width, rect.height};
}

static inline zfw_s_vec_2d_i ZFWRectISize(const zfw_s_rect_i rect) {
    return (zfw_s_vec_2d_i){rect.width, rect.height};
}

static inline float ZFWRectRight(const zfw_s_rect rect) {
    return rect.x + rect.width;
}

static inline int ZFWRectIRight(const zfw_s_rect_i rect) {
    return rect.x + rect.width;
}

static inline float ZFWRectBottom(const zfw_s_rect rect) {
    return rect.y + rect.height;
}

static inline int ZFWRectIBottom(const zfw_s_rect_i rect) {
    return rect.y + rect.height;
}

static inline zfw_s_vec_2d ZFWRectTopCenter(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.x + rect.width * 0.5f, rect.y};
}

static inline zfw_s_vec_2d_i ZFWRectITopCenter(const zfw_s_rect_i rect) {
    return (zfw_s_vec_2d_i){rect.x + rect.width / 2, rect.y};
}

static inline zfw_s_vec_2d ZFWRectTopRight(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.x + rect.width, rect.y};
}

static inline zfw_s_vec_2d_i ZFWRectITopRight(const zfw_s_rect_i rect) {
    return (zfw_s_vec_2d_i){rect.x + rect.width, rect.y};
}

static inline zfw_s_vec_2d ZFWRectCenterLeft(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.x, rect.y + rect.height * 0.5f};
}

static inline zfw_s_vec_2d_i ZFWRectICenterLeft(const zfw_s_rect_i rect) {
    return (zfw_s_vec_2d_i){rect.x, rect.y + rect.height / 2};
}

static inline zfw_s_vec_2d ZFWRectCenter(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f};
}

static inline zfw_s_vec_2d_i ZFWRectICenter(const zfw_s_rect_i rect) {
    return (zfw_s_vec_2d_i){rect.x + rect.width / 2, rect.y + rect.height / 2};
}

static inline zfw_s_vec_2d ZFWRectCenterRight(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.x + rect.width, rect.y + rect.height * 0.5f};
}

static inline zfw_s_vec_2d_i ZFWRectICenterRight(const zfw_s_rect_i rect) {
    return (zfw_s_vec_2d_i){rect.x + rect.width, rect.y + rect.height / 2};
}

static inline zfw_s_vec_2d ZFWRectBottomLeft(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.x, rect.y + rect.height};
}

static inline zfw_s_vec_2d_i ZFWRectIBottomLeft(const zfw_s_rect_i rect) {
    return (zfw_s_vec_2d_i){rect.x, rect.y + rect.height};
}

static inline zfw_s_vec_2d ZFWRectBottomCenter(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.x + rect.width * 0.5f, rect.y + rect.height};
}

static inline zfw_s_vec_2d_i ZFWRectIBottomCenter(const zfw_s_rect_i rect) {
    return (zfw_s_vec_2d_i){rect.x + rect.width / 2, rect.y + rect.height};
}

static inline zfw_s_vec_2d ZFWRectBottomRight(const zfw_s_rect rect) {
    return (zfw_s_vec_2d){rect.x + rect.width, rect.y + rect.height};
}

static inline zfw_s_vec_2d_i ZFWRectIBottomRight(const zfw_s_rect_i rect) {
    return (zfw_s_vec_2d_i){rect.x + rect.width, rect.y + rect.height};
}

static inline zfw_s_rect ZFWRectTranslated(const zfw_s_rect rect, const zfw_s_vec_2d trans) {
    return (zfw_s_rect){rect.x + trans.x, rect.y + trans.y, rect.width, rect.height};
}

static inline bool ZFWIsRangeValid(const zfw_s_rect_edges_i range, const zfw_s_vec_2d_i size) {
    assert(size.x > 0 && size.y > 0);

    return range.left >= 0 && range.left < size.x
        && range.right >= 0 && range.right <= size.x
        && range.top >= 0 && range.top < size.y
        && range.bottom >= 0 && range.bottom <= size.y
        && range.left <= range.right
        && range.top <= range.bottom;
}

static inline zfw_s_rect_edges ZFWRectEdgesClamped(const zfw_s_rect_edges edges, const zfw_s_rect_edges clamp_edges) {
    return (zfw_s_rect_edges){
        ZFW_MAX(edges.left, clamp_edges.left),
        ZFW_MAX(edges.top, clamp_edges.top),
        ZFW_MIN(edges.right, clamp_edges.right),
        ZFW_MIN(edges.bottom, clamp_edges.bottom)
    };
}

static inline zfw_s_rect_edges_i ZFWRectEdgesIClamped(const zfw_s_rect_edges_i edges, const zfw_s_rect_edges_i clamp_edges) {
    return (zfw_s_rect_edges_i){
        ZFW_MAX(edges.left, clamp_edges.left),
        ZFW_MAX(edges.top, clamp_edges.top),
        ZFW_MIN(edges.right, clamp_edges.right),
        ZFW_MIN(edges.bottom, clamp_edges.bottom)
    };
}

static inline zfw_s_rect_i ZFWRectIClamped(const zfw_s_rect_i rect, const zfw_s_vec_2d_i span) {
    assert(span.x >= 0 && span.y >= 0);

    const zfw_s_vec_2d_i rect_clamped_tl = {ZFW_MAX(rect.x, 0), ZFW_MAX(rect.y, 0)};

    const zfw_s_vec_2d_i rect_clamped_br = {
        ZFW_MIN(rect_clamped_tl.x + rect.width, span.x),
        ZFW_MIN(rect_clamped_tl.y + rect.height, span.y)
    };

    return (zfw_s_rect_i){
        rect_clamped_tl.x,
        rect_clamped_tl.y,
        rect_clamped_br.x - rect_clamped_tl.x,
        rect_clamped_br.y - rect_clamped_tl.y
    };
}

static inline bool ZFWIsPointInRect(const zfw_s_vec_2d pt, const zfw_s_rect rect) {
    return pt.x >= rect.x && pt.y >= rect.y && pt.x < ZFWRectRight(rect) && pt.y < ZFWRectBottom(rect);
}

static inline bool ZFWDoRectsInters(const zfw_s_rect a, const zfw_s_rect b) {
    return a.x < b.x + b.width && a.y < b.y + b.height && a.x + a.width > b.x && a.y + a.height > b.y;
}

static inline bool ZFWIsPolySet(const zfw_s_poly poly) {
    return poly.pts && poly.cnt > 0;
}

static inline float ZFWRandRot() {
    return ZFWRandPerc() * 2.0f * ZFW_PI;
}

#endif
