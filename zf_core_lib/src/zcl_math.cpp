#include <zcl/zcl_math.h>

namespace zf {
    s_rect_f CalcSpanningRect(const s_array<s_rect_f> rects) {
        ZF_ASSERT(rects.Len() > 0);

        auto min_left = rects[0].Left();
        auto min_top = rects[0].Top();
        auto max_right = rects[0].Right();
        auto max_bottom = rects[0].Bottom();

        for (t_i32 i = 1; i < rects.Len(); i++) {
            min_left = ZF_MIN(rects[i].x, min_left);
            min_top = ZF_MIN(rects[i].y, min_top);
            max_right = ZF_MAX(rects[i].Right(), max_right);
            max_bottom = ZF_MAX(rects[i].Bottom(), max_bottom);
        }

        return {min_left, min_top, max_right - min_left, max_bottom - min_top};
    }

    s_rect_i CalcSpanningRect(const s_array<s_rect_i> rects) {
        ZF_ASSERT(rects.Len() > 0);

        auto min_left = rects[0].Left();
        auto min_top = rects[0].Top();
        auto max_right = rects[0].Right();
        auto max_bottom = rects[0].Bottom();

        for (t_i32 i = 1; i < rects.Len(); i++) {
            min_left = ZF_MIN(rects[i].x, min_left);
            min_top = ZF_MIN(rects[i].y, min_top);
            max_right = ZF_MAX(rects[i].Right(), max_right);
            max_bottom = ZF_MAX(rects[i].Bottom(), max_bottom);
        }

        return {min_left, min_top, max_right - min_left, max_bottom - min_top};
    }
}
