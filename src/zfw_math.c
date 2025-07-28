#include "zfw_math.h"
#include "mem.h"

#include <float.h>

zfw_s_rect ZFW_GenSpanningRect(const zfw_s_rect* const rects, const int cnt) {
    assert(rects);
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

typedef struct {
    float min;
    float max;
} s_range;

static s_range ProjectPts(const zfw_s_vec_2d* const pts, const int cnt, const zfw_s_vec_2d edge) {
    assert(pts);
    assert(cnt > 0);

    s_range range = {
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
    assert(ZFW_IsPolyValid(poly));
    assert(ZFW_IsPolyValid(other));

    for (int i = 0; i < poly.cnt; ++i) {
        const zfw_s_vec_2d a = poly.pts[i];
        const zfw_s_vec_2d b = poly.pts[(i + 1) % poly.cnt];

        zfw_s_vec_2d normal = { b.y - a.y, -(b.x - a.x) };

        const s_range a_range = ProjectPts(poly.pts, poly.cnt, normal);
        const s_range b_range = ProjectPts(other.pts, other.cnt, normal);

        if (a_range.max <= b_range.min || b_range.max <= a_range.min) {
            return false;
        }
    }
    
    return true;
}

zfw_s_poly ZFW_PushQuadPoly(s_mem_arena* const mem_arena, const zfw_s_vec_2d pos, const zfw_s_vec_2d size, const zfw_s_vec_2d origin) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(size.x > 0.0f && size.y > 0.0f);
    assert(origin.x >= 0.0f && origin.y >= 0.0f && origin.x <= 1.0f && origin.y <= 1.0f);

    zfw_s_vec_2d* const pts = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_vec_2d, 4);

    if (!pts) {
        return (zfw_s_poly){0};
    }

    const zfw_s_vec_2d pos_base = {pos.x - (size.x * origin.x), pos.y - (size.y * origin.y)};

    pts[0] = pos_base;
    pts[1] = (zfw_s_vec_2d){pos_base.x + size.x, pos_base.y};
    pts[2] = (zfw_s_vec_2d){pos_base.x + size.x, pos_base.y + size.y};
    pts[3] = (zfw_s_vec_2d){pos_base.x, pos_base.y + size.y};

    return (zfw_s_poly){
        .pts = pts,
        .cnt = 4
    };
}

zfw_s_poly ZFW_PushQuadPolyRotated(s_mem_arena* const mem_arena, const zfw_s_vec_2d pos, const zfw_s_vec_2d size, const zfw_s_vec_2d origin, const float rot) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(size.x > 0.0f && size.y > 0.0f);
    assert(origin.x >= 0.0f && origin.y >= 0.0f && origin.x <= 1.0f && origin.y <= 1.0f);

    zfw_s_vec_2d* const pts = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_vec_2d, 4);

    if (!pts) {
        return (zfw_s_poly){0};
    }

    const zfw_s_vec_2d offs_left  = ZFW_LenDir(size.x * origin.x, rot + ZFW_PI);
    const zfw_s_vec_2d offs_up = ZFW_LenDir(size.y * origin.y, rot + (ZFW_PI * 0.5f));
    const zfw_s_vec_2d offs_right = ZFW_LenDir(size.x * (1.0f - origin.x), rot);
    const zfw_s_vec_2d offs_down = ZFW_LenDir(size.y * (1.0f - origin.y), rot - (ZFW_PI * 0.5f));

    pts[0] = (zfw_s_vec_2d){pos.x + offs_left.x + offs_up.x, pos.y + offs_left.y + offs_up.y};
    pts[1] = (zfw_s_vec_2d){pos.x + offs_right.x + offs_up.x, pos.y + offs_right.y + offs_up.y};
    pts[2] = (zfw_s_vec_2d){pos.x + offs_right.x + offs_down.x, pos.y + offs_right.y + offs_down.y};
    pts[3] = (zfw_s_vec_2d){pos.x + offs_left.x + offs_down.x, pos.y + offs_left.y + offs_down.y};

    return (zfw_s_poly){
        .pts = pts,
        .cnt = 4
    };
}

bool ZFW_DoPolysInters(const zfw_s_poly a, const zfw_s_poly b) {
    assert(ZFW_IsPolyValid(a));
    assert(ZFW_IsPolyValid(b));

    return CheckPolySep(a, b) && CheckPolySep(b, a);
}

bool ZFW_DoesPolyIntersWithRect(const zfw_s_poly poly, const zfw_s_rect rect) {
    assert(ZFW_IsPolyValid(poly));
    assert(rect.width > 0.0f && rect.height > 0.0f);

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

zfw_s_rect_edges ZFW_PolySpan(const zfw_s_poly poly) {
    assert(ZFW_IsPolyValid(poly));

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
