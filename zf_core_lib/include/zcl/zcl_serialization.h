#pragma once

#include <zcl/zcl_streams.h>
#include <zcl/zcl_gfx.h>
#include <zcl/zcl_audio.h>

namespace zcl {
    template <c_array tp_arr_type>
    [[nodiscard]] t_b8 SerializeArray(const t_stream_view stream_view, const tp_arr_type arr) {
        if (!StreamWriteItem(stream_view, arr.len)) {
            return false;
        }

        if (!StreamWriteItemsOfArray(stream_view, arr)) {
            return false;
        }

        return true;
    }

    template <c_array_elem tp_elem_type>
    [[nodiscard]] t_b8 DeserializeArray(const t_stream_view stream_view, t_arena *const arr_arena, t_array_mut<tp_elem_type> *const o_arr) {
        t_i32 len;

        if (!StreamReadItem(stream_view, &len)) {
            return false;
        }

        *o_arr = arena_push_array<tp_elem_type>(arr_arena, len);

        if (!StreamReadItemsIntoArray(stream_view, *o_arr, len)) {
            return false;
        }

        return true;
    }

    [[nodiscard]] t_b8 SerializeBitset(const t_stream_view stream_view, const t_bitset_rdonly bs);
    [[nodiscard]] t_b8 DeserializeBitset(const t_stream_view stream_view, t_arena *const bs_arena, t_bitset_mut *const o_bs);

    // @todo: List serialization!

    template <c_hash_map tp_hash_map_type>
    [[nodiscard]] t_b8 SerializeHashMap(const t_stream_view stream_view, const tp_hash_map_type *const hm, t_arena *const temp_arena) {
        if (!StreamWriteItem(stream_view, HashMapGetCap(hm))) {
            return false;
        }

        if (!StreamWriteItem(stream_view, HashMapGetEntryCount(hm))) {
            return false;
        }

        t_array_mut<typename tp_hash_map_type::t_key> keys;
        t_array_mut<typename tp_hash_map_type::t_value> values;
        HashMapLoadEntries(hm, temp_arena, &keys, &values);

        if (!StreamWriteItemsOfArray(stream_view, keys)) {
            return false;
        }

        if (!StreamWriteItemsOfArray(stream_view, values)) {
            return false;
        }

        return true;
    }

    template <c_hash_map tp_hash_map_type>
    [[nodiscard]] t_b8 DeserializeHashMap(const t_stream_view stream_view, t_arena *const hm_arena, const t_hash_func<typename tp_hash_map_type::t_key> hm_hash_func, t_arena *const temp_arena, tp_hash_map_type *const o_hm, const t_comparator_bin<typename tp_hash_map_type::t_key> hm_key_comparator = k_comparator_bin_default<typename tp_hash_map_type::t_key>) {
        t_i32 cap;

        if (!StreamReadItem(stream_view, &cap)) {
            return false;
        }

        t_i32 entry_cnt;

        if (!StreamReadItem(stream_view, &entry_cnt)) {
            return false;
        }

        *o_hm = HashMapCreate<typename tp_hash_map_type::t_key, typename tp_hash_map_type::t_value>(hm_hash_func, hm_arena, cap, hm_key_comparator);

        const auto keys = arena_push_array<typename tp_hash_map_type::t_key>(temp_arena, entry_cnt);

        if (!StreamReadItemsIntoArray(stream_view, keys, entry_cnt)) {
            return false;
        }

        const auto values = arena_push_array<typename tp_hash_map_type::t_value>(temp_arena, entry_cnt);

        if (!StreamReadItemsIntoArray(stream_view, values, entry_cnt)) {
            return false;
        }

        for (t_i32 i = 0; i < entry_cnt; i++) {
            HashMapPut(o_hm, keys[i], values[i]);
        }

        return true;
    }

    [[nodiscard]] t_b8 SerializeTexture(const t_stream_view stream_view, const t_texture_data_mut texture_data);
    [[nodiscard]] t_b8 DeserializeTexture(const t_stream_view stream_view, t_arena *const texture_data_arena, t_texture_data_mut *const o_texture_data);

    [[nodiscard]] t_b8 SerializeFont(const t_stream_view stream_view, const t_font_arrangement &arrangement, const t_array_rdonly<t_font_atlas_rgba> atlas_rgbas, t_arena *const temp_arena);
    [[nodiscard]] t_b8 DeserializeFont(const t_stream_view stream_view, t_arena *const arrangement_arena, t_arena *const atlas_rgbas_arena, t_arena *const temp_arena, t_font_arrangement *const o_arrangement, t_array_mut<t_font_atlas_rgba> *const o_atlas_rgbas);

    [[nodiscard]] t_b8 SerializeShader(const t_stream_view stream_view, const t_array_rdonly<t_u8> compiled_shader_bin);
    [[nodiscard]] t_b8 DeserializeShader(const t_stream_view stream_view, t_arena *const shader_bin_arena, t_array_mut<t_u8> *const o_shader_bin);

    [[nodiscard]] t_b8 SerializeSound(const t_stream_view stream_view, const t_sound_data_rdonly snd_data);
    [[nodiscard]] t_b8 DeserializeSound(const t_stream_view stream_view, t_arena *const snd_data_arena, t_sound_data_mut *const o_snd_data);
}
