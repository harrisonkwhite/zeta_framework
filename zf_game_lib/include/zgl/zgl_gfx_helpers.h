#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx_core.h>

namespace zf::gfx {
    void rendering_submit_texture(s_rendering_context *const rc, const s_resource *const texture, const s_v2 pos, const s_rect_i src_rect = {});

    struct s_font {
        s_font_arrangement arrangement;
        s_array_mut<s_resource *> atlases;
    };

    s_font font_create_from_raw(const s_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec *const code_pts, s_arena *const temp_arena, s_resource_group *const resource_group);
    s_font font_create_from_packed(const s_str_rdonly file_path, s_arena *const temp_arena, s_resource_group *const resource_group);

    s_array_mut<s_v2> calc_str_chr_positions(const s_str_rdonly str, const s_font_arrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, s_arena *const arena);

    void rendering_submit_str(s_rendering_context *const rc, const s_str_rdonly str, const s_font &font, const s_v2 pos, s_arena *const temp_arena, const s_v2 alignment = g_alignment_topleft, const s_color_rgba32f blend = g_color_white);
}
