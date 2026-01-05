#include <zcl/zcl_math.h>

namespace zf {
    t_rect_f f_math_calc_spanning_rect(const t_array_mut<t_rect_f> rects) {
        ZF_ASSERT(rects.len > 0);

        t_f32 min_left = f_math_get_rect_left(rects[0]);
        t_f32 min_top = f_math_get_rect_top(rects[0]);
        t_f32 max_right = f_math_get_rect_right(rects[0]);
        t_f32 max_bottom = f_math_get_rect_bottom(rects[0]);

        for (t_i32 i = 1; i < rects.len; i++) {
            min_left = ZF_MIN(f_math_get_rect_left(rects[i]), min_left);
            min_top = ZF_MIN(f_math_get_rect_top(rects[i]), min_top);
            max_right = ZF_MAX(f_math_get_rect_right(rects[i]), max_right);
            max_bottom = ZF_MAX(f_math_get_rect_bottom(rects[i]), max_bottom);
        }

        return {min_left, min_top, max_right - min_left, max_bottom - min_top};
    }

    t_rect_i f_math_calc_spanning_rect(const t_array_mut<t_rect_i> rects) {
        ZF_ASSERT(rects.len > 0);

        t_i32 min_left = f_math_get_rect_left(rects[0]);
        t_i32 min_top = f_math_get_rect_top(rects[0]);
        t_i32 max_right = f_math_get_rect_right(rects[0]);
        t_i32 max_bottom = f_math_get_rect_bottom(rects[0]);

        for (t_i32 i = 1; i < rects.len; i++) {
            min_left = ZF_MIN(f_math_get_rect_left(rects[i]), min_left);
            min_top = ZF_MIN(f_math_get_rect_top(rects[i]), min_top);
            max_right = ZF_MAX(f_math_get_rect_right(rects[i]), max_right);
            max_bottom = ZF_MAX(f_math_get_rect_bottom(rects[i]), max_bottom);
        }

        return {min_left, min_top, max_right - min_left, max_bottom - min_top};
    }
}
