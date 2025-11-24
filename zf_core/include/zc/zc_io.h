#pragma once

#include <cstdio>
#include <zc/zc_strs.h>
#include <zc/zc_mem.h>

namespace zf {
    enum e_file_access_mode : t_s32 {
        ek_file_access_mode_read,
        ek_file_access_mode_write,
        ek_file_access_mode_append
    };

    struct s_file_stream {
        FILE* raw;
    };

    [[nodiscard]] t_b8 OpenFile(const s_str_rdonly file_path, const e_file_access_mode mode, s_file_stream& o_fs);
    void CloseFile(s_file_stream& fs);
    t_size CalcFileSize(const s_file_stream& fs);

    template<typename tp_type>
    [[nodiscard]] t_b8 ReadItemFromFile(const s_file_stream& fs, tp_type& o_item) {
        return fread(&o_item, ZF_SIZE_OF(tp_type), 1, fs.raw) == 1;
    }

    template<c_array_mut tp_type>
    [[nodiscard]] t_size ReadItemArrayFromFile(const s_file_stream& fs, tp_type& dest_arr) {
        return fread(ArrayRaw(dest_arr), ZF_SIZE_OF(typename tp_type::t_elem), ArrayLen(dest_arr), fs.raw);
    }

    template<typename tp_type>
    [[nodiscard]] t_b8 WriteItemToFile(const s_file_stream& fs, const tp_type& o_item) {
        return fwrite(&o_item, ZF_SIZE_OF(tp_type), 1, fs.raw) == 1;
    }

    template<c_array tp_type>
    [[nodiscard]] t_size WriteItemArrayToFile(const s_file_stream& fs, tp_type& src_arr) {
        return fwrite(ArrayRaw(src_arr), ZF_SIZE_OF(typename tp_type::t_elem), ArrayLen(src_arr), fs.raw);
    }

    // Reserve a buffer and populate it with the binary contents of a file, optionally with a terminating byte.
    [[nodiscard]] t_b8 LoadFileContents(s_mem_arena& mem_arena, const s_str_rdonly file_path, s_array<t_u8>& o_contents, const t_b8 include_terminating_byte = false);

    // @todo: How will this work with non-ASCII?
    inline t_b8 LoadFileContentsAsStr(s_mem_arena& mem_arena, const s_str_rdonly file_path, s_str& o_contents) {
        s_array<t_u8> contents_default;

        if (!LoadFileContents(mem_arena, file_path, contents_default, true)) {
            return false;
        }

        o_contents = StrFromRawTerminated(reinterpret_cast<char*>(contents_default.buf_raw), contents_default.len - 1);

        return true;
    }

    // @todo: Use this!
    enum e_directory_creation_result : t_s32 {
        ek_directory_creation_result_success,
        ek_directory_creation_result_already_exists,
        ek_directory_creation_result_permission_denied,
        ek_directory_creation_result_path_not_found,
        ek_directory_creation_result_unknown_err
    };

    t_b8 CreateDirectory(const s_str_rdonly path); // This DOES NOT create non-existent parent directories.
    t_b8 CreateDirectoryAndParents(const s_str_rdonly path, s_mem_arena& temp_mem_arena);
    t_b8 CreateFileAndParentDirs(const s_str_rdonly path, s_mem_arena& temp_mem_arena);

    enum e_path_type : t_s32 {
        ek_path_type_not_found,
        ek_path_type_file,
        ek_path_type_directory
    };

    e_path_type CheckPathType(const s_str_rdonly path);
}
