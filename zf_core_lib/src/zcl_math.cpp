#include <zcl/zcl_math.h>

namespace zf {
    s_rect_f CalcSpanningRect(const t_array_mut<s_rect_f> rects) {
        ZF_ASSERT(rects.len > 0);

        t_f32 min_left = Left(rects[0]);
        t_f32 min_top = Top(rects[0]);
        t_f32 max_right = Right(rects[0]);
        t_f32 max_bottom = Bottom(rects[0]);

        for (t_i32 i = 1; i < rects.len; i++) {
            min_left = ZF_MIN(Left(rects[i]), min_left);
            min_top = ZF_MIN(Top(rects[i]), min_top);
            max_right = ZF_MAX(Right(rects[i]), max_right);
            max_bottom = ZF_MAX(Bottom(rects[i]), max_bottom);
        }

        return {min_left, min_top, max_right - min_left, max_bottom - min_top};
    }

    s_rect_i CalcSpanningRect(const t_array_mut<s_rect_i> rects) {
        ZF_ASSERT(rects.len > 0);

        t_i32 min_left = Left(rects[0]);
        t_i32 min_top = Top(rects[0]);
        t_i32 max_right = Right(rects[0]);
        t_i32 max_bottom = Bottom(rects[0]);

        for (t_i32 i = 1; i < rects.len; i++) {
            min_left = ZF_MIN(Left(rects[i]), min_left);
            min_top = ZF_MIN(Top(rects[i]), min_top);
            max_right = ZF_MAX(Right(rects[i]), max_right);
            max_bottom = ZF_MAX(Bottom(rects[i]), max_bottom);
        }

        return {min_left, min_top, max_right - min_left, max_bottom - min_top};
    }
}
