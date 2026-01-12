#pragma once

#include <cstdio>
#include <zcl/zcl_strs.h>

namespace zcl::io {
#if 0
    // ============================================================
    // @section: Streams

    enum t_stream_type : t_i32 {
        ek_stream_type_invalid,
        ek_stream_type_mem,
        ek_stream_type_file
    };

    enum t_stream_mode : t_i32 {
        ek_stream_mode_read,
        ek_stream_mode_write
    };

    struct t_stream {
        t_stream_type type;

        union {
            struct {
                t_array_mut<t_u8> bytes;
                t_i32 byte_pos;
            } mem;

            struct {
                FILE *file;
            } file;
        } type_data;

        t_stream_mode mode;
    };

    // @todo: Not sure how I feel about this function naming.
    inline t_stream mem_stream_create(const t_array_mut<t_u8> bytes, const t_stream_mode mode, const t_i32 pos = 0) {
        return {.type = ek_stream_type_mem, .type_data = {.mem = {.bytes = bytes, .byte_pos = pos}}, .mode = mode};
    }

    inline t_array_mut<t_u8> mem_stream_get_bytes_written(const t_stream *const stream) {
        ZF_ASSERT(stream->type == ek_stream_type_mem);
        return array_slice(stream->type_data.mem.bytes, 0, stream->type_data.mem.byte_pos);
    }

    inline t_stream file_stream_create(FILE *const file, const t_stream_mode mode) {
        return {.type = ek_stream_type_file, .type_data = {.file = {.file = file}}, .mode = mode};
    }

    inline t_stream get_std_in() { return file_stream_create(stdin, ek_stream_mode_read); }
    inline t_stream get_std_out() { return file_stream_create(stdout, ek_stream_mode_write); }
    inline t_stream get_std_error() { return file_stream_create(stderr, ek_stream_mode_write); }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 stream_read_item(t_stream *const stream, tp_type *const o_item) {
        ZF_ASSERT(stream->mode == ek_stream_mode_read);

        const t_i32 size = ZF_SIZE_OF(tp_type);

        switch (stream->type) {
        case ek_stream_type_mem: {
            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = array_slice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            const auto dest = mem::to_bytes(*o_item);
            array_copy(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ek_stream_type_file:
            return fread(o_item, size, 1, stream->type_data.file.file) == 1;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 stream_write_item(t_stream *const stream, const tp_type &item) {
        ZF_ASSERT(stream->mode == ek_stream_mode_write);

        const t_i32 size = ZF_SIZE_OF(tp_type);

        switch (stream->type) {
        case ek_stream_type_mem: {
            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = mem::to_bytes(item);
            const auto dest = array_slice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            array_copy(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ek_stream_type_file:
            return fwrite(&item, size, 1, stream->type_data.file.file) == 1;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <c_array_mut tp_arr_type>
    [[nodiscard]] t_b8 stream_read_items_into_array(t_stream *const stream, const tp_arr_type arr, const t_i32 cnt) {
        ZF_ASSERT(stream->mode == ek_stream_mode_read);
        ZF_ASSERT(cnt >= 0 && cnt <= arr.len);

        if (cnt == 0) {
            return true;
        }

        switch (stream->type) {
        case ek_stream_type_mem: {
            const t_i32 size = ZF_SIZE_OF(arr[0]) * cnt;

            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = array_slice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            const auto dest = mem::array_to_byte_array(arr);
            array_copy(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ek_stream_type_file:
            return static_cast<t_i32>(fread(arr.raw, sizeof(arr[0]), static_cast<size_t>(cnt), stream->type_data.file.file)) == cnt;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <c_array tp_arr_type>
    [[nodiscard]] t_b8 stream_write_items_of_array(t_stream *const stream, const tp_arr_type arr) {
        ZF_ASSERT(stream->mode == ek_stream_mode_write);

        if (arr.len == 0) {
            return true;
        }

        switch (stream->type) {
        case ek_stream_type_mem: {
            const t_i32 size = array_get_size_in_bytes(arr);

            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = mem::array_to_byte_array(arr);
            const auto dest = array_slice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            array_copy(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ek_stream_type_file:
            return static_cast<t_i32>(fwrite(arr.raw, sizeof(arr[0]), static_cast<size_t>(arr.len), stream->type_data.file.file)) == arr.len;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <c_array tp_arr_type>
    [[nodiscard]] t_b8 stream_serialize_array(t_stream *const stream, const tp_arr_type arr) {
        if (!stream_write_item(stream, arr.len)) {
            return false;
        }

        if (!stream_write_items_of_array(stream, arr)) {
            return false;
        }

        return true;
    }

    template <c_array_elem tp_elem_type>
    [[nodiscard]] t_b8 stream_deserialize_array(t_stream *const stream, mem::t_arena *const arr_arena, t_array_mut<tp_elem_type> *const o_arr) {
        t_i32 len;

        if (!stream_read_item(stream, &len)) {
            return false;
        }

        *o_arr = mem::arena_push_array<tp_elem_type>(arr_arena, len);

        if (!stream_read_items_into_array(stream, *o_arr, len)) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 stream_serialize_bitset(t_stream *const stream, const mem::t_bitset_rdonly bv) {
        if (!stream_write_item(stream, bv.bit_cnt)) {
            return false;
        }

        if (!stream_write_items_of_array(stream, mem::bitset_get_bytes(bv))) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 stream_deserialize_bitset(t_stream *const stream, mem::t_arena *const bv_arena, mem::t_bitset_mut *const o_bv) {
        t_i32 bit_cnt;

        if (!stream_read_item(stream, &bit_cnt)) {
            return false;
        }

        *o_bv = mem::bitset_create(bit_cnt, bv_arena);

        if (!stream_read_items_into_array(stream, mem::bitset_get_bytes(*o_bv), mem::bitset_get_bytes(*o_bv).len)) {
            return false;
        }

        return true;
    }

    // ============================================================


    // ============================================================
    // @section: Files and Directories

    enum t_file_access_mode : t_i32 {
        ek_file_access_mode_read,
        ek_file_access_mode_write,
        ek_file_access_mode_append
    };

    [[nodiscard]] t_b8 file_open(const strs::t_str_rdonly file_path, const t_file_access_mode mode, mem::t_arena *const temp_arena, t_stream *const o_stream);
    void file_close(t_stream *const stream);
    t_i32 file_calc_size(t_stream *const stream);
    [[nodiscard]] t_b8 file_load_contents(const strs::t_str_rdonly file_path, mem::t_arena *const contents_arena, mem::t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator = false);

    enum t_directory_creation_result : t_i32 {
        ek_directory_creation_result_success,
        ek_directory_creation_result_already_exists,
        ek_directory_creation_result_permission_denied,
        ek_directory_creation_result_path_not_found,
        ek_directory_creation_result_unknown_err
    };

    [[nodiscard]] t_b8 create_directory(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_creation_res = nullptr);
    [[nodiscard]] t_b8 create_directory_and_parents(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res = nullptr);
    [[nodiscard]] t_b8 create_file_and_parent_directories(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res = nullptr);

    enum t_path_type : t_i32 {
        ek_path_type_not_found,
        ek_path_type_file,
        ek_path_type_directory
    };

    t_path_type path_get_type(const strs::t_str_rdonly path, mem::t_arena *const temp_arena);

    strs::t_str_mut get_executable_directory(mem::t_arena *const arena);

    // ============================================================
#endif
}
