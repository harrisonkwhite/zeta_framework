#ifndef ZFWC_MATH_H
#define ZFWC_MATH_H

#include <cu.h>

typedef enum {
    ek_cardinal_dir_right,
    ek_cardinal_dir_left,
    ek_cardinal_dir_down,
    ek_cardinal_dir_up
} e_cardinal_dir;

static const s_v2_s32 g_cardinal_dirs[] = {
    [ek_cardinal_dir_right] = {1, 0},
    [ek_cardinal_dir_left] = {-1, 0},
    [ek_cardinal_dir_down] = {0, 1},
    [ek_cardinal_dir_up] = {0, -1}
};

typedef struct {
    s_v2_array_view pts;
} s_poly;

s_poly GenQuadPoly(s_mem_arena* const mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin);
s_poly GenQuadPolyRotated(s_mem_arena* const mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin, const t_r32 rot);
bool DoPolysInters(const s_poly a, const s_poly b);
bool DoesPolyIntersWithRect(const s_poly poly, const s_rect rect);
s_rect_edges PolySpan(const s_poly poly);

#endif
