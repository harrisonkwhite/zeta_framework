#include <zcl/zcl_serialization.h>

namespace zcl {
    t_b8 SerializeBitset(const t_stream_view stream_view, const t_bitset_rdonly bs) {
        if (!StreamWriteItem(stream_view, bs.bit_cnt)) {
            return false;
        }

        if (!StreamWriteItemsOfArray(stream_view, BitsetGetBytes(bs))) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeBitset(const t_stream_view stream_view, t_arena *const bs_arena, t_bitset_mut *const o_bs) {
        t_i32 bit_cnt;

        if (!StreamReadItem(stream_view, &bit_cnt)) {
            return false;
        }

        *o_bs = BitsetCreate(bit_cnt, bs_arena);

        if (!StreamReadItemsIntoArray(stream_view, BitsetGetBytes(*o_bs), BitsetGetBytes(*o_bs).len)) {
            return false;
        }

        return true;
    }

    t_b8 SerializeTexture(const t_stream_view stream_view, const t_texture_data_mut texture_data) {
        if (!StreamWriteItem(stream_view, texture_data.dims)) {
            return false;
        }

        if (!StreamWriteItem(stream_view, texture_data.format)) {
            return false;
        }

        switch (texture_data.format) {
            case ek_texture_format_rgba32f: {
                if (!StreamWriteItemsOfArray(stream_view, texture_data.pixels.rgba32f)) {
                    return false;
                }

                break;
            }

            case ek_texture_format_rgba8: {
                if (!StreamWriteItemsOfArray(stream_view, texture_data.pixels.rgba8)) {
                    return false;
                }

                break;
            }

            case ek_texture_format_r8: {
                if (!StreamWriteItemsOfArray(stream_view, texture_data.pixels.r8)) {
                    return false;
                }

                break;
            }
        }

        return true;
    }

    t_b8 DeserializeTexture(const t_stream_view stream_view, t_arena *const texture_data_arena, t_texture_data_mut *const o_texture_data) {
        *o_texture_data = {};

        if (!StreamReadItem(stream_view, &o_texture_data->dims)) {
            return false;
        }

        if (!StreamReadItem(stream_view, &o_texture_data->format)) {
            return false;
        }

        switch (o_texture_data->format) {
            case ek_texture_format_rgba32f: {
                o_texture_data->pixels.rgba32f = ArenaPushArray<t_color_rgba32f>(texture_data_arena, o_texture_data->dims.x * o_texture_data->dims.y);

                if (!StreamReadItemsIntoArray(stream_view, o_texture_data->pixels.rgba32f, o_texture_data->pixels.rgba32f.len)) {
                    return false;
                }

                break;
            }

            case ek_texture_format_rgba8: {
                o_texture_data->pixels.rgba8 = ArenaPushArray<t_color_rgba8>(texture_data_arena, o_texture_data->dims.x * o_texture_data->dims.y);

                if (!StreamReadItemsIntoArray(stream_view, o_texture_data->pixels.rgba8, o_texture_data->pixels.rgba8.len)) {
                    return false;
                }

                break;
            }

            case ek_texture_format_r8: {
                o_texture_data->pixels.r8 = ArenaPushArray<t_color_r8>(texture_data_arena, o_texture_data->dims.x * o_texture_data->dims.y);

                if (!StreamReadItemsIntoArray(stream_view, o_texture_data->pixels.r8, o_texture_data->pixels.r8.len)) {
                    return false;
                }

                break;
            }
        }

        return true;
    }

    t_b8 SerializeFont(const t_stream_view stream_view, const t_font_arrangement &arrangement, const t_array_rdonly<t_font_atlas_pixels_r8> atlas_pixels_arr, t_arena *const temp_arena) {
        if (!StreamWriteItem(stream_view, arrangement.line_height)) {
            return false;
        }

        if (!SerializeHashMap(stream_view, &arrangement.code_pts_to_glyph_infos, temp_arena)) {
            return false;
        }

        if (!SerializeHashMap(stream_view, &arrangement.code_pt_pairs_to_kernings, temp_arena)) {
            return false;
        }

        if (!SerializeArray(stream_view, atlas_pixels_arr)) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeFont(const t_stream_view stream_view, t_arena *const arrangement_arena, t_arena *const atlas_pixels_arr_arena, t_arena *const temp_arena, t_font_arrangement *const o_arrangement, t_array_mut<t_font_atlas_pixels_r8> *const o_atlas_pixels_arr) {
        *o_arrangement = {};

        if (!StreamReadItem(stream_view, &o_arrangement->line_height)) {
            return false;
        }

        if (!DeserializeHashMap(stream_view, arrangement_arena, k_font_code_point_hash_func, temp_arena, &o_arrangement->code_pts_to_glyph_infos)) {
            return false;
        }

        if (!DeserializeHashMap(stream_view, arrangement_arena, k_font_code_point_pair_hash_func, temp_arena, &o_arrangement->code_pt_pairs_to_kernings, k_font_code_point_pair_comparator)) {
            return false;
        }

        if (!DeserializeArray(stream_view, atlas_pixels_arr_arena, o_atlas_pixels_arr)) {
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

    t_b8 SerializeSound(const t_stream_view stream_view, const t_sound_data_rdonly snd_data) {
        if (!StreamWriteItem(stream_view, snd_data.meta)) {
            return false;
        }

        if (!SerializeArray(stream_view, snd_data.pcm)) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeSound(const t_stream_view stream_view, t_arena *const snd_data_arena, t_sound_data_mut *const o_snd_data) {
        *o_snd_data = {};

        if (!StreamReadItem(stream_view, &o_snd_data->meta)) {
            return false;
        }

        if (!DeserializeArray(stream_view, snd_data_arena, &o_snd_data->pcm)) {
            return false;
        }

        return true;
    }
}
