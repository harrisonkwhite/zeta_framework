#include "zf_math.h"

#include <cfloat>

namespace zf {
    struct s_range {
        float min;
        float max;
    };

    static s_range ProjectPts(const c_array<const s_v2> pts, const s_v2 edge) {
        s_range range{
            .min = FLT_MAX,
            .max = -FLT_MAX
        };

        for (t_s32 i = 0; i < pts.Len(); ++i) {
            const float dot = pts[i].Dot(edge);

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
        for (t_s32 i = 0; i < poly.pts.Len(); ++i) {
            const s_v2 a = poly.pts[i];
            const s_v2 b = poly.pts[(i + 1) % poly.pts.Len()];

            const s_v2 normal = {b.y - a.y, -(b.x - a.x)};

            const s_range a_range = ProjectPts(poly.pts, normal);
            const s_range b_range = ProjectPts(other.pts, normal);

            if (a_range.max <= b_range.min || b_range.max <= a_range.min) {
                return false;
            }
        }

        return true;
    }

    s_poly GenQuadPoly(c_mem_arena& mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin) {
        assert(origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f);

        const auto pts = PushArrayToMemArena<s_v2>(mem_arena, 4);

        if (pts.IsEmpty()) {
            //LOG_ERROR("Failed to reserve memory for quad polygon points!");
            return {};
        }

        const s_v2 pos_base = {pos.x - (size.x * origin.x), pos.y - (size.y * origin.y)};

        pts[0] = pos_base;
        pts[1] = {pos_base.x + size.x, pos_base.y};
        pts[2] = {pos_base.x + size.x, pos_base.y + size.y};
        pts[3] = {pos_base.x, pos_base.y + size.y};

        return {.pts = pts.View()};
    }

    s_poly GenQuadPolyRotated(c_mem_arena& mem_arena, const s_v2 pos, const s_v2 size, const s_v2 origin, const float rot) {
        assert(origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f);

        const auto pts = PushArrayToMemArena<s_v2>(mem_arena, 4);

        if (pts.IsEmpty()) {
            //LOG_ERROR("Failed to reserve memory for rotated quad polygon points!");
            return {};
        }

        const s_v2 offs_left = LenDir(size.x * origin.x, rot + g_pi);
        const s_v2 offs_up = LenDir(size.y * origin.y, rot + (g_pi * 0.5f));
        const s_v2 offs_right = LenDir(size.x * (1.0f - origin.x), rot);
        const s_v2 offs_down = LenDir(size.y * (1.0f - origin.y), rot - (g_pi * 0.5f));

        pts[0] = {pos.x + offs_left.x + offs_up.x, pos.y + offs_left.y + offs_up.y};
        pts[1] = {pos.x + offs_right.x + offs_up.x, pos.y + offs_right.y + offs_up.y};
        pts[2] = {pos.x + offs_right.x + offs_down.x, pos.y + offs_right.y + offs_down.y};
        pts[3] = {pos.x + offs_left.x + offs_down.x, pos.y + offs_left.y + offs_down.y};

        return {.pts = pts.View()};
    }

    bool DoPolysInters(const s_poly a, const s_poly b) {
        return CheckPolySep(a, b) && CheckPolySep(b, a);
    }

    bool DoesPolyIntersWithRect(const s_poly poly, const s_rect rect) {
        const s_static_array<s_v2, 4> pts = {{
            {rect.x, rect.y},
            {rect.x + rect.width, rect.y},
            {rect.x + rect.width, rect.y + rect.height},
            {rect.x, rect.y + rect.height}
        }};

        return DoPolysInters(poly, {.pts = pts.Nonstatic()});
    }

    s_rect_edges PolySpan(const s_poly poly) {
        s_rect_edges span{.left = FLT_MAX, .top = FLT_MAX, .right = FLT_MIN, .bottom = FLT_MIN};

        for (t_s32 i = 0; i < poly.pts.Len(); i++) {
            const s_v2 pt = poly.pts[i];

            span.left = Min(pt.x, span.left);
            span.right = Max(pt.x, span.right);
            span.top = Min(pt.y, span.top);
            span.bottom = Max(pt.y, span.bottom);
        }

        return span;
    }
}
