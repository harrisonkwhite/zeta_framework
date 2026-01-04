#pragma once

#include <cstdio>
#include <zcl/zcl_math.h>
#include <zcl/zcl_algos.h>
#include <zcl/zcl_strs.h>

namespace zf {
    // ============================================================
    // @section: Streams

    enum e_stream_type : I32 {
        ek_stream_type_invalid,
        ek_stream_type_mem,
        ek_stream_type_file
    };

    enum e_stream_mode : I32 {
        ek_stream_mode_read,
        ek_stream_mode_write
    };

    struct s_stream {
        e_stream_type type;

        union {
            struct {
                s_array_mut<U8> bytes;
                I32 byte_pos;
            } mem;

            struct {
                FILE *file;
            } file;
        } type_data;

        e_stream_mode mode;
    };

    inline s_stream CreateMemStream(const s_array_mut<U8> bytes, const e_stream_mode mode, const I32 pos = 0) {
        return {.type = ek_stream_type_mem, .type_data = {.mem = {.bytes = bytes, .byte_pos = pos}}, .mode = mode};
    }

    inline s_array_mut<U8> MemStreamBytesWritten(const s_stream *const stream) {
        ZF_ASSERT(stream->type == ek_stream_type_mem);
        return ArraySlice(stream->type_data.mem.bytes, 0, stream->type_data.mem.byte_pos);
    }

    inline s_stream CreateFileStream(FILE *const file, const e_stream_mode mode) {
        return {.type = ek_stream_type_file, .type_data = {.file = {.file = file}}, .mode = mode};
    }

    inline s_stream StdIn() { return CreateFileStream(stdin, ek_stream_mode_read); }
    inline s_stream StdOut() { return CreateFileStream(stdout, ek_stream_mode_write); }
    inline s_stream StdError() { return CreateFileStream(stderr, ek_stream_mode_write); }

    template <Simple tp_type>
    [[nodiscard]] B8 ReadItem(s_stream *const stream, tp_type *const o_item) {
        ZF_ASSERT(stream->mode == ek_stream_mode_read);

        const I32 size = ZF_SIZE_OF(tp_type);

        switch (stream->type) {
        case ek_stream_type_mem: {
            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = ArraySlice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            const auto dest = AsBytes(*o_item);
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

    template <Simple tp_type>
    [[nodiscard]] B8 WriteItem(s_stream *const stream, const tp_type &item) {
        ZF_ASSERT(stream->mode == ek_stream_mode_write);

        const I32 size = ZF_SIZE_OF(tp_type);

        switch (stream->type) {
        case ek_stream_type_mem: {
            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = AsBytes(item);
            const auto dest = ArraySlice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
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

    template <co_array_mut tp_type>
    [[nodiscard]] B8 ReadItemsIntoArray(s_stream *const stream, const tp_type arr, const I32 cnt) {
        ZF_ASSERT(stream->mode == ek_stream_mode_read);
        ZF_ASSERT(cnt >= 0 && cnt <= arr.len);

        if (cnt == 0) {
            return true;
        }

        switch (stream->type) {
        case ek_stream_type_mem: {
            const I32 size = ZF_SIZE_OF(arr[0]) * cnt;

            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = ArraySlice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            const auto dest = AsByteArray(arr);
            CopyAll(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ek_stream_type_file:
            return static_cast<I32>(fread(arr.raw, sizeof(arr[0]), static_cast<size_t>(cnt), stream->type_data.file.file)) == cnt;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <co_array tp_type>
    [[nodiscard]] B8 WriteItemsOfArray(s_stream *const stream, const tp_type arr) {
        ZF_ASSERT(stream->mode == ek_stream_mode_write);

        if (arr.len == 0) {
            return true;
        }

        switch (stream->type) {
        case ek_stream_type_mem: {
            const I32 size = ArraySizeInBytes(arr);

            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = AsByteArray(arr);
            const auto dest = ArraySlice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            CopyAll(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ek_stream_type_file:
            return static_cast<I32>(fwrite(arr.raw, sizeof(arr[0]), static_cast<size_t>(arr.len), stream->type_data.file.file)) == arr.len;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <co_array tp_type>
    [[nodiscard]] B8 SerializeArray(s_stream *const stream, const tp_type arr) {
        if (!WriteItem(stream, arr.len)) {
            return false;
        }

        if (!WriteItemsOfArray(stream, arr)) {
            return false;
        }

        return true;
    }

    template <Simple tp_type>
    [[nodiscard]] B8 DeserializeArray(s_stream *const stream, s_arena *const arr_arena, s_array_mut<tp_type> *const o_arr) {
        I32 len;

        if (!ReadItem(stream, &len)) {
            return false;
        }

        *o_arr = ArenaPushArray<tp_type>(arr_arena, len);

        if (!ReadItemsIntoArray(stream, *o_arr, len)) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline B8 SerializeBitVector(s_stream *const stream, const s_bit_vec_rdonly bv) {
        if (!WriteItem(stream, bv.bit_cnt)) {
            return false;
        }

        if (!WriteItemsOfArray(stream, bv.Bytes())) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline B8 DeserializeBitVector(s_stream *const stream, s_arena *const bv_arena, s_bit_vec_mut *const o_bv) {
        I32 bit_cnt;

        if (!ReadItem(stream, &bit_cnt)) {
            return false;
        }

        *o_bv = CreateBitVector(bit_cnt, bv_arena);

        if (!ReadItemsIntoArray(stream, o_bv->Bytes(), o_bv->Bytes().len)) {
            return false;
        }

        return true;
    }

    // ============================================================


    // ============================================================
    // @section: Files and Directories

    enum e_file_access_mode : I32 {
        ek_file_access_mode_read,
        ek_file_access_mode_write,
        ek_file_access_mode_append
    };

    [[nodiscard]] B8 FileOpen(const strs::StrRdonly file_path, const e_file_access_mode mode, s_arena *const temp_arena, s_stream *const o_stream);
    void FileClose(s_stream *const stream);
    I32 FileCalcSize(s_stream *const stream);
    [[nodiscard]] B8 LoadFileContents(const strs::StrRdonly file_path, s_arena *const contents_arena, s_arena *const temp_arena, s_array_mut<U8> *const o_contents, const B8 add_terminator = false);

    enum e_directory_creation_result : I32 {
        ek_directory_creation_result_success,
        ek_directory_creation_result_already_exists,
        ek_directory_creation_result_permission_denied,
        ek_directory_creation_result_path_not_found,
        ek_directory_creation_result_unknown_err
    };

    [[nodiscard]] B8 CreateDirectoryAssumingParentsExist(const strs::StrRdonly path, s_arena *const temp_arena, e_directory_creation_result *const o_creation_res = nullptr);
    [[nodiscard]] B8 CreateDirectoryAndParents(const strs::StrRdonly path, s_arena *const temp_arena, e_directory_creation_result *const o_dir_creation_res = nullptr);
    [[nodiscard]] B8 CreateFileAndParentDirectories(const strs::StrRdonly path, s_arena *const temp_arena, e_directory_creation_result *const o_dir_creation_res = nullptr);

    enum e_path_type : I32 {
        ek_path_type_not_found,
        ek_path_type_file,
        ek_path_type_directory
    };

    e_path_type DeterminePathType(const strs::StrRdonly path, s_arena *const temp_arena);

    strs::StrMut LoadExecutableDirectory(s_arena *const arena);

    // ============================================================


    // ============================================================
    // @section: Printing

    inline B8 Print(s_stream *const stream, const strs::StrRdonly str) {
        return WriteItemsOfArray(stream, str.bytes);
    }

    inline B8 PrintFormat(s_stream *const stream, const strs::StrRdonly fmt);

    template <Simple tp_arg_type, Simple... tp_arg_types_leftover>
    B8 PrintFormat(s_stream *const stream, const strs::StrRdonly fmt, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover);

    // Type format structs which are to be accepted as format printing arguments need to meet this (i.e. have the tag).
    template <typename tp_type>
    concept co_fmt = requires { typename tp_type::t_fmt_tag; };


    // ========================================
    // @subsection: Bool Printing

    struct s_bool_fmt {
        using t_fmt_tag = void;
        B8 value;
    };

    inline s_bool_fmt FormatBool(const B8 value) { return {value}; }
    inline s_bool_fmt FormatDefault(const B8 value) { return {value}; }

    inline B8 PrintType(s_stream *const stream, const s_bool_fmt fmt) {
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

    inline B8 PrintType(s_stream *const stream, const s_str_fmt fmt) {
        return Print(stream, fmt.value);
    }

    // @todo: Char printing too?

    // ========================================


    // ========================================
    // @subsection: Integer Printing

    template <Integral tp_type>
    struct s_integral_fmt {
        using t_fmt_tag = void;
        tp_type value;
    };

    template <Integral tp_type> s_integral_fmt<tp_type> FormatInt(const tp_type value) { return {value}; }
    template <Integral tp_type> s_integral_fmt<tp_type> FormatDefault(const tp_type value) { return FormatInt(value); }

    template <Integral tp_type>
    B8 PrintType(s_stream *const stream, const s_integral_fmt<tp_type> fmt) {
        s_static_array<U8, 20> str_bytes = {}; // Maximum possible number of ASCII characters needed to represent a 64-bit integer.
        s_stream str_bytes_stream = CreateMemStream(AsNonstatic(str_bytes), ek_stream_mode_write);
        B8 str_bytes_stream_write_success = true;

        if (fmt.value < 0) {
            str_bytes_stream_write_success = WriteItem(&str_bytes_stream, '-');
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        const I32 dig_cnt = CalcDigitCount(fmt.value);

        for (I32 i = 0; i < dig_cnt; i++) {
            const auto byte = static_cast<U8>('0' + DetermineDigitAt(fmt.value, dig_cnt - 1 - i));
            str_bytes_stream_write_success = WriteItem(&str_bytes_stream, byte);
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        return Print(stream, {MemStreamBytesWritten(&str_bytes_stream)});
    }

    // ========================================


    // ========================================
    // @subsection: Float Printing

    template <FloatingPoint tp_type>
    struct s_float_fmt {
        using t_fmt_tag = void;

        tp_type value;
        B8 trim_trailing_zeros;
    };

    template <FloatingPoint tp_type>
    s_float_fmt<tp_type> FormatFloat(const tp_type value, const B8 trim_trailing_zeros = false) {
        return {value, trim_trailing_zeros};
    }

    template <FloatingPoint tp_type> s_float_fmt<tp_type> FormatDefault(const tp_type value) {
        return FormatFloat(value);
    }

    template <FloatingPoint tp_type>
    B8 PrintType(s_stream *const stream, const s_float_fmt<tp_type> fmt) {
        s_static_array<U8, 400> str_bytes = {}; // Roughly more than how many bytes should ever be needed.

        I32 str_bytes_used = snprintf(reinterpret_cast<char *>(str_bytes.raw), str_bytes.g_len, "%f", static_cast<F64>(fmt.value));

        if (str_bytes_used < 0 || str_bytes_used >= str_bytes.g_len) {
            return false;
        }

        if (fmt.trim_trailing_zeros) {
            const auto str_bytes_relevant = ArraySlice(AsNonstatic(str_bytes), 0, str_bytes_used);

            if (DoAnyEqual(str_bytes_relevant, '.')) {
                for (I32 i = str_bytes_used - 1;; i--) {
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

        return Print(stream, {ArraySlice(AsNonstatic(str_bytes), 0, str_bytes_used)});
    }

    // ========================================


    // ========================================
    // @subsection: Hex Printing

    enum e_hex_fmt_flags : I32 {
        ek_hex_fmt_flags_none = 0,
        ek_hex_fmt_flags_omit_prefix = 1 << 0,
        ek_hex_fmt_flags_lower_case = 1 << 1,
        ek_hex_fmt_flags_allow_odd_digit_cnt = 1 << 2
    };

    constexpr I32 g_hex_fmt_digit_cnt_min = 1;
    constexpr I32 g_hex_fmt_digit_cnt_max = 16;

    template <IntegralUnsigned tp_type>
    struct s_hex_fmt {
        using t_fmt_tag = void;

        tp_type value;
        e_hex_fmt_flags flags;
        I32 min_digits; // Will be rounded UP to the next even if this is odd and the flag for allowing an odd digit count is unset.
    };

    template <IntegralUnsigned tp_type>
    s_hex_fmt<tp_type> FormatHex(const tp_type value, const e_hex_fmt_flags flags = {}, const I32 min_digits = g_hex_fmt_digit_cnt_min) {
        return {value, flags, min_digits};
    }

    inline s_hex_fmt<UIntPtr> FormatHex(const void *const ptr, const e_hex_fmt_flags flags = {}, const I32 min_digits = g_hex_fmt_digit_cnt_min) {
        return {reinterpret_cast<UIntPtr>(ptr), flags, min_digits};
    }

    inline s_hex_fmt<UIntPtr> FormatDefault(const void *const ptr) {
        return FormatHex(ptr, {}, 2 * ZF_SIZE_OF(UIntPtr));
    }

    template <IntegralUnsigned tp_type>
    B8 PrintType(s_stream *const stream, const s_hex_fmt<tp_type> fmt) {
        ZF_ASSERT(fmt.min_digits >= g_hex_fmt_digit_cnt_min && fmt.min_digits <= g_hex_fmt_digit_cnt_max);

        s_static_array<U8, 2 + g_hex_fmt_digit_cnt_max> str_bytes = {}; // Can facilitate max number of digits plus the "0x" prefix.
        s_stream str_bytes_stream = CreateMemStream(AsNonstatic(str_bytes), ek_stream_mode_write);

        B8 str_bytes_stream_write_success = true;

        if (!(fmt.flags & ek_hex_fmt_flags_omit_prefix)) {
            str_bytes_stream_write_success = WriteItem(&str_bytes_stream, '0');
            ZF_ASSERT(str_bytes_stream_write_success);

            str_bytes_stream_write_success = WriteItem(&str_bytes_stream, 'x');
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        const I32 str_bytes_digits_begin_pos = str_bytes_stream.type_data.mem.byte_pos;

        const auto dig_to_byte = [flags = fmt.flags](const I32 dig) -> U8 {
            if (dig < 10) {
                return static_cast<U8>('0' + dig);
            } else {
                if (flags & ek_hex_fmt_flags_lower_case) {
                    return static_cast<U8>('a' + dig - 10);
                } else {
                    return static_cast<U8>('A' + dig - 10);
                }
            }
        };

        auto value_mut = fmt.value;

        I32 cnter = 0;
        const I32 inner_loop_cnt = (fmt.flags & ek_hex_fmt_flags_allow_odd_digit_cnt) ? 1 : 2;

        do {
            for (I32 i = 0; i < inner_loop_cnt; i++) {
                const auto byte = dig_to_byte(value_mut % 16);
                str_bytes_stream_write_success = WriteItem(&str_bytes_stream, byte);
                ZF_ASSERT(str_bytes_stream_write_success);

                value_mut /= 16;

                cnter++;
            }
        } while (value_mut != 0 || cnter < fmt.min_digits);

        const auto str_bytes_digits = ArraySliceFrom(MemStreamBytesWritten(&str_bytes_stream), str_bytes_digits_begin_pos);
        Reverse(str_bytes_digits);

        return Print(stream, {MemStreamBytesWritten(&str_bytes_stream)});
    }

    // ========================================


    // ========================================
    // @subsection: V2 Printing

    struct s_v2_fmt {
        using t_fmt_tag = void;

        s_v2 value;
        B8 trim_trailing_zeros;
    };

    inline s_v2_fmt FormatV2(const s_v2 value, const B8 trim_trailing_zeros = false) { return {value, trim_trailing_zeros}; }
    inline s_v2_fmt FormatDefault(const s_v2 value) { return FormatV2(value); }

    inline B8 PrintType(s_stream *const stream, const s_v2_fmt fmt) {
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

    inline B8 PrintType(s_stream *const stream, const s_v2_i_fmt fmt) {
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
    concept co_formattable_array = co_array<tp_arr_type>
        && requires(const typename tp_arr_type::t_elem &v) { { FormatDefault(v) } -> co_fmt; };

    template <co_formattable_array tp_arr_type>
    struct s_array_fmt {
        using t_fmt_tag = void;

        tp_arr_type value;
        B8 one_per_line;
    };

    template <co_formattable_array tp_arr_type>
    s_array_fmt<tp_arr_type> FormatArray(const tp_arr_type value, const B8 one_per_line = false) {
        return {value, one_per_line};
    }

    template <co_formattable_array tp_arr_type>
    s_array_fmt<tp_arr_type> FormatDefault(const tp_arr_type value) {
        return {value};
    }

    template <co_formattable_array tp_arr_type>
    B8 PrintType(s_stream *const stream, const s_array_fmt<tp_arr_type> fmt) {
        if (fmt.one_per_line) {
            for (I32 i = 0; i < fmt.value.len; i++) {
                if (!PrintFormat(stream, ZF_STR_LITERAL("[%] %%"), i, fmt.value[i], i < fmt.value.len - 1 ? ZF_STR_LITERAL("\n") : ZF_STR_LITERAL(""))) {
                    return false;
                }
            }
        } else {
            if (!Print(stream, ZF_STR_LITERAL("["))) {
                return false;
            }

            for (I32 i = 0; i < fmt.value.len; i++) {
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

    enum e_bit_vec_fmt_style : I32 {
        ek_bit_vec_fmt_style_seq = 0,                // List all bits from LSB to MSB, not divided into bytes.
        ek_bit_vec_fmt_style_little_endian = 1 << 0, // Split into bytes, ordered in little endian.
        ek_bit_vec_fmt_style_big_endian = 1 << 1     // Split into bytes, ordered in big endian.
    };

    struct s_bit_vec_fmt {
        using t_fmt_tag = void;

        s_bit_vec_rdonly value;
        e_bit_vec_fmt_style style;
    };

    inline s_bit_vec_fmt FormatBitVec(const s_bit_vec_rdonly &value, const e_bit_vec_fmt_style style) { return {value, style}; }
    inline s_bit_vec_fmt FormatDefault(const s_bit_vec_rdonly &value) { return FormatBitVec(value, ek_bit_vec_fmt_style_seq); }

    inline B8 PrintType(s_stream *const stream, const s_bit_vec_fmt fmt) {
        const auto print_bit = [&](const I32 bit_index) {
            const strs::StrRdonly str = IsBitSet(fmt.value, bit_index) ? ZF_STR_LITERAL("1") : ZF_STR_LITERAL("0");
            return Print(stream, str);
        };

        const auto print_byte = [&](const I32 index) {
            const I32 bit_cnt = index == fmt.value.Bytes().len - 1 ? LastByteBitCount(fmt.value) : 8;

            for (I32 i = 7; i >= bit_cnt; i--) {
                Print(stream, ZF_STR_LITERAL("0"));
            }

            for (I32 i = bit_cnt - 1; i >= 0; i--) {
                print_bit((index * 8) + i);
            }
        };

        switch (fmt.style) {
        case ek_bit_vec_fmt_style_seq:
            for (I32 i = 0; i < fmt.value.bit_cnt; i++) {
                if (!print_bit(i)) {
                    return false;
                }
            }

            break;

        case ek_bit_vec_fmt_style_little_endian:
            for (I32 i = 0; i < fmt.value.Bytes().len; i++) {
                if (i > 0) {
                    Print(stream, ZF_STR_LITERAL(" "));
                }

                print_byte(i);
            }

            break;

        case ek_bit_vec_fmt_style_big_endian:
            for (I32 i = fmt.value.Bytes().len - 1; i >= 0; i--) {
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

    inline I32 CountFormatSpecifiers(const strs::StrRdonly str) {
        static_assert(strs::get_code_point_is_ascii(g_print_fmt_spec) && strs::get_code_point_is_ascii(g_print_fmt_esc)); // Assuming this for this algorithm.

        B8 escaped = false;
        I32 cnt = 0;

        for (I32 i = 0; i < str.bytes.len; i++) {
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

    inline B8 PrintFormat(s_stream *const stream, const strs::StrRdonly fmt) {
        ZF_ASSERT(CountFormatSpecifiers(fmt) == 0);

        // Just print the rest of the string.
        return Print(stream, fmt);
    }

    // Use a single '%' as the format specifier. To actually include a '%' in the output, write "^%". To actually include a '^', write "^^".
    // Returns true iff the operation was successful.
    template <Simple tp_arg_type, Simple... tp_arg_types_leftover>
    B8 PrintFormat(s_stream *const stream, const strs::StrRdonly fmt, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover) {
        static_assert(!co_cstr<tp_arg_type>, "C-strings are prohibited for default formatting as a form of error prevention.");

        ZF_ASSERT(CountFormatSpecifiers(fmt) == 1 + sizeof...(args_leftover));

        static_assert(strs::get_code_point_is_ascii(g_print_fmt_spec) && strs::get_code_point_is_ascii(g_print_fmt_esc)); // Assuming this for this algorithm.

        B8 escaped = false;

        for (I32 i = 0; i < fmt.bytes.len; i++) {
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

                    const strs::StrRdonly fmt_leftover = {ArraySlice(fmt.bytes, i + 1, fmt.bytes.len)}; // The substring of everything after the format specifier.
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

    template <Simple... tp_arg_types>
    B8 Log(const strs::StrRdonly fmt, const tp_arg_types &...args) {
        s_stream std_err = StdOut();

        if (!PrintFormat(&std_err, fmt, args...)) {
            return false;
        }

        if (!Print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    template <Simple... tp_arg_types>
    B8 LogError(const strs::StrRdonly fmt, const tp_arg_types &...args) {
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

    template <Simple... tp_arg_types>
    B8 LogErrorType(const strs::StrRdonly type_name, const strs::StrRdonly fmt, const tp_arg_types &...args) {
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

    template <Simple... tp_arg_types>
    B8 LogWarning(const strs::StrRdonly fmt, const tp_arg_types &...args) {
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
