#pragma once

#include <cu.h>

enum e_cardinal_dir {
    ek_cardinal_dir_right,
    ek_cardinal_dir_left,
    ek_cardinal_dir_down,
    ek_cardinal_dir_up,

    ek_cardinal_dir_cnt
};

static constexpr c_static_array<s_v2, ek_cardinal_dir_cnt> g_cardinal_dirs = {
    {1, 0},
    {-1, 0},
    {0, 1},
    {0, -1}
};

struct s_poly {
    c_array<const s_v2> pts;
};

s_poly GenQuadPoly(c_mem_arena& mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin);
s_poly GenQuadPolyRotated(c_mem_arena& mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin, const float rot);

bool DoPolysInters(const s_poly a, const s_poly b);
bool DoesPolyIntersWithRect(const s_poly poly, const s_rect rect);
s_rect_edges PolySpan(const s_poly poly);
