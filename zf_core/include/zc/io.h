#pragma once

#include <cstdio>
#include <zc/mem/strs.h>

namespace zf {
    constexpr char g_ascii_printable_min = ' ';
    constexpr char g_ascii_printable_max = '~';
    constexpr t_s32 g_ascii_printable_range_len = g_ascii_printable_max - g_ascii_printable_min + 1;

    struct s_file_stream {
        FILE* raw = nullptr;

        [[nodiscard]]
        t_b8 Open(const s_str_view file_path, const t_b8 is_write) {
            ZF_ASSERT(file_path.IsTerminated());
            raw = fopen(file_path.Raw(), is_write ? "wb" : "rb");
            return raw;
        }

        void Close() const {
            fclose(raw);
        }

        t_size CalcSize() const {
            const auto pos_old = ftell(raw);
            fseek(raw, 0, SEEK_END);
            const auto file_size = ftell(raw);
            fseek(raw, pos_old, SEEK_SET);
            return static_cast<t_size>(file_size);
        }

        template<typename tp_type>
        [[nodiscard]]
        t_b8 ReadItem(tp_type& item) const {
            return fread(&item, ZF_SIZE_OF(tp_type), 1, raw) == 1;
        }

        template<typename tp_type>
        [[nodiscard]]
        t_s32 ReadItems(const c_array<tp_type> arr) const {
            return static_cast<t_s32>(fread(arr.Raw(), ZF_SIZE_OF(tp_type), arr.Len(), raw));
        }

        template<typename tp_type>
        [[nodiscard]]
        t_b8 WriteItem(const tp_type& item) const {
            return fwrite(&item, ZF_SIZE_OF(tp_type), 1, raw) == 1;
        }

        template<typename tp_type>
        [[nodiscard]]
        t_s32 WriteItems(const c_array<const tp_type> arr) const {
            return static_cast<t_s32>(fwrite(arr.Raw(), ZF_SIZE_OF(tp_type), arr.Len(), raw));
        }
    };

    t_b8 LoadFileContents(c_array<t_s8>& contents, c_mem_arena& mem_arena, const s_str_view file_path, const t_b8 include_terminating_byte = false);

    inline t_b8 LoadFileContentsAsStr(s_str& contents, c_mem_arena& mem_arena, const s_str_view file_path) {
        ZF_ASSERT(contents.chrs.IsEmpty());

        c_array<t_s8> contents_default;

        if (!LoadFileContents(contents_default, mem_arena, file_path, true)) {
            return false;
        }

        contents = s_str::FromRawTerminated(reinterpret_cast<char*>(contents_default.Raw()), contents_default.Len() - 1);

        return true;
    }

#if 0
    enum class ec_directory_creation_result {
        success,
        already_exists,
        permission_denied,
        path_not_found,
        unknown_err
    };
#endif

    t_b8 CreateDirectory(const s_str_view path); // This DOES NOT create non-existent parent directories.
    t_b8 CreateDirectoryAndParents(const s_str_view path, c_mem_arena& temp_mem_arena);
    t_b8 CreateFileAndParentDirs(const s_str_view path, c_mem_arena& temp_mem_arena);

    enum class ec_path_type {
        not_found,
        file,
        directory
    };

    ec_path_type CheckPathType(const s_str_view path);
}
