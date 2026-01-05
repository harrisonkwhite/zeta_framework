#include <zcl/zcl_math.h>

namespace zf::math {
    t_rect_f rects_get_span(const t_array_mut<t_rect_f> rects) {
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

        return {min_left, min_top, max_right - min_left, max_bottom - min_top};
    }

    t_rect_i rects_get_span(const t_array_mut<t_rect_i> rects) {
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
}
