#pragma once

#include <cstdio>
#include <zc/zc_strs.h>
#include <zc/zc_mem.h>
#include <zc/zc_math.h>

namespace zf {
    // ============================================================
    // @section: Streams
    // ============================================================
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
                return static_cast<t_size>(fread(ArrayRaw(arr), sizeof(arr[0]), static_cast<size_t>(cnt), stream.type_data.file.fs_raw)) == cnt;
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
                return static_cast<t_size>(fwrite(ArrayRaw(arr), sizeof(arr[0]), static_cast<size_t>(ArrayLen(arr)), stream.type_data.file.fs_raw)) == ArrayLen(arr);
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

    [[nodiscard]] inline t_b8 SerializeBitVec(s_stream& stream, const s_bit_vec_rdonly& bv) {
        if (!StreamWriteItem(stream, bv.bit_cnt)) {
            return false;
        }

        if (!StreamWriteItemsOfArray(stream, bv.bytes)) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 DeserializeBitVec(s_stream& stream, s_mem_arena& mem_arena, s_bit_vec& o_bv) {
        o_bv = {};

        if (!StreamReadItem(stream, o_bv.bit_cnt)) {
            return false;
        }

        if (o_bv.bit_cnt > 0) {
            if (!MakeArray(mem_arena, BitsToBytes(o_bv.bit_cnt), o_bv.bytes)) {
                return false;
            }

            if (!StreamReadItemsIntoArray(stream, o_bv.bytes, o_bv.bytes.len)) {
                return false;
            }
        }

        return true;
    }

    // ============================================================
    // @section: Files and Directories
    // ============================================================
    enum e_file_access_mode : t_s32 {
        ek_file_access_mode_read,
        ek_file_access_mode_write,
        ek_file_access_mode_append
    };

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

    // ============================================================
    // @section: Printing
    // ============================================================
    inline t_b8 Print(s_stream& stream, const s_str_rdonly str) {
        return StreamWriteItemsOfArray(stream, str.bytes);
    }

    // Type format structs which are to be accepted as format printing arguments need to meet this (i.e. have the tag).
    template<typename tp_type>
    concept c_fmt = requires {
        typename tp_type::t_fmt_tag;
    };

    // ========================================
    // @subsection: Bool Printing
    // ========================================
    struct s_bool_fmt {
        using t_fmt_tag = void;
        t_b8 val;
    };

    inline s_bool_fmt FormatBool(const t_b8 val) {
        return {val};
    }

    inline s_bool_fmt FormatDefault(const t_b8 val) {
        return FormatBool(val);
    }

    inline t_b8 PrintType(s_stream& stream, const s_bool_fmt& fmt) {
        return Print(stream, fmt.val ? StrFromRaw("true") : StrFromRaw("false"));
    }

    // ========================================
    // @subsection: String Printing
    // ========================================
    struct s_str_fmt {
        using t_fmt_tag = void;
        s_str_rdonly val;
    };

    inline s_str_fmt FormatStr(const s_str_rdonly val) {
        return {val};
    }

    inline s_str_fmt FormatDefault(const s_str_rdonly val) {
        return FormatStr(val);
    }

    inline t_b8 PrintType(s_stream& stream, const s_str_fmt& fmt) {
        return Print(stream, fmt.val);
    }

    // ========================================
    // @subsection: Integer Printing
    // ========================================
    template<c_integral tp_type>
    struct s_integral_fmt {
        using t_fmt_tag = void;
        tp_type val;
    };

    template<c_integral tp_type>
    s_integral_fmt<tp_type> FormatInt(const tp_type val) {
        return {val};
    }

    template<c_integral tp_type>
    s_integral_fmt<tp_type> FormatDefault(const tp_type val) {
        return FormatInt(val);
    }

    template<c_integral tp_type>
    t_b8 PrintType(s_stream& stream, const s_integral_fmt<tp_type>& fmt) {
        s_static_array<t_u8, 20> str_bytes = {}; // Maximum possible number of ASCII characters needed to represent a 64-bit integer.
        t_size str_bytes_used = 0;

        if (fmt.val < 0) {
            str_bytes[str_bytes_used] = '-';
            str_bytes_used++;
        }

        const tp_type dig_cnt = DigitCnt(fmt.val);

        for (t_size i = 0; i < dig_cnt; i++) {
            str_bytes[str_bytes_used + i] = '0' + DigitAt(fmt.val, dig_cnt - 1 - i);
        }

        str_bytes_used += dig_cnt;

        const s_str_rdonly str = {Slice(str_bytes, 0, str_bytes_used)};
        return Print(stream, str);
    }

    // ========================================
    // @subsection: Float Printing
    // ========================================
    template<c_floating_point tp_type>
    struct s_float_fmt {
        using t_fmt_tag = void;

        tp_type val;
        t_b8 trim_trailing_zeros;
    };

    template<c_floating_point tp_type>
    s_float_fmt<tp_type> FormatFloat(const tp_type val, const t_b8 trim_trailing_zeros = false) {
        return {val, trim_trailing_zeros};
    }

    template<c_floating_point tp_type>
    s_float_fmt<tp_type> FormatDefault(const tp_type val) {
        return FormatFloat(val);
    }

    template<c_floating_point tp_type>
    t_b8 PrintType(s_stream& stream, const s_float_fmt<tp_type>& fmt) {
        s_static_array<t_u8, 400> str_bytes = {}; // Roughly more than how many bytes should ever be needed.

        auto str_bytes_used = snprintf(reinterpret_cast<char*>(str_bytes.buf_raw), str_bytes.g_len, "%f", fmt.val);

        if (str_bytes_used < 0 || str_bytes_used >= str_bytes.g_len) {
            return false;
        }

        if (fmt.trim_trailing_zeros) {
            const auto str_bytes_relevant = Slice(str_bytes, 0, str_bytes_used);

            if (AreAnyEqualTo(str_bytes_relevant, '.')) {
                for (t_size i = str_bytes_used - 1; ; i--) {
                    if (str_bytes[i] == '0') {
                        str_bytes_used--;
                    } else if (str_bytes[i] == '.') {
                        str_bytes_used--;
                        break;
                    } else {
                        break;
                    }
                }
            }
        }

        const s_str_rdonly str = {Slice(str_bytes, 0, str_bytes_used)};
        return Print(stream, str);
    }

    // ========================================
    // @subsection: Hex Printing
    // ========================================
    template<c_unsigned_integral tp_type>
    struct s_hex_fmt {
        using t_fmt_tag = void;

        tp_type val;
        t_b8 omit_prefix;
    };

    template<c_unsigned_integral tp_type>
    s_hex_fmt<tp_type> FormatHex(const tp_type val, const t_b8 omit_prefix = false) {
        return {val, omit_prefix};
    }

    template<c_unsigned_integral tp_type>
    t_b8 PrintType(s_stream& stream, const s_hex_fmt<tp_type>& fmt) {
        s_static_array<t_u8, 18> str_bytes = {}; // Maximum possible number of ASCII characters needed for hex representation of 64-bit integer.
        t_size str_bytes_used = 0;

        if (!fmt.omit_prefix) {
            str_bytes[0] = '0';
            str_bytes[1] = 'x';
            str_bytes_used += 2;
        }

        const t_size str_bytes_digits_begin_index = str_bytes_used;

        auto val_mut = fmt.val;

        do {
            const auto dig = val_mut % 16;

            if (dig < 10) {
                str_bytes[str_bytes_used] = '0' + dig;
            } else {
                str_bytes[str_bytes_used] = 'A' + dig - 10;
            }

            str_bytes_used++;

            val_mut /= 16;
        } while (val_mut != 0);

        const auto str_bytes_digits = Slice(str_bytes, str_bytes_digits_begin_index, str_bytes_used);
        Reverse(str_bytes_digits);

        const s_str_rdonly str = {Slice(str_bytes, 0, str_bytes_used)};
        return Print(stream, str);
    }

    // ========================================
    // @subsection: Bit Vector Printing
    // ========================================
    enum e_bit_vec_fmt_style {
        ek_bit_vec_fmt_style_seq = 0, // List all bits from LSB to MSB, not divided into bytes.
        ek_bit_vec_fmt_style_little_endian = 1 << 0, // Split into bytes, ordered in little endian.
        ek_bit_vec_fmt_style_big_endian = 1 << 1 // Split into bytes, ordered in big endian.
    };

    struct s_bit_vec_fmt {
        using t_fmt_tag = void;

        s_bit_vec_rdonly val;
        e_bit_vec_fmt_style style;
    };

    inline s_bit_vec_fmt FormatBitVec(const s_bit_vec_rdonly& val, const e_bit_vec_fmt_style style) {
        return {val, style};
    }

    inline s_bit_vec_fmt FormatDefault(const s_bit_vec_rdonly& val) {
        return FormatBitVec(val, ek_bit_vec_fmt_style_seq);
    }

    inline t_b8 PrintType(s_stream& stream, const s_bit_vec_fmt& fmt) {
        ZF_ASSERT(IsBitVecValid(fmt.val));

        const auto print_bit = [&](const t_size bit_index) {
            const s_str_rdonly str = IsBitSet(fmt.val, bit_index) ? "1" : "0";
            return Print(stream, str);
        };

        const auto print_byte = [&](const t_size index) {
            const t_size bit_cnt = index == fmt.val.bytes.len - 1 ? BitVecLastByteBitCnt(fmt.val) : 8;

            for (t_size i = 7; i >= bit_cnt; i--) {
                Print(stream, "0");
            }

            for (t_size i = bit_cnt - 1; i >= 0; i--) {
                print_bit((index * 8) + i);
            }
        };

        switch (fmt.style) {
            case ek_bit_vec_fmt_style_seq:
                for (t_size i = 0; i < fmt.val.bit_cnt; i++) {
                    if (!print_bit(i)) {
                        return false;
                    }
                }

                break;

            case ek_bit_vec_fmt_style_little_endian:
                for (t_size i = 0; i < fmt.val.bytes.len; i++) {
                    if (i > 0) {
                        Print(stream, " ");
                    }

                    print_byte(i);
                }

                break;

            case ek_bit_vec_fmt_style_big_endian:
                for (t_size i = fmt.val.bytes.len - 1; i >= 0; i--) {
                    print_byte(i);

                    if (i > 0) {
                        Print(stream, " ");
                    }
                }

                break;
        }

        return true;
    }

    // ========================================
    // @subsection: V2 Printing
    // ========================================
    template<c_integral tp_type>
    struct s_v2_int_fmt {
        using t_fmt_tag = void;
        s_v2<tp_type> val;
    };

    template<c_floating_point tp_type>
    struct s_v2_float_fmt {
        using t_fmt_tag = void;

        s_v2<tp_type> val;
        t_b8 trim_trailing_zeros;
    };

    template<c_integral tp_type>
    s_v2_int_fmt<tp_type> FormatV2(const s_v2<tp_type> val) {
        return {val};
    }

    template<c_integral tp_type>
    s_v2_int_fmt<tp_type> FormatDefault(const s_v2<tp_type> val) {
        return FormatV2(val);
    }

    template<c_floating_point tp_type>
    s_v2_float_fmt<tp_type> FormatV2(const s_v2<tp_type> val, const t_b8 trim_trailing_zeros = false) {
        return {val, trim_trailing_zeros};
    }

    template<c_floating_point tp_type>
    s_v2_float_fmt<tp_type> FormatDefault(const s_v2<tp_type> val) {
        return FormatV2(val);
    }

    template<c_integral tp_type>
    t_b8 PrintType(s_stream& stream, const s_v2_int_fmt<tp_type>& fmt) {
        return Print(stream, "(")
            && PrintType(stream, FormatInt(fmt.val.x))
            && Print(stream, ", ")
            && PrintType(stream, FormatInt(fmt.val.y))
            && Print(stream, ")");
    }

    template<c_floating_point tp_type>
    t_b8 PrintType(s_stream& stream, const s_v2_float_fmt<tp_type>& fmt) {
        return Print(stream, "(")
            && PrintType(stream, FormatFloat(fmt.val.x, fmt.trim_trailing_zeros))
            && Print(stream, ", ")
            && PrintType(stream, FormatFloat(fmt.val.y, fmt.trim_trailing_zeros))
            && Print(stream, ")");
    }

    // ========================================
    // @subsection: Format Printing
    // ========================================
    constexpr t_unicode_code_pt g_fmt_spec = '%';

    constexpr t_size CountFormatSpecifiers(const s_str_rdonly str) {
        ZF_ASSERT(IsValidUTF8Str(str));

        static_assert(IsASCII(g_fmt_spec)); // Assuming this for this algorithm.

        t_size cnt = 0;

        for (t_size i = 0; i < str.bytes.len; i++) {
            if (str.bytes[i] == g_fmt_spec) {
                if (i + 1 < str.bytes.len && str.bytes[i + 1] == g_fmt_spec) {
                    // There's a duplicate, so don't count it as an occurrence.
                    i++;
                } else {
                    cnt++;
                }
            }
        }

        return cnt;
    }

    inline t_b8 PrintFormat(s_stream& stream, const s_str_rdonly fmt) {
        ZF_ASSERT(CountFormatSpecifiers(fmt) == 0);

        // Just print the rest of the string.
        return Print(stream, fmt);
    }

    // Use a single '%' as the format specifier. To actually include a '%' in the output, write '%%'.
    // Returns true iff the operation was successful (does not include the case of having too many arguments or too many format specifiers).
    template<typename tp_arg_type, typename... tp_arg_types_leftover>
    t_b8 PrintFormat(s_stream& stream, const s_str_rdonly fmt, const tp_arg_type& arg, const tp_arg_types_leftover&... args_leftover) {
        ZF_ASSERT(CountFormatSpecifiers(fmt) == 1 + sizeof...(args_leftover));

        static_assert(IsASCII(g_fmt_spec)); // Assuming this for this algorithm.

        // Determine how many bytes to print.
        t_size byte_cnt = 0;
        t_b8 fmt_spec_found = false;

        while (byte_cnt < fmt.bytes.len) {
            if (fmt.bytes[byte_cnt] == g_fmt_spec) {
                fmt_spec_found = true;
                break; // We don't want to include the format specifier.
            }

            byte_cnt++;
        }

        const t_b8 fmt_spec_is_duped = fmt_spec_found && byte_cnt + 1 < fmt.bytes.len && fmt.bytes[byte_cnt + 1] == g_fmt_spec;

        if (fmt_spec_is_duped) {
            // Actually include the format specifier in the print.
            byte_cnt++;
        }

        // Print the bytes.
        const auto bytes_to_print = Slice(fmt.bytes, 0, byte_cnt);

        if (!StreamWriteItemsOfArray(stream, bytes_to_print)) {
            return false;
        }

        // Handle leftovers.
        if (fmt_spec_found) {
            const s_str_rdonly fmt_leftover = {Slice(fmt.bytes, byte_cnt + 1, fmt.bytes.len)}; // The substring of everything after the format specifier.

            if (fmt_spec_is_duped) {
                return PrintFormat(stream, fmt_leftover, arg, args_leftover...);
            } else {
                if constexpr (c_fmt<tp_arg_type>) {
                    if (!PrintType(stream, arg)) {
                        return false;
                    }
                } else {
                    if (!PrintType(stream, FormatDefault(arg))) {
                        return false;
                    }
                }

                return PrintFormat(stream, fmt_leftover, args_leftover...);
            }
        }

        return true;
    }

    // ========================================
    // @subsection: Logging Helpers
    // ========================================
    template<typename... tp_arg_types>
    t_b8 Log(const s_str_rdonly fmt, const tp_arg_types&... args) {
        s_stream std_err = StdOut();

        if (!PrintFormat(std_err, fmt, args...)) {
            return false;
        }

        if (!Print(std_err, "\n")) {
            return false;
        }

        return true;
    }

    template<typename... tp_arg_types>
    t_b8 LogError(const s_str_rdonly fmt, const tp_arg_types&... args) {
        s_stream std_err = StdError();

        if (!Print(std_err, "Error: ")) {
            return false;
        }

        if (!PrintFormat(std_err, fmt, args...)) {
            return false;
        }

        if (!Print(std_err, "\n")) {
            return false;
        }

        return true;
    }

    template<typename... tp_arg_types>
    t_b8 LogErrorType(const s_str_rdonly type_name, const s_str_rdonly fmt, const tp_arg_types&... args) {
        ZF_ASSERT(!IsStrEmpty(type_name));

        s_stream std_err = StdError();

        if (!PrintFormat(std_err, "% Error: ", type_name)) {
            return false;
        }

        if (!PrintFormat(std_err, fmt, args...)) {
            return false;
        }

        if (!Print(std_err, "\n")) {
            return false;
        }

        return true;
    }

    template<typename... tp_arg_types>
    t_b8 LogWarning(const s_str_rdonly fmt, const tp_arg_types&... args) {
        s_stream std_err = StdError();

        if (!Print(std_err, "Warning: ")) {
            return false;
        }

        if (!PrintFormat(std_err, fmt, args...)) {
            return false;
        }

        if (!Print(std_err, "\n")) {
            return false;
        }

        return true;
    }
}
