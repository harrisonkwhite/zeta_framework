#pragma once

#include <cstdio>
#include <zc/mem/strs.h>

namespace zf {
    constexpr char g_ascii_printable_min = ' ';
    constexpr char g_ascii_printable_max = '~';
    constexpr int g_ascii_printable_range_len = g_ascii_printable_max - g_ascii_printable_min + 1;

    struct s_file_stream {
        FILE* raw = nullptr;

        [[nodiscard]]
        bool Open(const s_str_view file_path, const bool is_write) {
            ZF_ASSERT(file_path.IsTerminated());
            raw = fopen(file_path.Raw(), is_write ? "wb" : "rb");
            return raw;
        }

        void Close() {
            fclose(raw);
            raw = nullptr;
        }

        size_t CalcSize() {
            const auto pos_old = ftell(raw);
            fseek(raw, 0, SEEK_END);
            const size_t file_size = ftell(raw);
            fseek(raw, pos_old, SEEK_SET);
            return file_size;
        }

        template<typename tp_type>
        [[nodiscard]]
        bool ReadItem(tp_type& item) {
            return fread(&item, sizeof(tp_type), 1, raw) == 1;
        }

        template<typename tp_type>
        [[nodiscard]]
        int ReadItems(const c_array<tp_type> arr) {
            return fread(arr.Raw(), sizeof(tp_type), arr.Len(), raw);
        }

        template<typename tp_type>
        [[nodiscard]]
        bool WriteItem(const tp_type& item) {
            return fwrite(&item, sizeof(tp_type), 1, raw) == 1;
        }

        template<typename tp_type>
        [[nodiscard]]
        int WriteItems(const c_array<const tp_type> arr) {
            return fwrite(arr.Raw(), sizeof(tp_type), arr.Len(), raw);
        }
    };

    bool LoadFileContents(c_array<t_byte>& contents, c_mem_arena& mem_arena, const s_str_view file_path, const bool include_terminating_byte = false);

    inline bool LoadFileContentsAsStr(s_str& contents, c_mem_arena& mem_arena, const s_str_view file_path) {
        ZF_ASSERT(contents.chrs.IsEmpty());

        c_array<t_byte> contents_default;

        if (!LoadFileContents(contents_default, mem_arena, file_path, true)) {
            return false;
        }

        contents = s_str::FromRawTerminated(reinterpret_cast<char*>(contents_default.Raw()), contents_default.Len() - 1);

        return true;
    }

    enum class ec_folder_create_result {
        success,
        already_exists,
        permission_denied,
        path_not_found,
        unknown_err
    };

    ec_folder_create_result CreateFolder(const s_str_view path); // This DOES NOT create non-existent parent folders.
}
