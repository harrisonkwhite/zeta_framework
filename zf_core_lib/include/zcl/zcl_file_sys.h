#pragma once

#include <cstdio>
#include <zcl/zcl_strs.h>

namespace zcl::file_sys {
    // ============================================================
    // @section: Types and Constants

    enum t_file_access_mode : t_i32 {
        ek_file_access_mode_read,
        ek_file_access_mode_write,
        ek_file_access_mode_append
    };

    enum t_directory_creation_result : t_i32 {
        ek_directory_creation_result_success,
        ek_directory_creation_result_already_exists,
        ek_directory_creation_result_permission_denied,
        ek_directory_creation_result_path_not_found,
        ek_directory_creation_result_unknown_err
    };

    enum t_path_type : t_i32 {
        ek_path_type_not_found,
        ek_path_type_file,
        ek_path_type_directory
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    struct t_file_stream {
        FILE *file;
        t_stream_mode mode;

        operator t_stream() {
            const auto read_func = [](t_stream *const stream, const t_array_mut<t_i8> dest_bytes) {
                ZF_ASSERT(stream->mode == ek_stream_mode_read);

                const auto state = static_cast<t_file_stream *>(stream->data);
                return static_cast<t_i32>(fread(dest_bytes.raw, 1, static_cast<size_t>(dest_bytes.len), state->file)) == dest_bytes.len;
            };

            const auto write_func = [](t_stream *const stream, const t_array_rdonly<t_i8> src_bytes) {
                ZF_ASSERT(stream->mode == ek_stream_mode_read);

                const auto state = static_cast<t_file_stream *>(stream->data);
                return static_cast<t_i32>(fwrite(src_bytes.raw, 1, static_cast<size_t>(src_bytes.len), state->file)) == src_bytes.len;
            };

            return {
                .data = this,
                .read_func = read_func,
                .write_func = write_func,
                .mode = mode,
            };
        }
    };

    inline t_file_stream file_stream_create(FILE *const file, const t_stream_mode mode) {
        return {.file = file, .mode = mode};
    }

    inline t_file_stream get_std_in() { return file_stream_create(stdin, ek_stream_mode_read); }
    inline t_file_stream get_std_out() { return file_stream_create(stdout, ek_stream_mode_write); }
    inline t_file_stream get_std_error() { return file_stream_create(stderr, ek_stream_mode_write); }

    [[nodiscard]] t_b8 file_open(const strs::t_str_rdonly file_path, const t_file_access_mode mode, mem::t_arena *const temp_arena, t_stream *const o_stream);
    void file_close(t_stream *const stream);
    t_i32 file_calc_size(t_stream *const stream);
    [[nodiscard]] t_b8 file_load_contents(const strs::t_str_rdonly file_path, mem::t_arena *const contents_arena, mem::t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator = false);

    [[nodiscard]] t_b8 create_directory(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_creation_res = nullptr);
    [[nodiscard]] t_b8 create_directory_and_parents(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res = nullptr);
    [[nodiscard]] t_b8 create_file_and_parent_directories(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res = nullptr);

    t_path_type path_get_type(const strs::t_str_rdonly path, mem::t_arena *const temp_arena);

    strs::t_str_mut get_executable_directory(mem::t_arena *const arena);

#if 0
    inline t_file_stream get_std_in() { return file_stream_create(stdin, ek_stream_mode_read); }
    inline t_file_stream get_std_out() { return file_stream_create(stdout, ek_stream_mode_write); }
    inline t_file_stream get_std_error() { return file_stream_create(stderr, ek_stream_mode_write); }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 file_stream_read_item(t_stream *const stream, tp_type *const o_item) {
        ZF_ASSERT(stream->mode == ek_stream_mode_read);
        return fread(o_item, ZF_SIZE_OF(tp_type), 1, ) == 1;
    }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 file_stream_write_item(t_stream *const stream, const tp_type &item) {
        ZF_ASSERT(stream->mode == ek_stream_mode_write);
        return fwrite(&item, ZF_SIZE_OF(tp_type), 1, stream->type_data.file.file) == 1;
    }

    template <c_array_mut tp_arr_type>
    [[nodiscard]] t_b8 file_stream_read_items_into_array(t_stream *const stream, const tp_arr_type arr, const t_i32 cnt) {
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
    [[nodiscard]] t_b8 file_stream_write_items_of_array(t_stream *const stream, const tp_arr_type arr) {
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


    [[nodiscard]] t_b8 file_open(const strs::t_str_rdonly file_path, const t_file_access_mode mode, mem::t_arena *const temp_arena, t_stream *const o_stream);
    void file_close(t_stream *const stream);
    t_i32 file_calc_size(t_stream *const stream);
    [[nodiscard]] t_b8 file_load_contents(const strs::t_str_rdonly file_path, mem::t_arena *const contents_arena, mem::t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator = false);


    [[nodiscard]] t_b8 create_directory(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_creation_res = nullptr);
    [[nodiscard]] t_b8 create_directory_and_parents(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res = nullptr);
    [[nodiscard]] t_b8 create_file_and_parent_directories(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res = nullptr);


    t_path_type path_get_type(const strs::t_str_rdonly path, mem::t_arena *const temp_arena);

    strs::t_str_mut get_executable_directory(mem::t_arena *const arena);
#endif

    // ============================================================
}
