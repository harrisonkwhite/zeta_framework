#pragma once

#include <cu.h>

/*enum e_cardinal_dir {
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
};*/

struct s_matrix_4x4 {
    float elems[4][4];

    static s_matrix_4x4 Identity() {
        s_matrix_4x4 mat = {0};

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

        s_matrix_4x4 mat = {0};
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
