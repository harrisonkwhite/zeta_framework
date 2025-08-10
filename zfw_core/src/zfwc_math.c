#include "zfwc_math.h"

#include <float.h>

typedef struct {
    t_r32 min;
    t_r32 max;
} s_range;

static s_range ProjectPts(const s_v2_array_view pts, const s_v2 edge) {
    s_range range = {
        .min = FLT_MAX,
        .max = -FLT_MAX
    };

    for (int i = 0; i < pts.len; ++i) {
        const s_v2 pt = *V2ElemView(pts, i);
        const t_r32 dot = Dot(pt, edge);

        if (dot < range.min) {
            range.min = dot;
        }
        
        if (dot > range.max) {
            range.max = dot;
        }
    }

    return range;
}

static bool CheckPolySep(const s_poly poly, const s_poly other) {
    for (int i = 0; i < poly.pts.len; ++i) {
        const s_v2 a = *V2ElemView(poly.pts, i);
        const s_v2 b = *V2ElemView(poly.pts, (i + 1) % poly.pts.len);

        const s_v2 normal = { b.y - a.y, -(b.x - a.x) };

        const s_range a_range = ProjectPts(poly.pts, normal);
        const s_range b_range = ProjectPts(other.pts, normal);

        if (a_range.max <= b_range.min || b_range.max <= a_range.min) {
            return false;
        }
    }
    
    return true;
}

s_poly GenQuadPoly(s_mem_arena* const mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin) {
    assert(origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f);

    const s_v2_array pts = PushV2ArrayToMemArena(mem_arena, 4);

    if (IS_ZERO(pts)) {
        LOG_ERROR("Failed to reserve memory for quad polygon points!");
        return (s_poly){0};
    }

    const s_v2 pos_base = {pos.x - (size.x * origin.x), pos.y - (size.y * origin.y)};

    *V2Elem(pts, 0) = pos_base;
    *V2Elem(pts, 1) = (s_v2){pos_base.x + size.x, pos_base.y};
    *V2Elem(pts, 2) = (s_v2){pos_base.x + size.x, pos_base.y + size.y};
    *V2Elem(pts, 3) = (s_v2){pos_base.x, pos_base.y + size.y};

    return (s_poly){
        .pts = V2ArrayView(pts)
    };
}

s_poly GenQuadPolyRotated(s_mem_arena* const mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin, const t_r32 rot) {
    assert(origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f);

    const s_v2_array pts = PushV2ArrayToMemArena(mem_arena, 4);

    if (IS_ZERO(pts)) {
        LOG_ERROR("Failed to reserve memory for rotated quad polygon points!");
        return (s_poly){0};
    }

    const s_v2 offs_left  = LenDir(size.x * origin.x, rot + PI);
    const s_v2 offs_up = LenDir(size.y * origin.y, rot + (PI * 0.5f));
    const s_v2 offs_right = LenDir(size.x * (1.0f - origin.x), rot);
    const s_v2 offs_down = LenDir(size.y * (1.0f - origin.y), rot - (PI * 0.5f));

    *V2Elem(pts, 0) = (s_v2){pos.x + offs_left.x + offs_up.x, pos.y + offs_left.y + offs_up.y};
    *V2Elem(pts, 1) = (s_v2){pos.x + offs_right.x + offs_up.x, pos.y + offs_right.y + offs_up.y};
    *V2Elem(pts, 2) = (s_v2){pos.x + offs_right.x + offs_down.x, pos.y + offs_right.y + offs_down.y};
    *V2Elem(pts, 3) = (s_v2){pos.x + offs_left.x + offs_down.x, pos.y + offs_left.y + offs_down.y};

    return (s_poly){
        .pts = V2ArrayView(pts)
    };
}

bool DoPolysInters(const s_poly a, const s_poly b) {
    return CheckPolySep(a, b) && CheckPolySep(b, a);
}

bool DoesPolyIntersWithRect(const s_poly poly, const s_rect rect) {
    const s_v2 pts[4] = {
        {rect.x, rect.y},
        {rect.x + rect.width, rect.y},
        {rect.x + rect.width, rect.y + rect.height},
        {rect.x, rect.y + rect.height}
    };

    const s_poly rect_poly = {
        .pts = ARRAY_FROM_STATIC(s_v2_array_view, pts)
    };

    return DoPolysInters(poly, rect_poly);
}

s_rect_edges PolySpan(const s_poly poly) {
    s_rect_edges span = {
        .left = FLT_MAX,
        .top = FLT_MAX,
        .right = FLT_MIN,
        .bottom = FLT_MIN
    };

    for (int i = 0; i < poly.pts.len; i++) {
        const s_v2 pt = *V2ElemView(poly.pts, i);

        span.left = MIN(pt.x, span.left);
        span.right = MAX(pt.x, span.right);
        span.top = MIN(pt.y, span.top);
        span.bottom = MAX(pt.y, span.bottom);
    }

    return span;
}
