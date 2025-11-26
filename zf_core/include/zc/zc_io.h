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

    enum e_stream_type : t_s32 {
        ek_stream_type_mem,
        ek_stream_type_file
    };

    enum e_stream_mode : t_s32 {
        ek_stream_mode_read,
        ek_stream_mode_write
    };

    struct s_stream {
        e_stream_mode mode;

        e_stream_type type;

        union {
            struct {
                s_array<t_u8> bytes;
                t_size pos;
            } mem;

            struct {
                FILE* fs_raw;
            } file;
        } type_data;
    };

    inline s_stream StdIn() {
        return {
            .mode = ek_stream_mode_read,
            .type = ek_stream_type_file,
            .type_data = {.file = {.fs_raw = stdin}}
        };
    }

    inline s_stream StdOut() {
        return {
            .mode = ek_stream_mode_write,
            .type = ek_stream_type_file,
            .type_data = {.file = {.fs_raw = stdout}}
        };
    }

    inline s_stream StdError() {
        return {
            .mode = ek_stream_mode_write,
            .type = ek_stream_type_file,
            .type_data = {.file = {.fs_raw = stderr}}
        };
    }

    template<typename tp_type>
    [[nodiscard]] t_b8 StreamReadItem(s_stream& stream, tp_type& o_item) {
        ZF_ASSERT(stream.mode == ek_stream_mode_read);

        constexpr t_size size = ZF_SIZE_OF(tp_type);

        switch(stream.type) {
            case ek_stream_type_mem:
                if (stream.type_data.mem.pos + size > stream.type_data.mem.bytes.len) {
                    return false;
                }

                {
                    const auto dest = ToBytes(o_item);
                    const auto src = Slice(stream.type_data.mem.bytes, stream.type_data.mem.pos, stream.type_data.mem.pos + size);
                    Copy(dest, src);

                    stream.type_data.mem.pos += size;
                }

                return true;

            case ek_stream_type_file:
                return fread(&o_item, size, 1, stream.type_data.file.fs_raw) == 1;
        }

        ZF_ASSERT(false);
        return false;
    }

    template<typename tp_type>
    [[nodiscard]] t_b8 StreamWriteItem(s_stream& stream, const tp_type& item) {
        ZF_ASSERT(stream.mode == ek_stream_mode_write);

        constexpr t_size size = ZF_SIZE_OF(tp_type);

        switch(stream.type) {
            case ek_stream_type_mem:
                if (stream.type_data.mem.pos + size > stream.type_data.mem.bytes.len) {
                    return false;
                }

                {
                    const auto dest = Slice(stream.type_data.mem.bytes, stream.type_data.mem.pos, stream.type_data.mem.pos + size);
                    const auto src = ToBytes(item);
                    Copy(dest, src);

                    stream.type_data.mem.pos += size;
                }

                return true;

            case ek_stream_type_file:
                return fwrite(&item, size, 1, stream.type_data.file.fs_raw) == 1;
        }

        ZF_ASSERT(false);
        return false;
    }

    template<c_array tp_type>
    [[nodiscard]] t_b8 StreamReadItemsIntoArray(s_stream& stream, tp_type& arr, const t_size cnt) {
        ZF_ASSERT(stream.mode == ek_stream_mode_read);
        ZF_ASSERT(cnt >= 0 && cnt <= ArrayLen(arr));

        if (cnt == 0) {
            return true;
        }

        switch(stream.type) {
            case ek_stream_type_mem:
                {
                    const t_size size = ZF_SIZE_OF(arr[0]) * cnt;

                    if (stream.type_data.mem.pos + size > stream.type_data.mem.bytes.len) {
                        return false;
                    }

                    const auto dest = Slice(stream.type_data.mem.bytes, stream.type_data.mem.pos, stream.type_data.mem.pos + size);
                    const auto src = ToByteArray(arr);
                    Copy(dest, src);

                    stream.type_data.mem.pos += size;
                }

                return true;

            case ek_stream_type_file:
                return fread(ArrayRaw(arr), sizeof(arr[0]), cnt, stream.type_data.file.fs_raw) == cnt;
        }

        ZF_ASSERT(false);
        return false;
    }

    template<c_array tp_type>
    [[nodiscard]] t_b8 StreamWriteItemsOfArray(s_stream& stream, tp_type& arr) {
        ZF_ASSERT(stream.mode == ek_stream_mode_write);

        if (IsArrayEmpty(arr)) {
            return true;
        }

        switch(stream.type) {
            case ek_stream_type_mem:
                {
                    const t_size size = ArraySizeInBytes(arr);

                    if (stream.type_data.mem.pos + size > stream.type_data.mem.bytes.len) {
                        return false;
                    }

                    const auto dest = Slice(stream.type_data.mem.bytes, stream.type_data.mem.pos, stream.type_data.mem.pos + size);
                    const auto src = ToByteArray(arr);
                    Copy(dest, src);

                    stream.type_data.mem.pos += size;
                }

                return true;

            case ek_stream_type_file:
                return fwrite(ArrayRaw(arr), sizeof(arr[0]), ArrayLen(arr), stream.type_data.file.fs_raw) == ArrayLen(arr);
        }

        ZF_ASSERT(false);
        return false;
    }

    template<c_array tp_type>
    [[nodiscard]] t_b8 SerializeArray(s_stream& stream, tp_type& arr) {
        if (!StreamWriteItem(stream, arr.len)) {
            return false;
        }

        if (!StreamWriteItemsOfArray(stream, arr)) {
            return false;
        }

        return true;
    }

    template<typename tp_type>
    [[nodiscard]] t_b8 DeserializeArray(s_stream& stream, s_mem_arena& mem_arena, s_array<tp_type>& o_arr) {
        o_arr = {};

        if (!StreamReadItem(stream, o_arr.len)) {
            return false;
        }

        if (o_arr.len > 0) {
            if (!MakeArray(mem_arena, o_arr.len, o_arr)) {
                return false;
            }

            if (!StreamReadItemsIntoArray(stream, o_arr, o_arr.len)) {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] t_b8 OpenFile(const s_str_rdonly file_path, const e_file_access_mode mode, s_mem_arena& temp_mem_arena, s_stream& o_fs);
    void CloseFile(s_stream& fs);
    t_size CalcFileSize(const s_stream& fs);
    [[nodiscard]] t_b8 LoadFileContents(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena, s_array<t_u8>& o_contents);

    enum e_directory_creation_result : t_s32 {
        ek_directory_creation_result_success,
        ek_directory_creation_result_already_exists,
        ek_directory_creation_result_permission_denied,
        ek_directory_creation_result_path_not_found,
        ek_directory_creation_result_unknown_err
    };

    [[nodiscard]] t_b8 CreateDirectory(const s_str_rdonly path, s_mem_arena& temp_mem_arena, e_directory_creation_result* const o_creation_res = nullptr); // This DOES NOT create non-existent parent directories.
    [[nodiscard]] t_b8 CreateDirectoryAndParents(const s_str_rdonly path, s_mem_arena& temp_mem_arena, e_directory_creation_result* const o_dir_creation_res = nullptr);
    [[nodiscard]] t_b8 CreateFileAndParentDirs(const s_str_rdonly path, s_mem_arena& temp_mem_arena, e_directory_creation_result* const o_dir_creation_res = nullptr);

    enum e_path_type : t_s32 {
        ek_path_type_not_found,
        ek_path_type_file,
        ek_path_type_directory
    };

    [[nodiscard]] t_b8 CheckPathType(const s_str_rdonly path, s_mem_arena& temp_mem_arena, e_path_type& o_type);

    t_b8 Print(s_stream& stream, const s_str_rdonly str);

    template<typename tp_type> t_b8 PrintType(s_stream& stream, const tp_type& val);

    template<> inline t_b8 PrintType<s_str>(s_stream& stream, const s_str& val) {
        return Print(stream, val);
    }

    template<> inline t_b8 PrintType<s_str_rdonly>(s_stream& stream, const s_str_rdonly& val) {
        return Print(stream, val);
    }

    inline t_b8 PrintFmt(s_stream& stream, const s_str_rdonly fmt) {
        // Just print the rest of the string.
        return Print(stream, fmt);
    }

    // Use a single '%' as the format specifier - the type is inferred. To actually include a '%' in the output, write '%%'.
    // Returns true iff the operation was successful (this does not include the case of having too many arguments or too many format specifiers).
    template<typename tp_arg, typename... tp_args_leftover>
    t_b8 PrintFmt(s_stream& stream, const s_str_rdonly fmt, tp_arg arg, tp_args_leftover... args_leftover) {
        constexpr t_unicode_code_pt fmt_spec = '%';
        constexpr t_size fmt_spec_byte_cnt = UnicodeCodePointToByteCnt(fmt_spec);

        // Determine how many bytes to print.
        t_size byte_cnt = 0;
        t_b8 fmt_spec_found = false;

        ZF_WALK_STR(fmt, chr_info) {
            if (chr_info.code_pt == fmt_spec) {
                fmt_spec_found = true;
                break;
            }

            byte_cnt += UnicodeCodePointToByteCnt(chr_info.code_pt);
        }

        const t_b8 fmt_spec_is_duped = fmt_spec_found && byte_cnt < fmt.bytes.len && StrChrAtByte(fmt, byte_cnt + fmt_spec_byte_cnt) == fmt_spec;

        if (fmt_spec_is_duped) {
            // Actually include the format specifier in the print.
            byte_cnt += fmt_spec_byte_cnt;
        }

        // Print the bytes.
        const auto bytes_to_print = Slice(fmt.bytes, 0, byte_cnt);

        if (!StreamWriteItemsOfArray(stream, bytes_to_print)) {
            return false;
        }

        // Handle leftovers.
        if (fmt_spec_found) {
            const s_str_rdonly fmt_leftover = {Slice(fmt.bytes, byte_cnt + fmt_spec_byte_cnt, fmt.bytes.len)}; // The substring of everything after the format specifier.

            if (fmt_spec_is_duped) {
                return PrintFmt(stream, fmt_leftover, arg, args_leftover...);
            } else {
                if (!PrintType(stream, arg)) {
                    return false;
                }

                return PrintFmt(stream, fmt_leftover, args_leftover...);
            }
        }

        return true;
    }
}
