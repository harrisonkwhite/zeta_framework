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

    [[nodiscard]] t_b8 DirectoryCreate(const t_str_rdonly path, t_arena *const temp_arena, t_directory_create_result *const o_create_result = nullptr);
    [[nodiscard]] t_b8 DirectoryCreateRecursive(const t_str_rdonly path, t_arena *const temp_arena, t_directory_create_result *const o_create_result = nullptr);

    t_str_mut GetExecutableDirectory(t_arena *const arena);

    // ============================================================


    // ============================================================
    // @section: Files

    struct t_file_stream {
        t_b8 open;
        FILE *file_raw;
        t_stream_mode mode;
    };

    inline t_file_stream FileStreamCreateOpen(FILE *const file_raw, const t_stream_mode mode) {
        return {
            .open = true,
            .file_raw = file_raw,
            .mode = mode,
        };
    }

    // This assumes the given  - there's no way of asserting this as far as I'm aware.
    t_file_stream FileStreamCreateOpen(FILE *const file_raw, const t_stream_mode mode);

    // The file stream must be open to get a view into it.
    t_stream_view FileStreamGetView(t_file_stream *const stream);

    inline t_file_stream FileStreamCreateStdIn() { return FileStreamCreateOpen(stdin, ek_stream_mode_read); }
    inline t_file_stream FileStreamCreateStdOut() { return FileStreamCreateOpen(stdout, ek_stream_mode_write); }
    inline t_file_stream FileStreamCreateStdError() { return FileStreamCreateOpen(stderr, ek_stream_mode_write); }

    [[nodiscard]] t_b8 FileCreate(const t_str_rdonly path, t_arena *const temp_arena);
    [[nodiscard]] t_b8 FileCreateRecursive(const t_str_rdonly path, t_arena *const temp_arena, t_directory_create_result *const o_dir_create_result = nullptr);

    enum t_file_access_mode : t_i32 {
        ek_file_access_mode_read,
        ek_file_access_mode_write,
        ek_file_access_mode_append
    };

    [[nodiscard]] t_b8 FileOpen(const t_str_rdonly path, const t_file_access_mode mode, t_arena *const temp_arena, t_file_stream *const o_stream);

    // Leave o_dir_create_result as nullptr if you don't want it.
    [[nodiscard]] t_b8 FileOpenRecursive(const t_str_rdonly path, const t_file_access_mode mode, t_arena *const temp_arena, t_file_stream *const o_stream, t_directory_create_result *const o_dir_create_result = nullptr);

    // Leave o_stream_new as nullptr if you don't want it.
    // o_stream_new is allowed to be equal to stream_current.
    [[nodiscard]] t_b8 FileReopen(t_file_stream *const stream_current, const t_str_rdonly path, const t_file_access_mode mode, t_arena *const temp_arena, t_file_stream *const o_stream_new);

    void FileClose(t_file_stream *const stream);

    void FileFlush(t_file_stream *const stream);

    t_i32 FileCalcSize(t_file_stream *const stream);

    [[nodiscard]] t_b8 FileLoadContents(const t_str_rdonly path, t_arena *const contents_arena, t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator = false);

    // ============================================================
}
