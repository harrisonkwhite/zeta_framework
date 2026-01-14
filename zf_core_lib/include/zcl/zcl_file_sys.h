#pragma once

#include <cstdio>
#include <zcl/zcl_basic.h>
#include <zcl/zcl_streams.h>
#include <zcl/zcl_strs.h>

namespace zcl {
    struct t_file_stream {
        FILE *file;
        t_stream_mode mode;

        operator t_stream() {
            const auto read_func = [](const t_stream stream, const t_array_mut<t_u8> dest_bytes) {
                ZCL_ASSERT(stream.mode == ek_stream_mode_read);

                const auto state = static_cast<t_file_stream *>(stream.data);
                return static_cast<t_i32>(fread(dest_bytes.raw, 1, static_cast<size_t>(dest_bytes.len), state->file)) == dest_bytes.len;
            };

            const auto write_func = [](const t_stream stream, const t_array_rdonly<t_u8> src_bytes) {
                ZCL_ASSERT(stream.mode == ek_stream_mode_write);

                const auto state = static_cast<t_file_stream *>(stream.data);
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

    inline t_file_stream file_stream_create_std_in() { return file_stream_create(stdin, ek_stream_mode_read); }
    inline t_file_stream file_stream_create_std_out() { return file_stream_create(stdout, ek_stream_mode_write); }
    inline t_file_stream file_stream_create_std_error() { return file_stream_create(stderr, ek_stream_mode_write); }

    enum t_file_access_mode : t_i32 {
        ek_file_access_mode_read,
        ek_file_access_mode_write,
        ek_file_access_mode_append
    };

    [[nodiscard]] t_b8 file_open(const t_str_rdonly file_path, const t_file_access_mode mode, t_arena *const temp_arena, t_file_stream *const o_stream);
    void file_close(t_file_stream *const stream);
    t_i32 file_calc_size(t_file_stream *const stream);
    [[nodiscard]] t_b8 file_load_contents(const t_str_rdonly file_path, t_arena *const contents_arena, t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator = false);

    enum t_directory_creation_result : t_i32 {
        ek_directory_creation_result_success,
        ek_directory_creation_result_already_exists,
        ek_directory_creation_result_permission_denied,
        ek_directory_creation_result_path_not_found,
        ek_directory_creation_result_unknown_err
    };

    [[nodiscard]] t_b8 create_directory(const t_str_rdonly path, t_arena *const temp_arena, t_directory_creation_result *const o_creation_res = nullptr);
    [[nodiscard]] t_b8 create_directory_and_parents(const t_str_rdonly path, t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res = nullptr);
    [[nodiscard]] t_b8 create_file_and_parent_directories(const t_str_rdonly path, t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res = nullptr);

    enum t_path_type : t_i32 {
        ek_path_type_not_found,
        ek_path_type_file,
        ek_path_type_directory
    };

    t_path_type get_path_type(const t_str_rdonly path, t_arena *const temp_arena);

    t_str_mut get_executable_directory(t_arena *const arena);
}
