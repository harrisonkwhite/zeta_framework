#pragma once

#include <cstdio>
#include <zc/zc_strs.h>
#include <zc/zc_mem.h>

namespace zf {
    enum ec_file_access_mode {
        read,
        write,
        append
    };

    struct s_file_stream {
        FILE* raw;
    };

    [[nodiscard]] t_b8 OpenFile(const s_str_rdonly file_path, const ec_file_access_mode mode, s_file_stream& o_fs);
    void CloseFile(s_file_stream& fs);
    t_size CalcFileSize(const s_file_stream& fs);

    template<typename tp_type>
    [[nodiscard]] t_b8 ReadItemFromFile(const s_file_stream& fs, tp_type& o_item) {
        return fread(&o_item, ZF_SIZE_OF(tp_type), 1, fs.raw) == 1;
    }

    template<typename tp_type>
    [[nodiscard]] t_size ReadItemArrayFromFile(const s_file_stream& fs, const s_array<tp_type> dest_arr) {
        return fread(dest_arr.buf_raw, ZF_SIZE_OF(tp_type), dest_arr.len, fs.raw);
    }

    template<typename tp_type>
    [[nodiscard]] t_b8 WriteItemToFile(const s_file_stream& fs, const tp_type& o_item) {
        return fwrite(&o_item, ZF_SIZE_OF(tp_type), 1, fs.raw) == 1;
    }

    template<typename tp_type>
    [[nodiscard]] t_size WriteItemArrayToFile(const s_file_stream& fs, const s_array_rdonly<tp_type> src_arr) {
        return fwrite(src_arr.buf_raw, ZF_SIZE_OF(tp_type), src_arr.len, fs.raw);
    }

    // Reserve a buffer and populate it with the binary contents of a file, optionally with a terminating byte.
    [[nodiscard]] t_b8 LoadFileContents(s_mem_arena& mem_arena, const s_str_rdonly file_path, s_array<t_s8>& o_contents, const t_b8 include_terminating_byte = false);

    // @todo: How will this work with non-ASCII?
    inline t_b8 LoadFileContentsAsStr(s_mem_arena& mem_arena, const s_str_rdonly file_path, s_str& o_contents) {
        s_array<t_s8> contents_default;

        if (!LoadFileContents(mem_arena, file_path, contents_default, true)) {
            return false;
        }

        o_contents = StrFromRawTerminated(reinterpret_cast<char*>(contents_default.buf_raw), contents_default.len - 1);

        return true;
    }

    // @todo: Use this!
    enum class ec_directory_creation_result {
        success,
        already_exists,
        permission_denied,
        path_not_found,
        unknown_err
    };

    t_b8 CreateDirectory(const s_str_rdonly path); // This DOES NOT create non-existent parent directories.
    t_b8 CreateDirectoryAndParents(const s_str_rdonly path, s_mem_arena& temp_mem_arena);
    t_b8 CreateFileAndParentDirs(const s_str_rdonly path, s_mem_arena& temp_mem_arena);

    enum class ec_path_type {
        not_found,
        file,
        directory
    };

    ec_path_type CheckPathType(const s_str_rdonly path);
}
