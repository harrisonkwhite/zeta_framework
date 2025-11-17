#pragma once

#include <cstdio>
#include <zc/zc_strs.h>
#include <zc/zc_allocators.h>

namespace zf {
    constexpr char g_ascii_printable_min = ' ';
    constexpr char g_ascii_printable_max = '~';
    //constexpr t_s32 g_ascii_printable_range_len = g_ascii_printable_max - g_ascii_printable_min + 1;

    enum ec_file_access_mode {
        read,
        write
    };

    struct s_file_stream {
        FILE* raw = nullptr;

        [[nodiscard]]
        t_b8 Open(const s_str_rdonly file_path, const ec_file_access_mode mode) {
            ZF_ASSERT(IsStrTerminated(file_path));

            switch (mode) {
            case ec_file_access_mode::read:
                raw = fopen(file_path.Raw(), "rb");
                break;

            case ec_file_access_mode::write:
                raw = fopen(file_path.Raw(), "wb");
                break;
            }

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
        t_size ReadItems(const s_array<tp_type> arr) const {
            return fread(arr.Raw(), ZF_SIZE_OF(tp_type), arr.Len(), raw);
        }

        template<typename tp_type>
        [[nodiscard]]
        t_b8 WriteItem(const tp_type& item) const {
            return fwrite(&item, ZF_SIZE_OF(tp_type), 1, raw) == 1;
        }

        template<typename tp_type>
        [[nodiscard]]
        t_size WriteItems(const s_array<const tp_type> arr) const {
            return fwrite(arr.Raw(), ZF_SIZE_OF(tp_type), arr.Len(), raw);
        }
    };

    t_b8 LoadFileContents(s_mem_arena& mem_arena, const s_str_rdonly file_path, s_array<t_s8>& o_contents, const t_b8 include_terminating_byte = false);

    inline t_b8 LoadFileContentsAsStr(s_mem_arena& mem_arena, const s_str_rdonly file_path, s_str& o_contents) {
        s_array<t_s8> contents_default;

        if (!LoadFileContents(mem_arena, file_path, contents_default, true)) {
            return false;
        }

        o_contents = StrFromRawTerminated(reinterpret_cast<char*>(contents_default.Raw()), contents_default.Len() - 1);

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
