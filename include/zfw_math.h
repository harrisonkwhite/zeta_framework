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
    const s_v2* pts;
    int cnt;
} zfw_s_poly;

zfw_s_poly ZFW_PushQuadPoly(s_mem_arena* const mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin);
zfw_s_poly ZFW_PushQuadPolyRotated(s_mem_arena* const mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin, const float rot);
bool ZFW_DoPolysInters(const zfw_s_poly a, const zfw_s_poly b);
bool ZFW_DoesPolyIntersWithRect(const zfw_s_poly poly, const s_rect rect);
s_rect_edges ZFW_PolySpan(const zfw_s_poly poly);

static inline bool ZFW_IsPolyValid(const zfw_s_poly poly) {
    return poly.pts && poly.cnt > 0;
}

#endif
