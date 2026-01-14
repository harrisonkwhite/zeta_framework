#pragma once

#include <cstdio>
#include <zcl/zcl_basic.h>
#include <zcl/zcl_streams.h>
#include <zcl/zcl_strs.h>

namespace zcl {
    enum t_path_type : t_i32 {
        ek_path_type_not_found,
        ek_path_type_directory,
        ek_path_type_file
    };

    t_path_type PathGetType(const t_str_rdonly path, t_arena *const temp_arena);


    // ============================================================
    // @section: Directories

    enum t_directory_create_result : t_i32 {
        ek_directory_create_result_success,
        ek_directory_create_result_already_exists,
        ek_directory_create_result_permission_denied,
        ek_directory_create_result_path_not_found,
        ek_directory_create_result_unknown_error
    };

    [[nodiscard]] t_b8 DirectoryCreate(const t_str_rdonly path, t_arena *const temp_arena, t_directory_create_result *const o_create_res = nullptr);
    [[nodiscard]] t_b8 DirectoryCreateRecursive(const t_str_rdonly path, t_arena *const temp_arena, t_directory_create_result *const o_create_res = nullptr);

    t_str_mut GetExecutableDirectory(t_arena *const arena);

    // ============================================================


    // ============================================================
    // @section: Files

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

    inline t_file_stream FileStreamCreate(FILE *const file, const t_stream_mode mode) {
        return {.file = file, .mode = mode};
    }

    inline t_file_stream FileStreamCreateStdIn() { return FileStreamCreate(stdin, ek_stream_mode_read); }
    inline t_file_stream FileStreamCreateStdOut() { return FileStreamCreate(stdout, ek_stream_mode_write); }
    inline t_file_stream FileStreamCreateStdError() { return FileStreamCreate(stderr, ek_stream_mode_write); }

    [[nodiscard]] t_b8 FileCreate(const t_str_rdonly path, t_arena *const temp_arena);
    [[nodiscard]] t_b8 FileCreateRecursive(const t_str_rdonly path, t_arena *const temp_arena, t_directory_create_result *const o_dir_create_res = nullptr);

    enum t_file_access_mode : t_i32 {
        ek_file_access_mode_read,
        ek_file_access_mode_write,
        ek_file_access_mode_append
    };

    [[nodiscard]] t_b8 FileOpen(const t_str_rdonly file_path, const t_file_access_mode mode, t_arena *const temp_arena, t_file_stream *const o_stream);

    void FileClose(t_file_stream *const stream);

    t_i32 FileCalcSize(t_file_stream *const stream);

    [[nodiscard]] t_b8 FileLoadContents(const t_str_rdonly file_path, t_arena *const contents_arena, t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator = false);

    // ============================================================
}
