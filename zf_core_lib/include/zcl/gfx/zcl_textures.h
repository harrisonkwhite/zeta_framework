#pragma once

#include <zcl/zcl_math.h>
#include <zcl/zcl_strs.h>

namespace zcl::gfx {
    struct t_texture_data_rdonly {
        t_v2_i size_in_pxs;
        t_array_rdonly<t_u8> rgba_px_data;
    };

    struct t_texture_data_mut {
        t_v2_i size_in_pxs;
        t_array_mut<t_u8> rgba_px_data;

        operator t_texture_data_rdonly() const {
            return {.size_in_pxs = size_in_pxs, .rgba_px_data = rgba_px_data};
        }
    };

    [[nodiscard]] t_b8 texture_load_from_raw(const t_str_rdonly file_path, t_arena *const texture_data_arena, t_arena *const temp_arena, t_texture_data_mut *const o_texture_data);

    [[nodiscard]] t_b8 texture_pack(const t_str_rdonly file_path, const t_texture_data_mut texture_data, t_arena *const temp_arena);
    [[nodiscard]] t_b8 texture_unpack(const t_str_rdonly file_path, t_arena *const texture_data_arena, t_arena *const temp_arena, t_texture_data_mut *const o_texture_data);

    constexpr t_rect_f texture_calc_uv_rect(const t_rect_i src_rect, const t_v2_i tex_size) {
        ZF_ASSERT(tex_size.x > 0 && tex_size.y > 0);
        ZF_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && src_rect.width > 0 && src_rect.height > 0 && rect_get_right(src_rect) <= tex_size.x && rect_get_bottom(src_rect) <= tex_size.y);

        return {
            static_cast<t_f32>(src_rect.x) / static_cast<t_f32>(tex_size.x),
            static_cast<t_f32>(src_rect.y) / static_cast<t_f32>(tex_size.y),
            static_cast<t_f32>(src_rect.width) / static_cast<t_f32>(tex_size.x),
            static_cast<t_f32>(src_rect.height) / static_cast<t_f32>(tex_size.y),
        };
    }
}
