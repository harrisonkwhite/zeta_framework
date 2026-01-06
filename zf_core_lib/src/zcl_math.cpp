#include <zcl/zcl_math.h>

namespace zf::math {
    t_rect_f rects_calc_span(const t_array_mut<t_rect_f> rects) {
        ZF_ASSERT(rects.len > 0);

        t_f32 min_left = rect_get_left(rects[0]);
        t_f32 min_top = rect_get_top(rects[0]);
        t_f32 max_right = rect_get_right(rects[0]);
        t_f32 max_bottom = rect_get_bottom(rects[0]);

        for (t_i32 i = 1; i < rects.len; i++) {
            min_left = min(rect_get_left(rects[i]), min_left);
            min_top = min(rect_get_top(rects[i]), min_top);
            max_right = max(rect_get_right(rects[i]), max_right);
            max_bottom = max(rect_get_bottom(rects[i]), max_bottom);
        }

        return rect_create_f32(min_left, min_top, max_right - min_left, max_bottom - min_top);
    }

    t_rect_i rects_calc_span(const t_array_mut<t_rect_i> rects) {
        ZF_ASSERT(rects.len > 0);

        t_i32 min_left = rect_get_left(rects[0]);
        t_i32 min_top = rect_get_top(rects[0]);
        t_i32 max_right = rect_get_right(rects[0]);
        t_i32 max_bottom = rect_get_bottom(rects[0]);

        for (t_i32 i = 1; i < rects.len; i++) {
            min_left = min(rect_get_left(rects[i]), min_left);
            min_top = min(rect_get_top(rects[i]), min_top);
            max_right = max(rect_get_right(rects[i]), max_right);
            max_bottom = max(rect_get_bottom(rects[i]), max_bottom);
        }

        return {min_left, min_top, max_right - min_left, max_bottom - min_top};
    }

    struct t_proj_interval {
        t_f32 min;
        t_f32 max;
    };

    static t_proj_interval project_pts(const t_array_rdonly<t_v2> pts, const t_v2 edge) {
        t_proj_interval interval = {
            .min = g_f32_max,
            .max = -g_f32_max,
        };

        for (t_i32 i = 0; i < pts.len; ++i) {
            const t_f32 dot = v2_calc_dot_prod(pts[i], edge);

            if (dot < interval.min) {
                interval.min = dot;
            }

            if (dot > interval.max) {
                interval.max = dot;
            }
        }

        return interval;
    }

    static t_b8 poly_check_separation(const t_poly_rdonly poly, const t_poly_rdonly other) {
        for (t_i32 i = 0; i < poly.pts.len; ++i) {
            const t_v2 a = poly.pts[i];
            const t_v2 b = poly.pts[(i + 1) % poly.pts.len];

            const t_v2 normal = {b.y - a.y, -(b.x - a.x)};

            const t_proj_interval a_interval = project_pts(poly.pts, normal);
            const t_proj_interval b_interval = project_pts(other.pts, normal);

            if (a_interval.max <= b_interval.min || b_interval.max <= a_interval.min) {
                return false;
            }
        }

        return true;
    }

    t_poly_mut poly_create_quad(const t_v2 pos, const t_v2 size, const t_v2 origin, mem::t_arena *const arena) {
        const t_poly_mut poly = {
            .pts = mem::arena_push_array<t_v2>(arena, 4),
        };

        const t_v2 pos_base = pos - v2_calc_compwise_prod(size, origin);

        poly.pts[0] = pos_base;
        poly.pts[1] = pos_base + t_v2{size.x, 0.0f};
        poly.pts[2] = pos_base + t_v2{size.x, size.y};
        poly.pts[3] = pos_base + t_v2{0.0f, size.y};

        return poly;
    }

    t_poly_mut poly_create_quad_rotated(const t_v2 pos, const t_v2 size, const t_v2 origin, const t_f32 rot, mem::t_arena *const arena) {
        const t_poly_mut poly = {
            .pts = mem::arena_push_array<t_v2>(arena, 4),
        };

        const t_v2 offs_left = calc_lengthdir(size.x * origin.x, rot - g_pi);
        const t_v2 offs_up = calc_lengthdir(size.y * origin.y, rot - (g_pi * 0.5f));
        const t_v2 offs_right = calc_lengthdir(size.x * (1.0f - origin.x), rot);
        const t_v2 offs_down = calc_lengthdir(size.y * (1.0f - origin.y), rot + (g_pi * 0.5f));

        poly.pts[0] = pos + offs_left + offs_up;
        poly.pts[1] = pos + offs_right + offs_up;
        poly.pts[2] = pos + offs_right + offs_down;
        poly.pts[3] = pos + offs_left + offs_down;

        return poly;
    }

    t_b8 polys_check_inters(const t_poly_rdonly a, const t_poly_rdonly b) {
        return poly_check_separation(a, b) && poly_check_separation(b, a);
    }

    t_b8 poly_check_inters_with_rect(const t_poly_rdonly poly, const t_rect_f rect) {
        const t_static_array<t_v2, 4> rect_poly_pts = {{
            {rect.x, rect.y},
            {rect.x + rect.width, rect.y},
            {rect.x + rect.width, rect.y + rect.height},
            {rect.x, rect.y + rect.height},
        }};

        return polys_check_inters(poly, {.pts = rect_poly_pts});
    }

    t_rect_f poly_calc_span(const t_poly_rdonly poly) {
        t_f32 min_left = poly.pts[0].x;
        t_f32 min_top = poly.pts[0].y;
        t_f32 max_right = poly.pts[0].x;
        t_f32 max_bottom = poly.pts[0].y;

        for (t_i32 i = 0; i < poly.pts.len; i++) {
            const t_v2 pt = poly.pts[i];

            min_left = min(pt.x, min_left);
            min_top = min(pt.y, min_top);
            max_right = max(pt.x, max_right);
            max_bottom = max(pt.y, max_bottom);
        }

        return rect_create_f32(min_left, min_top, max_right - min_left, max_bottom - min_top);
    }
}
