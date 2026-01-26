#include <zcl/zcl_math.h>

#include <cmath>
#include <zcl/zcl_bits.h>

namespace zcl {
    t_f32 CalcMag(const t_v2 v) {
        return sqrt((v.x * v.x) + (v.y * v.y));
    }

    t_mat4x4 MatrixCreateRotated(const t_f32 rot) {
        t_mat4x4 result = MatrixCreateIdentity();
        result.elems[0][0] = cos(rot);
        result.elems[0][1] = sin(rot);
        result.elems[1][0] = -sin(rot);
        result.elems[1][1] = cos(rot);

        return result;
    }

    struct t_proj_interval {
        t_f32 min;
        t_f32 max;
    };

    static t_proj_interval ProjPoints(const t_array_rdonly<t_v2> pts, const t_v2 edge) {
        t_proj_interval interval = {
            .min = k_f32_max,
            .max = -k_f32_max,
        };

        for (t_i32 i = 0; i < pts.len; ++i) {
            const t_f32 dot = CalcDotProd(pts[i], edge);

            if (dot < interval.min) {
                interval.min = dot;
            }

            if (dot > interval.max) {
                interval.max = dot;
            }
        }

        return interval;
    }

    static t_b8 PolyCheckSep(const t_poly_rdonly poly, const t_poly_rdonly other) {
        for (t_i32 i = 0; i < poly.pts.len; ++i) {
            const t_v2 a = poly.pts[i];
            const t_v2 b = poly.pts[(i + 1) % poly.pts.len];

            const t_v2 normal = {b.y - a.y, -(b.x - a.x)};

            const t_proj_interval a_interval = ProjPoints(poly.pts, normal);
            const t_proj_interval b_interval = ProjPoints(other.pts, normal);

            if (a_interval.max <= b_interval.min || b_interval.max <= a_interval.min) {
                return false;
            }
        }

        return true;
    }

    t_poly_mut PolyCreateQuad(const t_v2 pos, const t_v2 size, const t_v2 origin, t_arena *const arena) {
        const t_poly_mut poly = {
            .pts = ArenaPushArray<t_v2>(arena, 4),
        };

        const t_v2 pos_base = pos - CalcCompwiseProd(size, origin);

        poly.pts[0] = pos_base;
        poly.pts[1] = pos_base + t_v2{size.x, 0.0f};
        poly.pts[2] = pos_base + t_v2{size.x, size.y};
        poly.pts[3] = pos_base + t_v2{0.0f, size.y};

        return poly;
    }

    t_poly_mut PolyCreateQuadRotated(const t_v2 pos, const t_v2 size, const t_v2 origin, const t_f32 rot, t_arena *const arena) {
        const t_poly_mut poly = {
            .pts = ArenaPushArray<t_v2>(arena, 4),
        };

        const t_v2 offs_left = CalcLengthdir(size.x * origin.x, rot - k_pi);
        const t_v2 offs_up = CalcLengthdir(size.y * origin.y, rot - (k_pi * 0.5f));
        const t_v2 offs_right = CalcLengthdir(size.x * (1.0f - origin.x), rot);
        const t_v2 offs_down = CalcLengthdir(size.y * (1.0f - origin.y), rot + (k_pi * 0.5f));

        poly.pts[0] = pos + offs_left + offs_up;
        poly.pts[1] = pos + offs_right + offs_up;
        poly.pts[2] = pos + offs_right + offs_down;
        poly.pts[3] = pos + offs_left + offs_down;

        return poly;
    }

    t_b8 PolyCheckIntersWithRect(const t_poly_rdonly poly, const t_rect_f rect) {
        const t_static_array<t_v2, 4> rect_poly_pts = {{
            {rect.x, rect.y},
            {rect.x + rect.width, rect.y},
            {rect.x + rect.width, rect.y + rect.height},
            {rect.x, rect.y + rect.height},
        }};

        return PolysCheckInters(poly, {.pts = rect_poly_pts});
    }

    t_poly_mut PolyCalcSpan(const t_rect_f a, const t_rect_f b, t_arena *const arena) {
        // The result is guaranteed to be a subset of the eight given polygon points. So really we just need to go through various cases and select which ones to omit.

        enum t_poly_pt_id : zcl::t_i32 {
            ek_poly_pt_id_rect_a_top_left,
            ek_poly_pt_id_rect_a_top_right,
            ek_poly_pt_id_rect_a_bottom_right, //
            ek_poly_pt_id_rect_a_bottom_left,
            ek_poly_pt_id_rect_b_top_left, //
            ek_poly_pt_id_rect_b_top_right,
            ek_poly_pt_id_rect_b_bottom_right,
            ek_poly_pt_id_rect_b_bottom_left,

            eks_poly_pt_id_cnt
        };

        t_static_bitset<eks_poly_pt_id_cnt> poly_pts_to_omit = {};

        if (RectGetBottom(a) <= RectGetBottom(b)) {
            if (RectGetRight(a) <= RectGetRight(b)) {
                BitsetUnset(poly_pts_to_omit, ek_poly_pt_id_rect_a_bottom_right);
                BitsetUnset(poly_pts_to_omit, ek_poly_pt_id_rect_b_top_left);
            }

            if (RectGetLeft(a) >= RectGetLeft(b)) {
                BitsetUnset(poly_pts_to_omit, ek_poly_pt_id_rect_a_bottom_left);
                BitsetUnset(poly_pts_to_omit, ek_poly_pt_id_rect_b_top_right);
            }
        }

        if (RectGetBottom(b) <= RectGetBottom(a)) {
            if (RectGetRight(b) <= RectGetRight(a)) {
                BitsetUnset(poly_pts_to_omit, ek_poly_pt_id_rect_b_bottom_right);
                BitsetUnset(poly_pts_to_omit, ek_poly_pt_id_rect_a_top_left);
            }

            if (RectGetLeft(b) >= RectGetLeft(a)) {
                BitsetUnset(poly_pts_to_omit, ek_poly_pt_id_rect_b_bottom_left);
                BitsetUnset(poly_pts_to_omit, ek_poly_pt_id_rect_a_top_right);
            }
        }
    }

    t_rect_f PolyCalcSpanRect(const t_poly_rdonly poly) {
        t_f32 min_left = poly.pts[0].x;
        t_f32 min_top = poly.pts[0].y;
        t_f32 max_right = poly.pts[0].x;
        t_f32 max_bottom = poly.pts[0].y;

        for (t_i32 i = 0; i < poly.pts.len; i++) {
            const t_v2 pt = poly.pts[i];

            min_left = CalcMin(pt.x, min_left);
            min_top = CalcMin(pt.y, min_top);
            max_right = CalcMax(pt.x, max_right);
            max_bottom = CalcMax(pt.y, max_bottom);
        }

        return RectCreateF(min_left, min_top, max_right - min_left, max_bottom - min_top);
    }

    t_b8 PolysCheckInters(const t_poly_rdonly a, const t_poly_rdonly b) {
        return PolyCheckSep(a, b) && PolyCheckSep(b, a);
    }

    t_f32 CalcDirRads(const t_v2 a, const t_v2 b) {
        const t_f32 rise = b.y - a.y;
        const t_f32 run = b.x - a.x;

        if (rise == 0.0f && run == 0.0f) {
            return 0.0f;
        }

        return atan2(rise, run);
    }

    t_v2 CalcLengthdir(const t_f32 len, const t_f32 dir) {
        return t_v2{cos(dir), sin(dir)} * len;
    }

    t_rect_f CalcSpanningRect(const t_array_mut<t_v2> pts) {
        ZCL_ASSERT(pts.len > 0);

        t_f32 min_left = pts[0].x;
        t_f32 min_top = pts[0].y;
        t_f32 max_right = pts[0].x;
        t_f32 max_bottom = pts[0].y;

        for (t_i32 i = 1; i < pts.len; i++) {
            min_left = CalcMin(pts[i].x, min_left);
            min_top = CalcMin(pts[i].y, min_top);
            max_right = CalcMax(pts[i].x, max_right);
            max_bottom = CalcMax(pts[i].y, max_bottom);
        }

        return RectCreateF(min_left, min_top, max_right - min_left, max_bottom - min_top);
    }

    t_rect_f CalcSpanningRect(const t_array_mut<t_rect_f> rects) {
        ZCL_ASSERT(rects.len > 0);

        t_f32 min_left = RectGetLeft(rects[0]);
        t_f32 min_top = RectGetTop(rects[0]);
        t_f32 max_right = RectGetRight(rects[0]);
        t_f32 max_bottom = RectGetBottom(rects[0]);

        for (t_i32 i = 1; i < rects.len; i++) {
            min_left = CalcMin(RectGetLeft(rects[i]), min_left);
            min_top = CalcMin(RectGetTop(rects[i]), min_top);
            max_right = CalcMax(RectGetRight(rects[i]), max_right);
            max_bottom = CalcMax(RectGetBottom(rects[i]), max_bottom);
        }

        return RectCreateF(min_left, min_top, max_right - min_left, max_bottom - min_top);
    }

    t_rect_i CalcSpanningRect(const t_array_mut<t_v2_i> pts) {
        ZCL_ASSERT(pts.len > 0);

        t_i32 min_left = pts[0].x;
        t_i32 min_top = pts[0].y;
        t_i32 max_right = pts[0].x;
        t_i32 max_bottom = pts[0].y;

        for (t_i32 i = 1; i < pts.len; i++) {
            min_left = CalcMin(pts[i].x, min_left);
            min_top = CalcMin(pts[i].y, min_top);
            max_right = CalcMax(pts[i].x, max_right);
            max_bottom = CalcMax(pts[i].y, max_bottom);
        }

        return RectCreateI(min_left, min_top, max_right - min_left, max_bottom - min_top);
    }

    t_rect_i CalcSpanningRect(const t_array_mut<t_rect_i> rects) {
        ZCL_ASSERT(rects.len > 0);

        t_i32 min_left = RectGetLeft(rects[0]);
        t_i32 min_top = RectGetTop(rects[0]);
        t_i32 max_right = RectGetRight(rects[0]);
        t_i32 max_bottom = RectGetBottom(rects[0]);

        for (t_i32 i = 1; i < rects.len; i++) {
            min_left = CalcMin(RectGetLeft(rects[i]), min_left);
            min_top = CalcMin(RectGetTop(rects[i]), min_top);
            max_right = CalcMax(RectGetRight(rects[i]), max_right);
            max_bottom = CalcMax(RectGetBottom(rects[i]), max_bottom);
        }

        return {min_left, min_top, max_right - min_left, max_bottom - min_top};
    }
}
