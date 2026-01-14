#include <zcl/zcl_serialization.h>

namespace zcl {
    t_b8 SerializeBitset(const t_stream_view stream_view, const t_bitset_rdonly bs) {
        if (!stream_write_item(stream_view, bs.bit_cnt)) {
            return false;
        }

        if (!stream_write_items_of_array(stream_view, BitsetGetBytes(bs))) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeTexture(const t_stream_view stream_view, t_arena *const bs_arena, t_bitset_mut *const o_bs) {
        t_i32 bit_cnt;

        if (!stream_read_item(stream_view, &bit_cnt)) {
            return false;
        }

        *o_bs = BitsetCreate(bit_cnt, bs_arena);

        if (!stream_read_items_into_array(stream_view, BitsetGetBytes(*o_bs), BitsetGetBytes(*o_bs).len)) {
            return false;
        }

        return true;
    }

    t_b8 SerializeTexture(const t_stream_view stream_view, const t_texture_data_mut texture_data) {
        if (!stream_write_item(stream_view, texture_data.size_in_pxs)) {
            return false;
        }

        if (!stream_write_items_of_array(stream_view, texture_data.rgba_px_data)) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeTexture(const t_stream_view stream_view, t_arena *const texture_data_arena, t_texture_data_mut *const o_texture_data) {
        t_v2_i size_in_pxs;

        if (!stream_read_item(stream_view, &size_in_pxs)) {
            return false;
        }

        const auto rgba_px_data = arena_push_array<t_u8>(texture_data_arena, 4 * size_in_pxs.x * size_in_pxs.y);

        if (!stream_read_items_into_array(stream_view, rgba_px_data, rgba_px_data.len)) {
            return false;
        }

        *o_texture_data = {size_in_pxs, rgba_px_data};

        return true;
    }

    t_b8 SerializeFont(const t_stream_view stream_view, const t_font_arrangement &arrangement, const t_array_rdonly<t_font_atlas_rgba> atlas_rgbas, t_arena *const temp_arena) {
        if (!stream_write_item(stream_view, arrangement.line_height)) {
            return false;
        }

        if (!SerializeHashMap(&arrangement.code_pts_to_glyph_infos, stream_view, temp_arena)) {
            return false;
        }

        if (!SerializeHashMap(&arrangement.code_pt_pairs_to_kernings, stream_view, temp_arena)) {
            return false;
        }

        if (!SerializeArray(stream_view, atlas_rgbas)) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeFont(const t_stream_view stream_view, t_arena *const arrangement_arena, t_arena *const atlas_rgbas_arena, t_arena *const temp_arena, t_font_arrangement *const o_arrangement, t_array_mut<t_font_atlas_rgba> *const o_atlas_rgbas) {
        if (!stream_read_item(stream_view, &o_arrangement->line_height)) {
            return false;
        }

        if (!DeserializeHashMap(stream_view, arrangement_arena, k_font_code_point_hash_func, temp_arena, &o_arrangement->code_pts_to_glyph_infos)) {
            return false;
        }

        if (!DeserializeHashMap(stream_view, arrangement_arena, k_font_code_point_pair_hash_func, temp_arena, &o_arrangement->code_pt_pairs_to_kernings, k_font_code_point_pair_comparator)) {
            return false;
        }

        if (!DeserializeArray(stream_view, atlas_rgbas_arena, o_atlas_rgbas)) {
            return false;
        }

        return true;
    }

    t_b8 SerializeShader(const t_stream_view stream_view, const t_array_rdonly<t_u8> compiled_shader_bin) {
        if (!SerializeArray(stream_view, compiled_shader_bin)) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeShader(const t_stream_view stream_view, t_arena *const shader_bin_arena, t_array_mut<t_u8> *const o_shader_bin) {
        if (!DeserializeArray(stream_view, shader_bin_arena, o_shader_bin)) {
            return false;
        }

        return true;
    }
}
