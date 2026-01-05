#pragma once

#include <cstdio>
#include <zcl/zcl_math.h>
#include <zcl/zcl_algos.h>
#include <zcl/zcl_strs.h>

namespace zf {
    // ============================================================
    // @section: Streams

    enum e_stream_type : t_i32 {
        ek_stream_type_invalid,
        ek_stream_type_mem,
        ek_stream_type_file
    };

    enum e_stream_mode : t_i32 {
        ek_stream_mode_read,
        ek_stream_mode_write
    };

    struct s_stream {
        e_stream_type type;

        union {
            struct {
                t_array_mut<t_u8> bytes;
                t_i32 byte_pos;
            } mem;

            struct {
                FILE *file;
            } file;
        } type_data;

        e_stream_mode mode;
    };

    inline s_stream CreateMemStream(const t_array_mut<t_u8> bytes, const e_stream_mode mode, const t_i32 pos = 0) {
        return {.type = ek_stream_type_mem, .type_data = {.mem = {.bytes = bytes, .byte_pos = pos}}, .mode = mode};
    }

    inline t_array_mut<t_u8> MemStreamBytesWritten(const s_stream *const stream) {
        ZF_ASSERT(stream->type == ek_stream_type_mem);
        return f_mem_slice_array(stream->type_data.mem.bytes, 0, stream->type_data.mem.byte_pos);
    }

    inline s_stream CreateFileStream(FILE *const file, const e_stream_mode mode) {
        return {.type = ek_stream_type_file, .type_data = {.file = {.file = file}}, .mode = mode};
    }

    inline s_stream StdIn() { return CreateFileStream(stdin, ek_stream_mode_read); }
    inline s_stream StdOut() { return CreateFileStream(stdout, ek_stream_mode_write); }
    inline s_stream StdError() { return CreateFileStream(stderr, ek_stream_mode_write); }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 ReadItem(s_stream *const stream, tp_type *const o_item) {
        ZF_ASSERT(stream->mode == ek_stream_mode_read);

        const t_i32 size = ZF_SIZE_OF(tp_type);

        switch (stream->type) {
        case ek_stream_type_mem: {
            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = f_mem_slice_array(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            const auto dest = f_mem_as_bytes(*o_item);
            CopyAll(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ek_stream_type_file:
            return fread(o_item, size, 1, stream->type_data.file.file) == 1;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 WriteItem(s_stream *const stream, const tp_type &item) {
        ZF_ASSERT(stream->mode == ek_stream_mode_write);

        const t_i32 size = ZF_SIZE_OF(tp_type);

        switch (stream->type) {
        case ek_stream_type_mem: {
            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = f_mem_as_bytes(item);
            const auto dest = f_mem_slice_array(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            CopyAll(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ek_stream_type_file:
            return fwrite(&item, size, 1, stream->type_data.file.file) == 1;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <c_array_mut tp_type>
    [[nodiscard]] t_b8 ReadItemsIntoArray(s_stream *const stream, const tp_type arr, const t_i32 cnt) {
        ZF_ASSERT(stream->mode == ek_stream_mode_read);
        ZF_ASSERT(cnt >= 0 && cnt <= arr.len);

        if (cnt == 0) {
            return true;
        }

        switch (stream->type) {
        case ek_stream_type_mem: {
            const t_i32 size = ZF_SIZE_OF(arr[0]) * cnt;

            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = f_mem_slice_array(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            const auto dest = f_mem_array_as_byte_array(arr);
            CopyAll(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ek_stream_type_file:
            return static_cast<t_i32>(fread(arr.raw, sizeof(arr[0]), static_cast<size_t>(cnt), stream->type_data.file.file)) == cnt;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <c_array tp_type>
    [[nodiscard]] t_b8 WriteItemsOfArray(s_stream *const stream, const tp_type arr) {
        ZF_ASSERT(stream->mode == ek_stream_mode_write);

        if (arr.len == 0) {
            return true;
        }

        switch (stream->type) {
        case ek_stream_type_mem: {
            const t_i32 size = f_mem_array_size_in_bytes(arr);

            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = f_mem_array_as_byte_array(arr);
            const auto dest = f_mem_slice_array(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            CopyAll(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ek_stream_type_file:
            return static_cast<t_i32>(fwrite(arr.raw, sizeof(arr[0]), static_cast<size_t>(arr.len), stream->type_data.file.file)) == arr.len;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <c_array tp_type>
    [[nodiscard]] t_b8 SerializeArray(s_stream *const stream, const tp_type arr) {
        if (!WriteItem(stream, arr.len)) {
            return false;
        }

        if (!WriteItemsOfArray(stream, arr)) {
            return false;
        }

        return true;
    }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 DeserializeArray(s_stream *const stream, t_arena *const arr_arena, t_array_mut<tp_type> *const o_arr) {
        t_i32 len;

        if (!ReadItem(stream, &len)) {
            return false;
        }

        *o_arr = f_mem_push_array<tp_type>(arr_arena, len);

        if (!ReadItemsIntoArray(stream, *o_arr, len)) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 SerializeBitVector(s_stream *const stream, const t_bit_vec_rdonly bv) {
        if (!WriteItem(stream, bv.bit_cnt)) {
            return false;
        }

        if (!WriteItemsOfArray(stream, f_mem_bit_vector_bytes(bv))) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 DeserializeBitVector(s_stream *const stream, t_arena *const bv_arena, t_bit_vec_mut *const o_bv) {
        t_i32 bit_cnt;

        if (!ReadItem(stream, &bit_cnt)) {
            return false;
        }

        *o_bv = f_mem_create_bit_vector(bit_cnt, bv_arena);

        if (!ReadItemsIntoArray(stream, f_mem_bit_vector_bytes(*o_bv), f_mem_bit_vector_bytes(*o_bv).len)) {
            return false;
        }

        return true;
    }

    // ============================================================


    // ============================================================
    // @section: Files and Directories

    enum e_file_access_mode : t_i32 {
        ek_file_access_mode_read,
        ek_file_access_mode_write,
        ek_file_access_mode_append
    };

    [[nodiscard]] t_b8 FileOpen(const strs::StrRdonly file_path, const e_file_access_mode mode, t_arena *const temp_arena, s_stream *const o_stream);
    void FileClose(s_stream *const stream);
    t_i32 FileCalcSize(s_stream *const stream);
    [[nodiscard]] t_b8 LoadFileContents(const strs::StrRdonly file_path, t_arena *const contents_arena, t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator = false);

    enum e_directory_creation_result : t_i32 {
        ek_directory_creation_result_success,
        ek_directory_creation_result_already_exists,
        ek_directory_creation_result_permission_denied,
        ek_directory_creation_result_path_not_found,
        ek_directory_creation_result_unknown_err
    };

    [[nodiscard]] t_b8 CreateDirectoryAssumingParentsExist(const strs::StrRdonly path, t_arena *const temp_arena, e_directory_creation_result *const o_creation_res = nullptr);
    [[nodiscard]] t_b8 CreateDirectoryAndParents(const strs::StrRdonly path, t_arena *const temp_arena, e_directory_creation_result *const o_dir_creation_res = nullptr);
    [[nodiscard]] t_b8 CreateFileAndParentDirectories(const strs::StrRdonly path, t_arena *const temp_arena, e_directory_creation_result *const o_dir_creation_res = nullptr);

    enum e_path_type : t_i32 {
        ek_path_type_not_found,
        ek_path_type_file,
        ek_path_type_directory
    };

    e_path_type DeterminePathType(const strs::StrRdonly path, t_arena *const temp_arena);

    strs::StrMut LoadExecutableDirectory(t_arena *const arena);

    // ============================================================


    // ============================================================
    // @section: Printing

    inline t_b8 Print(s_stream *const stream, const strs::StrRdonly str) {
        return WriteItemsOfArray(stream, str.bytes);
    }

    inline t_b8 PrintFormat(s_stream *const stream, const strs::StrRdonly fmt);

    template <c_simple tp_arg_type, c_simple... tp_arg_types_leftover>
    t_b8 PrintFormat(s_stream *const stream, const strs::StrRdonly fmt, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover);

    // Type format structs which are to be accepted as format printing arguments need to meet this (i.e. have the tag).
    template <typename tp_type>
    concept co_fmt = requires { typename tp_type::t_fmt_tag; };


    // ========================================
    // @subsection: Bool Printing

    struct s_bool_fmt {
        using t_fmt_tag = void;
        t_b8 value;
    };

    inline s_bool_fmt FormatBool(const t_b8 value) { return {value}; }
    inline s_bool_fmt FormatDefault(const t_b8 value) { return {value}; }

    inline t_b8 PrintType(s_stream *const stream, const s_bool_fmt fmt) {
        const strs::StrRdonly true_str = ZF_STR_LITERAL("true");
        const strs::StrRdonly false_str = ZF_STR_LITERAL("false");

        return Print(stream, fmt.value ? true_str : false_str);
    }

    // ========================================


    // ========================================
    // @subsection: String Printing

    struct s_str_fmt {
        using t_fmt_tag = void;
        strs::StrRdonly value;
    };

    inline s_str_fmt FormatStr(const strs::StrRdonly value) { return {value}; }
    inline s_str_fmt FormatDefault(const strs::StrRdonly value) { return FormatStr(value); }

    inline t_b8 PrintType(s_stream *const stream, const s_str_fmt fmt) {
        return Print(stream, fmt.value);
    }

    // @todo: Char printing too?

    // ========================================


    // ========================================
    // @subsection: Integer Printing

    template <c_integral tp_type>
    struct s_integral_fmt {
        using t_fmt_tag = void;
        tp_type value;
    };

    template <c_integral tp_type> s_integral_fmt<tp_type> FormatInt(const tp_type value) { return {value}; }
    template <c_integral tp_type> s_integral_fmt<tp_type> FormatDefault(const tp_type value) { return FormatInt(value); }

    template <c_integral tp_type>
    t_b8 PrintType(s_stream *const stream, const s_integral_fmt<tp_type> fmt) {
        t_static_array<t_u8, 20> str_bytes = {}; // Maximum possible number of ASCII characters needed to represent a 64-bit integer.
        s_stream str_bytes_stream = CreateMemStream(f_mem_as_nonstatic_array(str_bytes), ek_stream_mode_write);
        t_b8 str_bytes_stream_write_success = true;

        if (fmt.value < 0) {
            str_bytes_stream_write_success = WriteItem(&str_bytes_stream, '-');
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        const t_i32 dig_cnt = CalcDigitCount(fmt.value);

        for (t_i32 i = 0; i < dig_cnt; i++) {
            const auto byte = static_cast<t_u8>('0' + DetermineDigitAt(fmt.value, dig_cnt - 1 - i));
            str_bytes_stream_write_success = WriteItem(&str_bytes_stream, byte);
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        return Print(stream, {MemStreamBytesWritten(&str_bytes_stream)});
    }

    // ========================================


    // ========================================
    // @subsection: Float Printing

    template <c_floating_point tp_type>
    struct s_float_fmt {
        using t_fmt_tag = void;

        tp_type value;
        t_b8 trim_trailing_zeros;
    };

    template <c_floating_point tp_type>
    s_float_fmt<tp_type> FormatFloat(const tp_type value, const t_b8 trim_trailing_zeros = false) {
        return {value, trim_trailing_zeros};
    }

    template <c_floating_point tp_type> s_float_fmt<tp_type> FormatDefault(const tp_type value) {
        return FormatFloat(value);
    }

    template <c_floating_point tp_type>
    t_b8 PrintType(s_stream *const stream, const s_float_fmt<tp_type> fmt) {
        t_static_array<t_u8, 400> str_bytes = {}; // Roughly more than how many bytes should ever be needed.

        t_i32 str_bytes_used = snprintf(reinterpret_cast<char *>(str_bytes.raw), str_bytes.g_len, "%f", static_cast<t_f64>(fmt.value));

        if (str_bytes_used < 0 || str_bytes_used >= str_bytes.g_len) {
            return false;
        }

        if (fmt.trim_trailing_zeros) {
            const auto str_bytes_relevant = f_mem_slice_array(f_mem_as_nonstatic_array(str_bytes), 0, str_bytes_used);

            if (DoAnyEqual(str_bytes_relevant, '.')) {
                for (t_i32 i = str_bytes_used - 1;; i--) {
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

        return Print(stream, {f_mem_slice_array(f_mem_as_nonstatic_array(str_bytes), 0, str_bytes_used)});
    }

    // ========================================


    // ========================================
    // @subsection: Hex Printing

    enum e_hex_fmt_flags : t_i32 {
        ek_hex_fmt_flags_none = 0,
        ek_hex_fmt_flags_omit_prefix = 1 << 0,
        ek_hex_fmt_flags_lower_case = 1 << 1,
        ek_hex_fmt_flags_allow_odd_digit_cnt = 1 << 2
    };

    constexpr t_i32 g_hex_fmt_digit_cnt_min = 1;
    constexpr t_i32 g_hex_fmt_digit_cnt_max = 16;

    template <c_integral_unsigned tp_type>
    struct s_hex_fmt {
        using t_fmt_tag = void;

        tp_type value;
        e_hex_fmt_flags flags;
        t_i32 min_digits; // Will be rounded UP to the next even if this is odd and the flag for allowing an odd digit count is unset.
    };

    template <c_integral_unsigned tp_type>
    s_hex_fmt<tp_type> FormatHex(const tp_type value, const e_hex_fmt_flags flags = {}, const t_i32 min_digits = g_hex_fmt_digit_cnt_min) {
        return {value, flags, min_digits};
    }

    inline s_hex_fmt<t_uintptr> FormatHex(const void *const ptr, const e_hex_fmt_flags flags = {}, const t_i32 min_digits = g_hex_fmt_digit_cnt_min) {
        return {reinterpret_cast<t_uintptr>(ptr), flags, min_digits};
    }

    inline s_hex_fmt<t_uintptr> FormatDefault(const void *const ptr) {
        return FormatHex(ptr, {}, 2 * ZF_SIZE_OF(t_uintptr));
    }

    template <c_integral_unsigned tp_type>
    t_b8 PrintType(s_stream *const stream, const s_hex_fmt<tp_type> fmt) {
        ZF_ASSERT(fmt.min_digits >= g_hex_fmt_digit_cnt_min && fmt.min_digits <= g_hex_fmt_digit_cnt_max);

        t_static_array<t_u8, 2 + g_hex_fmt_digit_cnt_max> str_bytes = {}; // Can facilitate max number of digits plus the "0x" prefix.
        s_stream str_bytes_stream = CreateMemStream(f_mem_as_nonstatic_array(str_bytes), ek_stream_mode_write);

        t_b8 str_bytes_stream_write_success = true;

        if (!(fmt.flags & ek_hex_fmt_flags_omit_prefix)) {
            str_bytes_stream_write_success = WriteItem(&str_bytes_stream, '0');
            ZF_ASSERT(str_bytes_stream_write_success);

            str_bytes_stream_write_success = WriteItem(&str_bytes_stream, 'x');
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        const t_i32 str_bytes_digits_begin_pos = str_bytes_stream.type_data.mem.byte_pos;

        const auto dig_to_byte = [flags = fmt.flags](const t_i32 dig) -> t_u8 {
            if (dig < 10) {
                return static_cast<t_u8>('0' + dig);
            } else {
                if (flags & ek_hex_fmt_flags_lower_case) {
                    return static_cast<t_u8>('a' + dig - 10);
                } else {
                    return static_cast<t_u8>('A' + dig - 10);
                }
            }
        };

        auto value_mut = fmt.value;

        t_i32 cnter = 0;
        const t_i32 inner_loop_cnt = (fmt.flags & ek_hex_fmt_flags_allow_odd_digit_cnt) ? 1 : 2;

        do {
            for (t_i32 i = 0; i < inner_loop_cnt; i++) {
                const auto byte = dig_to_byte(value_mut % 16);
                str_bytes_stream_write_success = WriteItem(&str_bytes_stream, byte);
                ZF_ASSERT(str_bytes_stream_write_success);

                value_mut /= 16;

                cnter++;
            }
        } while (value_mut != 0 || cnter < fmt.min_digits);

        const auto str_bytes_digits = f_mem_slice_array_from(MemStreamBytesWritten(&str_bytes_stream), str_bytes_digits_begin_pos);
        Reverse(str_bytes_digits);

        return Print(stream, {MemStreamBytesWritten(&str_bytes_stream)});
    }

    // ========================================


    // ========================================
    // @subsection: V2 Printing

    struct s_v2_fmt {
        using t_fmt_tag = void;

        s_v2 value;
        t_b8 trim_trailing_zeros;
    };

    inline s_v2_fmt FormatV2(const s_v2 value, const t_b8 trim_trailing_zeros = false) { return {value, trim_trailing_zeros}; }
    inline s_v2_fmt FormatDefault(const s_v2 value) { return FormatV2(value); }

    inline t_b8 PrintType(s_stream *const stream, const s_v2_fmt fmt) {
        return Print(stream, ZF_STR_LITERAL("("))
            && PrintType(stream, FormatFloat(fmt.value.x, fmt.trim_trailing_zeros))
            && Print(stream, ZF_STR_LITERAL(", "))
            && PrintType(stream, FormatFloat(fmt.value.y, fmt.trim_trailing_zeros))
            && Print(stream, ZF_STR_LITERAL(")"));
    }

    struct s_v2_i_fmt {
        using t_fmt_tag = void;

        s_v2_i value;
    };

    inline s_v2_i_fmt FormatV2(const s_v2_i value) { return {value}; }
    inline s_v2_i_fmt FormatDefault(const s_v2_i value) { return FormatV2(value); }

    inline t_b8 PrintType(s_stream *const stream, const s_v2_i_fmt fmt) {
        return Print(stream, ZF_STR_LITERAL("("))
            && PrintType(stream, FormatInt(fmt.value.x))
            && Print(stream, ZF_STR_LITERAL(", "))
            && PrintType(stream, FormatInt(fmt.value.y))
            && Print(stream, ZF_STR_LITERAL(")"));
    }

    // ========================================


    // ========================================
    // @subsection: Array Printing

    template <typename tp_arr_type>
    concept co_formattable_array = c_array<tp_arr_type>
        && requires(const typename tp_arr_type::t_elem &v) { { FormatDefault(v) } -> co_fmt; };

    template <co_formattable_array tp_arr_type>
    struct s_array_fmt {
        using t_fmt_tag = void;

        tp_arr_type value;
        t_b8 one_per_line;
    };

    template <co_formattable_array tp_arr_type>
    s_array_fmt<tp_arr_type> FormatArray(const tp_arr_type value, const t_b8 one_per_line = false) {
        return {value, one_per_line};
    }

    template <co_formattable_array tp_arr_type>
    s_array_fmt<tp_arr_type> FormatDefault(const tp_arr_type value) {
        return {value};
    }

    template <co_formattable_array tp_arr_type>
    t_b8 PrintType(s_stream *const stream, const s_array_fmt<tp_arr_type> fmt) {
        if (fmt.one_per_line) {
            for (t_i32 i = 0; i < fmt.value.len; i++) {
                if (!PrintFormat(stream, ZF_STR_LITERAL("[%] %%"), i, fmt.value[i], i < fmt.value.len - 1 ? ZF_STR_LITERAL("\n") : ZF_STR_LITERAL(""))) {
                    return false;
                }
            }
        } else {
            if (!Print(stream, ZF_STR_LITERAL("["))) {
                return false;
            }

            for (t_i32 i = 0; i < fmt.value.len; i++) {
                if (!PrintFormat(stream, ZF_STR_LITERAL("%"), fmt.value[i])) {
                    return false;
                }

                if (i < fmt.value.len - 1) {
                    if (!Print(stream, ZF_STR_LITERAL(", "))) {
                        return false;
                    }
                }
            }

            if (!Print(stream, ZF_STR_LITERAL("]"))) {
                return false;
            }
        }

        return true;
    }

    // ========================================


    // ========================================
    // @subsection: Bit Vector Printing

    enum e_bit_vec_fmt_style : t_i32 {
        ek_bit_vec_fmt_style_seq = 0,                // List all bits from LSB to MSB, not divided into bytes.
        ek_bit_vec_fmt_style_little_endian = 1 << 0, // Split into bytes, ordered in little endian.
        ek_bit_vec_fmt_style_big_endian = 1 << 1     // Split into bytes, ordered in big endian.
    };

    struct s_bit_vec_fmt {
        using t_fmt_tag = void;

        t_bit_vec_rdonly value;
        e_bit_vec_fmt_style style;
    };

    inline s_bit_vec_fmt FormatBitVec(const t_bit_vec_rdonly &value, const e_bit_vec_fmt_style style) { return {value, style}; }
    inline s_bit_vec_fmt FormatDefault(const t_bit_vec_rdonly &value) { return FormatBitVec(value, ek_bit_vec_fmt_style_seq); }

    inline t_b8 PrintType(s_stream *const stream, const s_bit_vec_fmt fmt) {
        const auto print_bit = [&](const t_i32 bit_index) {
            const strs::StrRdonly str = f_mem_is_bit_set(fmt.value, bit_index) ? ZF_STR_LITERAL("1") : ZF_STR_LITERAL("0");
            return Print(stream, str);
        };

        const auto print_byte = [&](const t_i32 index) {
            const t_i32 bit_cnt = index == f_mem_bit_vector_bytes(fmt.value).len - 1 ? f_mem_bit_vector_last_byte_bit_cnt(fmt.value) : 8;

            for (t_i32 i = 7; i >= bit_cnt; i--) {
                Print(stream, ZF_STR_LITERAL("0"));
            }

            for (t_i32 i = bit_cnt - 1; i >= 0; i--) {
                print_bit((index * 8) + i);
            }
        };

        switch (fmt.style) {
        case ek_bit_vec_fmt_style_seq:
            for (t_i32 i = 0; i < fmt.value.bit_cnt; i++) {
                if (!print_bit(i)) {
                    return false;
                }
            }

            break;

        case ek_bit_vec_fmt_style_little_endian:
            for (t_i32 i = 0; i < f_mem_bit_vector_bytes(fmt.value).len; i++) {
                if (i > 0) {
                    Print(stream, ZF_STR_LITERAL(" "));
                }

                print_byte(i);
            }

            break;

        case ek_bit_vec_fmt_style_big_endian:
            for (t_i32 i = f_mem_bit_vector_bytes(fmt.value).len - 1; i >= 0; i--) {
                print_byte(i);

                if (i > 0) {
                    Print(stream, ZF_STR_LITERAL(" "));
                }
            }

            break;
        }

        return true;
    }

    // ========================================


    // ========================================
    // @subsection: Format Printing

    constexpr strs::CodePoint g_print_fmt_spec = '%';
    constexpr strs::CodePoint g_print_fmt_esc = '^';

    inline t_i32 CountFormatSpecifiers(const strs::StrRdonly str) {
        static_assert(strs::get_code_point_is_ascii(g_print_fmt_spec) && strs::get_code_point_is_ascii(g_print_fmt_esc)); // Assuming this for this algorithm.

        t_b8 escaped = false;
        t_i32 cnt = 0;

        for (t_i32 i = 0; i < str.bytes.len; i++) {
            if (!escaped) {
                if (str.bytes[i] == g_print_fmt_esc) {
                    escaped = true;
                } else if (str.bytes[i] == g_print_fmt_spec) {
                    cnt++;
                }
            } else {
                escaped = false;
            }
        }

        return cnt;
    }

    inline t_b8 PrintFormat(s_stream *const stream, const strs::StrRdonly fmt) {
        ZF_ASSERT(CountFormatSpecifiers(fmt) == 0);

        // Just print the rest of the string.
        return Print(stream, fmt);
    }

    // Use a single '%' as the format specifier. To actually include a '%' in the output, write "^%". To actually include a '^', write "^^".
    // Returns true iff the operation was successful.
    template <c_simple tp_arg_type, c_simple... tp_arg_types_leftover>
    t_b8 PrintFormat(s_stream *const stream, const strs::StrRdonly fmt, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover) {
        static_assert(!c_cstr<tp_arg_type>, "C-strings are prohibited for default formatting as a form of error prevention.");

        ZF_ASSERT(CountFormatSpecifiers(fmt) == 1 + sizeof...(args_leftover));

        static_assert(strs::get_code_point_is_ascii(g_print_fmt_spec) && strs::get_code_point_is_ascii(g_print_fmt_esc)); // Assuming this for this algorithm.

        t_b8 escaped = false;

        for (t_i32 i = 0; i < fmt.bytes.len; i++) {
            if (!escaped) {
                if (fmt.bytes[i] == g_print_fmt_esc) {
                    escaped = true;
                    continue;
                } else if (fmt.bytes[i] == g_print_fmt_spec) {
                    if constexpr (co_fmt<tp_arg_type>) {
                        if (!PrintType(stream, arg)) {
                            return false;
                        }
                    } else {
                        if (!PrintType(stream, FormatDefault(arg))) {
                            return false;
                        }
                    }

                    const strs::StrRdonly fmt_leftover = {f_mem_slice_array(fmt.bytes, i + 1, fmt.bytes.len)}; // The substring of everything after the format specifier.
                    return PrintFormat(stream, fmt_leftover, args_leftover...);
                }
            }

            if (!WriteItem(stream, fmt.bytes[i])) {
                return false;
            }

            escaped = false;
        }

        return true;
    }

    // ========================================


    // ========================================
    // @subsection: Logging Helpers

    template <c_simple... tp_arg_types>
    t_b8 Log(const strs::StrRdonly fmt, const tp_arg_types &...args) {
        s_stream std_err = StdOut();

        if (!PrintFormat(&std_err, fmt, args...)) {
            return false;
        }

        if (!Print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    template <c_simple... tp_arg_types>
    t_b8 LogError(const strs::StrRdonly fmt, const tp_arg_types &...args) {
        s_stream std_err = StdError();

        if (!Print(&std_err, ZF_STR_LITERAL("Error: "))) {
            return false;
        }

        if (!PrintFormat(&std_err, fmt, args...)) {
            return false;
        }

        if (!Print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    template <c_simple... tp_arg_types>
    t_b8 LogErrorType(const strs::StrRdonly type_name, const strs::StrRdonly fmt, const tp_arg_types &...args) {
        ZF_ASSERT(!strs::get_is_empty(type_name));

        s_stream std_err = StdError();

        if (!PrintFormat(&std_err, ZF_STR_LITERAL("% Error: "), type_name)) {
            return false;
        }

        if (!PrintFormat(&std_err, fmt, args...)) {
            return false;
        }

        if (!Print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    template <c_simple... tp_arg_types>
    t_b8 LogWarning(const strs::StrRdonly fmt, const tp_arg_types &...args) {
        s_stream std_err = StdError();

        if (!Print(&std_err, ZF_STR_LITERAL("Warning: "))) {
            return false;
        }

        if (!PrintFormat(&std_err, fmt, args...)) {
            return false;
        }

        if (!Print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    // ========================================


    // ============================================================
}
