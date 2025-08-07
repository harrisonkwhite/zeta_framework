#include "zfw_math.h"

#include <float.h>

typedef struct {
    float min;
    float max;
} s_range;

static s_range ProjectPts(const s_v2_array pts, const s_v2 edge) {
    V2Array_AssertValidity(pts);
    assert(pts.len > 0);

    s_range range = {
        .min = FLT_MAX,
        .max = -FLT_MAX
    };

    for (int i = 0; i < pts.len; ++i) {
        const s_v2 pt = *V2Array_Get(pts, i);
        const float dot = Dot(pt, edge);
        
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
    Poly_AssertValidity(poly);
    Poly_AssertValidity(other);

    for (int i = 0; i < poly.pts.len; ++i) {
        const s_v2 a = *V2Array_Get(poly.pts, i);
        const s_v2 b = *V2Array_Get(poly.pts, (i + 1) % poly.pts.len);

        const s_v2 normal = { b.y - a.y, -(b.x - a.x) };

        const s_range a_range = ProjectPts(poly.pts, normal);
        const s_range b_range = ProjectPts(other.pts, normal);

        if (a_range.max <= b_range.min || b_range.max <= a_range.min) {
            return false;
        }
    }
    
    return true;
}

zfw_s_poly ZFW_PushQuadPoly(s_mem_arena* const mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(size.x > 0.0f && size.y > 0.0f);
    assert(origin.x >= 0.0f && origin.y >= 0.0f && origin.x <= 1.0f && origin.y <= 1.0f);

    const s_v2_array pts = PushV2s(mem_arena, 4);

    if (IS_ZERO(pts)) {
        LOG_ERROR("Failed to reserve memory for quad polygon points!");
        return (zfw_s_poly){0};
    }

    const s_v2 pos_base = {pos.x - (size.x * origin.x), pos.y - (size.y * origin.y)};

    *V2Array_Get(pts, 0) = pos_base;
    *V2Array_Get(pts, 1) = (s_v2){pos_base.x + size.x, pos_base.y};
    *V2Array_Get(pts, 2) = (s_v2){pos_base.x + size.x, pos_base.y + size.y};
    *V2Array_Get(pts, 3) = (s_v2){pos_base.x, pos_base.y + size.y};

    return (zfw_s_poly){
        .pts = pts
    };
}

zfw_s_poly ZFW_PushQuadPolyRotated(s_mem_arena* const mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin, const float rot) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(size.x > 0.0f && size.y > 0.0f);
    assert(origin.x >= 0.0f && origin.y >= 0.0f && origin.x <= 1.0f && origin.y <= 1.0f);

    const s_v2_array pts = PushV2s(mem_arena, 4);

    if (IS_ZERO(pts)) {
        LOG_ERROR("Failed to reserve memory for rotated quad polygon points!");
        return (zfw_s_poly){0};
    }

    const s_v2 offs_left  = LenDir(size.x * origin.x, rot + PI);
    const s_v2 offs_up = LenDir(size.y * origin.y, rot + (PI * 0.5f));
    const s_v2 offs_right = LenDir(size.x * (1.0f - origin.x), rot);
    const s_v2 offs_down = LenDir(size.y * (1.0f - origin.y), rot - (PI * 0.5f));

    *V2Array_Get(pts, 0) = (s_v2){pos.x + offs_left.x + offs_up.x, pos.y + offs_left.y + offs_up.y};
    *V2Array_Get(pts, 1) = (s_v2){pos.x + offs_right.x + offs_up.x, pos.y + offs_right.y + offs_up.y};
    *V2Array_Get(pts, 2) = (s_v2){pos.x + offs_right.x + offs_down.x, pos.y + offs_right.y + offs_down.y};
    *V2Array_Get(pts, 3) = (s_v2){pos.x + offs_left.x + offs_down.x, pos.y + offs_left.y + offs_down.y};

    return (zfw_s_poly){
        .pts = pts
    };
}

bool ZFW_DoPolysInters(const zfw_s_poly a, const zfw_s_poly b) {
    Poly_AssertValidity(a);
    Poly_AssertValidity(b);

    return CheckPolySep(a, b) && CheckPolySep(b, a);
}

bool ZFW_DoesPolyIntersWithRect(const zfw_s_poly poly, const s_rect rect) {
    Poly_AssertValidity(poly);
    assert(rect.width > 0.0f && rect.height > 0.0f);

    const s_v2 pts[4] = {
        {rect.x, rect.y},
        {rect.x + rect.width, rect.y},
        {rect.x + rect.width, rect.y + rect.height},
        {rect.x, rect.y + rect.height}
    };

    const zfw_s_poly rect_poly = {
        .pts = {
            .buf_raw = pts,
            .len = 4
        }
    };

    return ZFW_DoPolysInters(poly, rect_poly);
}

s_rect_edges ZFW_PolySpan(const zfw_s_poly poly) {
    Poly_AssertValidity(poly);

    s_rect_edges span = {
        .left = FLT_MAX,
        .top = FLT_MAX,
        .right = FLT_MIN,
        .bottom = FLT_MIN
    };

    for (int i = 0; i < poly.pts.len; i++) {
        const s_v2 pt = *V2Array_Get(poly.pts, i);

        span.left = MIN(pt.x, span.left);
        span.right = MAX(pt.x, span.right);
        span.top = MIN(pt.y, span.top);
        span.bottom = MAX(pt.y, span.bottom);
    }

    return span;
}
