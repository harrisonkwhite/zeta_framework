#include "zfw_math.h"

#include <float.h>

const zfw_s_vec_2d zfw_g_cardinal_dir_vecs[] = {
    [zfw_ek_cardinal_dir_right] = {1.0f, 0.0f},
    [zfw_ek_cardinal_dir_left] = {-1.0f, 0.0f},
    [zfw_ek_cardinal_dir_down] = {0.0f, 1.0f},
    [zfw_ek_cardinal_dir_up] = {0.0f, -1.0f}
};

zfw_s_rect ZFW_GenSpanningRect(const zfw_s_rect* const rects, const int cnt) {
    assert(cnt > 0);

    zfw_s_rect_edges span = {
        rects[0].x,
        rects[0].y,
        rects[0].x + rects[0].width,
        rects[0].y + rects[0].height
    };

    for (int i = 1; i < cnt; ++i) {
        const zfw_s_rect* const r = &rects[i];

        if (r->x < span.left) {
            span.left = r->x;
        }
        
        if (r->y < span.top) {
            span.top = r->y;
        }
        
        if (r->x + r->width > span.right) {
            span.right = r->x + r->width;
        }
        
        if (r->y + r->height > span.bottom) {
            span.bottom = r->y + r->height;
        }
    }

    return (zfw_s_rect){span.left, span.top, span.right - span.left, span.bottom - span.top};
}

void ZFW_InitIdenMatrix4x4(zfw_t_matrix_4x4* const mat) {
    assert(ZFW_IS_ZERO(*mat));

    (*mat)[0][0] = 1.0f;
    (*mat)[1][1] = 1.0f;
    (*mat)[2][2] = 1.0f;
    (*mat)[3][3] = 1.0f;
}

void ZFW_TranslateMatrix4x4(zfw_t_matrix_4x4* const mat, const zfw_s_vec_2d trans) {
    (*mat)[3][0] += trans.x;
    (*mat)[3][1] += trans.y;
}

void ZFW_ScaleMatrix4x4(zfw_t_matrix_4x4* const mat, const float scalar) {
    (*mat)[0][0] *= scalar;
    (*mat)[1][1] *= scalar;
    (*mat)[2][2] *= scalar;
}

void ZFW_InitOrthoMatrix4x4(zfw_t_matrix_4x4* const mat, const float left, const float right, const float bottom, const float top, const float near, const float far) {
    assert(ZFW_IS_ZERO(*mat));
    assert(right > left);
    assert(top < bottom);
    assert(far > near);
    assert(near < far);

    (*mat)[0][0] = 2.0f / (right - left);
    (*mat)[1][1] = 2.0f / (top - bottom);
    (*mat)[2][2] = -2.0f / (far - near);
    (*mat)[3][0] = -(right + left) / (right - left);
    (*mat)[3][1] = -(top + bottom) / (top - bottom);
    (*mat)[3][2] = -(far + near) / (far - near);
    (*mat)[3][3] = 1.0f;
}

static zfw_s_range_f ProjectPts(const zfw_s_vec_2d* const pts, const int cnt, const zfw_s_vec_2d edge) {
    zfw_s_range_f range = {
        .min = FLT_MAX,
        .max = -FLT_MAX
    };
    
    for (int i = 0; i < cnt; ++i) {
        const float dot = ZFW_Dot(pts[i], edge);
        
        if (dot < range.min) {
            range.min = dot;
        }
        
        if (dot > range.max) {
            range.max = dot;
        }
    }
    
    return range;
}

static bool CheckPolySep(const zfw_s_poly poly, const zfw_s_poly other) {
    assert(ZFW_IsPolySet(poly));
    assert(ZFW_IsPolySet(other));

    for (int i = 0; i < poly.cnt; ++i) {
        const zfw_s_vec_2d a = poly.pts[i];
        const zfw_s_vec_2d b = poly.pts[(i + 1) % poly.cnt];

        zfw_s_vec_2d normal = { b.y - a.y, -(b.x - a.x) };

        const zfw_s_range_f a_range = ProjectPts(poly.pts, poly.cnt, normal);
        const zfw_s_range_f b_range = ProjectPts(other.pts, other.cnt, normal);

        if (a_range.max <= b_range.min || b_range.max <= a_range.min) {
            return false;
        }
    }
    
    return true;
}

bool ZFW_PushQuadPoly(zfw_s_poly* const poly, zfw_s_mem_arena* const mem_arena, const zfw_s_vec_2d pos, const zfw_s_vec_2d size, const zfw_s_vec_2d origin) {
    assert(ZFW_IS_ZERO(*poly));
    assert(size.x > 0.0f && size.y > 0.0f);
    assert(origin.x >= 0.0f && origin.y >= 0.0f && origin.x <= 1.0f && origin.y <= 1.0f);
    
    poly->pts = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_s_vec_2d, 4);

    if (!poly->pts) {
        return false;
    }

    poly->cnt = 4;

    const zfw_s_vec_2d pos_base = {pos.x - size.x * origin.x, pos.y - size.y * origin.y};
    poly->pts[0] = pos_base;
    poly->pts[1] = (zfw_s_vec_2d){pos_base.x + size.x, pos_base.y};
    poly->pts[2] = (zfw_s_vec_2d){pos_base.x + size.x, pos_base.y + size.y};
    poly->pts[3] = (zfw_s_vec_2d){pos_base.x, pos_base.y + size.y};

    return true;
}

bool ZFW_PushQuadPolyRotated(zfw_s_poly* const poly, zfw_s_mem_arena* const mem_arena, const zfw_s_vec_2d pos, const zfw_s_vec_2d size, const zfw_s_vec_2d origin, const float rot) {
    assert(ZFW_IS_ZERO(*poly));
    assert(size.x > 0.0f && size.y > 0.0f);
    assert(origin.x >= 0.0f && origin.y >= 0.0f && origin.x <= 1.0f && origin.y <= 1.0f);
    
    poly->pts = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_s_vec_2d, 4);

    if (!poly->pts) {
        return false;
    }

    poly->cnt = 4;

    const zfw_s_vec_2d left_offs  = ZFW_LenDir(size.x * origin.x, rot + ZFW_PI);
    const zfw_s_vec_2d up_offs = ZFW_LenDir(size.y * origin.y, rot + ZFW_PI * 0.5f);
    const zfw_s_vec_2d right_offs = ZFW_LenDir(size.x * (1.0f - origin.x), rot);
    const zfw_s_vec_2d down_offs = ZFW_LenDir(size.y * (1.0f - origin.y), rot - ZFW_PI * 0.5f);

    poly->pts[0] = (zfw_s_vec_2d){pos.x + left_offs.x + up_offs.x, pos.y + left_offs.y + up_offs.y};
    poly->pts[1] = (zfw_s_vec_2d){pos.x + right_offs.x + up_offs.x, pos.y + right_offs.y + up_offs.y};
    poly->pts[2] = (zfw_s_vec_2d){pos.x + right_offs.x + down_offs.x, pos.y + right_offs.y + down_offs.y};
    poly->pts[3] = (zfw_s_vec_2d){pos.x + left_offs.x + down_offs.x, pos.y + left_offs.y + down_offs.y};

    return true;
}

bool ZFW_DoPolysInters(const zfw_s_poly a, const zfw_s_poly b) {
    assert(ZFW_IsPolySet(a));
    assert(ZFW_IsPolySet(b));

    return CheckPolySep(a, b) && CheckPolySep(b, a);
}

bool ZFW_DoesPolyIntersWithRect(const zfw_s_poly poly, const zfw_s_rect rect) {
    const zfw_s_vec_2d pts[4] = {
        {rect.x, rect.y},
        {rect.x + rect.width, rect.y},
        {rect.x + rect.width, rect.y + rect.height},
        {rect.x, rect.y + rect.height}
    };

    const zfw_s_poly rect_poly = {
        .pts = (zfw_s_vec_2d*)pts,
        .cnt = 4
    };

    return ZFW_DoPolysInters(poly, rect_poly);
}

zfw_s_rect_edges PolySpan(const zfw_s_poly poly) {
    assert(ZFW_IsPolySet(poly));

    zfw_s_rect_edges span = {
        .left = FLT_MAX,
        .top = FLT_MAX,
        .right = FLT_MIN,
        .bottom = FLT_MIN
    };

    for (int i = 0; i < poly.cnt; i++) {
        const zfw_s_vec_2d pt = poly.pts[i];

        span.left = ZFW_MIN(pt.x, span.left);
        span.right = ZFW_MAX(pt.x, span.right);
        span.top = ZFW_MIN(pt.y, span.top);
        span.bottom = ZFW_MAX(pt.y, span.bottom);
    }

    return span;
}
