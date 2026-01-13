#pragma once

#include <zcl/zcl_math.h>
#include <zcl/ds/zcl_hash_maps.h>
#include <zcl/zcl_strs.h>

namespace zcl::gfx {
    constexpr math::t_v2_i k_font_atlas_size = {1024, 1024};

    using t_font_atlas_rgba = t_static_array<t_u8, 4 * k_font_atlas_size.x * k_font_atlas_size.y>;

    struct t_font_glyph_info {
        math::t_v2_i offs;
        math::t_v2_i size;
        t_i32 adv;

        t_i32 atlas_index;
        math::t_rect_i atlas_rect;
    };

    struct t_font_code_pt_pair {
        strs::t_code_pt a;
        strs::t_code_pt b;
    };

    struct t_font_arrangement {
        t_i32 line_height;

        ds::t_hash_map<strs::t_code_pt, t_font_glyph_info> code_pts_to_glyph_infos;

        t_b8 has_kernings;
        ds::t_hash_map<t_font_code_pt_pair, t_i32> code_pt_pairs_to_kernings;
    };

    [[nodiscard]] t_b8 font_load_from_raw(const strs::t_str_rdonly file_path, const t_i32 height, strs::t_code_pt_bitset *const code_pts, t_arena *const arrangement_arena, t_arena *const atlas_rgbas_arena, t_arena *const temp_arena, t_font_arrangement *const o_arrangement, t_array_mut<t_font_atlas_rgba> *const o_atlas_rgbas);

    [[nodiscard]] t_b8 font_pack(const strs::t_str_rdonly file_path, const t_font_arrangement &arrangement, const t_array_rdonly<t_font_atlas_rgba> atlas_rgbas, t_arena *const temp_arena);
    [[nodiscard]] t_b8 font_unpack(const strs::t_str_rdonly file_path, t_arena *const arrangement_arena, t_arena *const atlas_rgbas_arena, t_arena *const temp_arena, t_font_arrangement *const o_arrangement, t_array_mut<t_font_atlas_rgba> *const o_atlas_rgbas);
}
