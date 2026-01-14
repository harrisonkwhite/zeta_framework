#pragma once

#include <zcl/zcl_basic.h>
#include <zcl/zcl_file_sys.h>
#include <zcl/zcl_math.h>

namespace zcl {
    // Type format structs which are to be accepted as format printing arguments need to meet this (i.e. have the tag).
    template <typename tp_type>
    concept c_format = requires { typename tp_type::t_formatting; };

    inline t_b8 Print(const t_stream stream, const t_str_rdonly str) {
        return stream_write_items_of_array(stream, str.bytes);
    }

    inline t_b8 PrintFormat(const t_stream stream, const t_str_rdonly format);

    template <typename tp_arg_type, typename... tp_arg_types_leftover>
    t_b8 PrintFormat(const t_stream stream, const t_str_rdonly format, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover);


    // ============================================================
    // @section: Bools

    struct t_format_bool {
        using t_formatting = void;

        t_b8 value;
    };

    inline t_format_bool FormatBool(const t_b8 value) { return {.value = value}; }

    template <typename tp_type>
        requires c_same<t_cvref_removed<tp_type>, t_b8>
    inline t_format_bool Format(const tp_type value) {
        return FormatBool(value);
    }

    t_b8 PrintType(const t_stream stream, const t_format_bool format);

    // ============================================================


    // ============================================================
    // @section: Strings

    struct t_format_str {
        using t_formatting = void;

        t_str_rdonly value;
    };

    inline t_format_str FormatStr(const t_str_rdonly value) { return {.value = value}; }
    inline t_format_str Format(const t_str_rdonly value) { return FormatStr(value); }

    t_b8 PrintType(const t_stream stream, const t_format_str format);

    // ============================================================


    // ============================================================
    // @section: Code Points

    struct t_format_code_pt {
        using t_formatting = void;

        t_code_pt value;
    };

    inline t_format_code_pt FormatCodePt(const t_code_pt value) { return {.value = value}; }
    inline t_format_code_pt Format(const t_code_pt value) { return FormatCodePt(value); }

    t_b8 PrintType(const t_stream stream, const t_format_code_pt format);

    // ============================================================


    // ============================================================
    // @section: Integrals

    template <c_integral tp_type>
    struct t_format_int {
        using t_formatting = void;

        tp_type value;
    };

    template <c_integral tp_type> t_format_int<tp_type> FormatInt(const tp_type value) { return {.value = value}; }
    template <c_integral tp_type> t_format_int<tp_type> Format(const tp_type value) { return FormatInt(value); }

    template <c_integral tp_type>
    t_b8 PrintType(const t_stream stream, const t_format_int<tp_type> format) {
        t_static_array<t_u8, 20> str_bytes = {}; // Maximum possible number of ASCII characters needed to represent a 64-bit integer.
        t_mem_stream str_bytes_stream = mem_stream_create(array_to_nonstatic(&str_bytes), ek_stream_mode_write);
        t_b8 str_bytes_stream_write_success = true;

        if (format.value < 0) {
            str_bytes_stream_write_success = stream_write_item(str_bytes_stream, '-');
            ZCL_ASSERT(str_bytes_stream_write_success);
        }

        const t_i32 dig_cnt = calc_digit_cnt(format.value);

        for (t_i32 i = 0; i < dig_cnt; i++) {
            const auto byte = static_cast<t_u8>('0' + calc_digit_at(format.value, dig_cnt - 1 - i));
            str_bytes_stream_write_success = stream_write_item(str_bytes_stream, byte);
            ZCL_ASSERT(str_bytes_stream_write_success);
        }

        return Print(stream, {mem_stream_get_bytes_written(&str_bytes_stream)});
    }

    // ============================================================


    // ============================================================
    // @section: Floats

    template <c_floating_point tp_type>
    struct t_format_float {
        using t_formatting = void;

        tp_type value;
        t_i32 precision;
        t_b8 trim_trailing_zeros;
    };

    template <c_floating_point tp_type>
    t_format_float<tp_type> FormatFloat(const tp_type value, const t_i32 precision = 6, const t_b8 trim_trailing_zeros = false) {
        return {
            .value = value,
            .precision = precision,
            .trim_trailing_zeros = trim_trailing_zeros,
        };
    }

    template <c_floating_point tp_type>
    t_format_float<tp_type> Format(const tp_type value) { return FormatFloat(value); }

    template <c_floating_point tp_type>
    t_b8 PrintType(const t_stream stream, const t_format_float<tp_type> format) {
        ZCL_ASSERT(format.precision > 0);

        t_static_array<t_u8, 400> str_bytes = {}; // Roughly more than how many bytes should ever be needed.

        t_i32 str_bytes_used = snprintf(reinterpret_cast<char *>(str_bytes.raw), str_bytes.k_len, "%.*f", format.precision, static_cast<t_f64>(format.value));

        if (str_bytes_used < 0 || str_bytes_used >= str_bytes.k_len) {
            return false;
        }

        if (format.trim_trailing_zeros) {
            const auto str_bytes_relevant = array_slice(array_to_nonstatic(&str_bytes), 0, str_bytes_used);

            if (array_check_any_equal(str_bytes_relevant, '.')) {
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

        return Print(stream, {array_slice(array_to_nonstatic(&str_bytes), 0, str_bytes_used)});
    }

    // ============================================================


    // ============================================================
    // @section: Hexadecimal

    enum t_format_hex_flags : t_i32 {
        ek_format_hex_flags_none = 0,
        ek_format_hex_flags_omit_prefix = 1 << 0,
        ek_format_hex_flags_lower_case = 1 << 1,
        ek_format_hex_flags_allow_odd_digit_cnt = 1 << 2
    };

    constexpr t_i32 k_format_hex_digit_cnt_min = 1;
    constexpr t_i32 k_format_hex_digit_cnt_max = 16;

    template <c_integral_unsigned tp_type>
    struct t_format_hex {
        using t_formatting = void;

        tp_type value;
        t_format_hex_flags flags;
        t_i32 min_digits; // Will be rounded UP to the next even if this is odd and the flag for allowing an odd digit count is unset.
    };

    template <c_integral_unsigned tp_type>
    t_format_hex<tp_type> FormatHex(const tp_type value, const t_format_hex_flags flags = {}, const t_i32 min_digits = k_format_hex_digit_cnt_min) {
        return {
            .value = value,
            .flags = flags,
            .min_digits = min_digits,
        };
    }

    inline t_format_hex<t_uintptr> FormatHex(const void *const ptr, const t_format_hex_flags flags = {}, const t_i32 min_digits = k_format_hex_digit_cnt_min) {
        return {
            .value = reinterpret_cast<t_uintptr>(ptr),
            .flags = flags,
            .min_digits = min_digits,
        };
    }

    inline t_format_hex<t_uintptr> Format(const void *const ptr) {
        return FormatHex(ptr, {}, 2 * ZCL_SIZE_OF(t_uintptr));
    }

    template <c_integral_unsigned tp_type>
    t_b8 PrintType(const t_stream stream, const t_format_hex<tp_type> format) {
        ZCL_ASSERT(format.min_digits >= k_format_hex_digit_cnt_min && format.min_digits <= k_format_hex_digit_cnt_max);

        t_static_array<t_u8, 2 + k_format_hex_digit_cnt_max> str_bytes = {}; // Can facilitate max number of digits plus the "0x" prefix.
        t_mem_stream str_bytes_stream = mem_stream_create(array_to_nonstatic(&str_bytes), ek_stream_mode_write);

        t_b8 str_bytes_stream_write_success = true;

        if (!(format.flags & ek_format_hex_flags_omit_prefix)) {
            str_bytes_stream_write_success = stream_write_item(str_bytes_stream, '0');
            ZCL_ASSERT(str_bytes_stream_write_success);

            str_bytes_stream_write_success = stream_write_item(str_bytes_stream, 'x');
            ZCL_ASSERT(str_bytes_stream_write_success);
        }

        const t_i32 str_bytes_digits_begin_pos = str_bytes_stream.byte_pos;

        const auto dig_to_byte = [flags = format.flags](const t_i32 dig) -> t_u8 {
            if (dig < 10) {
                return static_cast<t_u8>('0' + dig);
            } else {
                if (flags & ek_format_hex_flags_lower_case) {
                    return static_cast<t_u8>('a' + dig - 10);
                } else {
                    return static_cast<t_u8>('A' + dig - 10);
                }
            }
        };

        auto value_mut = format.value;

        t_i32 cnter = 0;
        const t_i32 inner_loop_cnt = (format.flags & ek_format_hex_flags_allow_odd_digit_cnt) ? 1 : 2;

        do {
            for (t_i32 i = 0; i < inner_loop_cnt; i++) {
                const auto byte = dig_to_byte(value_mut % 16);
                str_bytes_stream_write_success = stream_write_item(str_bytes_stream, byte);
                ZCL_ASSERT(str_bytes_stream_write_success);

                value_mut /= 16;

                cnter++;
            }
        } while (value_mut != 0 || cnter < format.min_digits);

        const auto str_bytes_digits = array_slice_from(mem_stream_get_bytes_written(&str_bytes_stream), str_bytes_digits_begin_pos);
        array_reverse(str_bytes_digits);

        return Print(stream, {mem_stream_get_bytes_written(&str_bytes_stream)});
    }

    // ============================================================


    // ============================================================
    // @section: V2s

    struct t_format_v2 {
        using t_formatting = void;

        t_v2 value;
        t_b8 trim_trailing_zeros;
    };

    struct t_format_v2_i {
        using t_formatting = void;

        t_v2_i value;
    };

    inline t_format_v2 FormatV2(const t_v2 value, const t_b8 trim_trailing_zeros = false) {
        return {.value = value, .trim_trailing_zeros = trim_trailing_zeros};
    }

    inline t_format_v2 Format(const t_v2 value) { return FormatV2(value); }

    t_b8 PrintType(const t_stream stream, const t_format_v2 format);

    inline t_format_v2_i FormatV2(const t_v2_i value) { return {.value = value}; }
    inline t_format_v2_i Format(const t_v2_i value) { return FormatV2(value); }

    t_b8 PrintType(const t_stream stream, const t_format_v2_i format);

    // ============================================================


    // ============================================================
    // @section: Arrays

    template <typename tp_arr_type>
    concept c_formattable_array = c_array<tp_arr_type>
        && requires(const typename tp_arr_type::t_elem &v) { { Format(v) } -> c_format; };

    template <c_formattable_array tp_arr_type>
    struct t_array_format {
        using t_formatting = void;

        tp_arr_type value;
        t_b8 one_per_line;
    };

    template <c_formattable_array tp_arr_type>
    t_array_format<tp_arr_type> FormatArray(const tp_arr_type value, const t_b8 one_per_line = false) {
        return {.value = value, .one_per_line = one_per_line};
    }

    template <c_formattable_array tp_arr_type>
    t_array_format<tp_arr_type> Format(const tp_arr_type value) { return FormatArray(value); }

    template <c_formattable_array tp_arr_type>
    t_b8 PrintType(const t_stream stream, const t_array_format<tp_arr_type> format) {
        if (format.one_per_line) {
            for (t_i32 i = 0; i < format.value.len; i++) {
                if (!PrintFormat(stream, ZCL_STR_LITERAL("[%] %%"), i, format.value[i], i < format.value.len - 1 ? ZCL_STR_LITERAL("\n") : ZCL_STR_LITERAL(""))) {
                    return false;
                }
            }
        } else {
            if (!Print(stream, ZCL_STR_LITERAL("["))) {
                return false;
            }

            for (t_i32 i = 0; i < format.value.len; i++) {
                if (!PrintFormat(stream, ZCL_STR_LITERAL("%"), format.value[i])) {
                    return false;
                }

                if (i < format.value.len - 1) {
                    if (!Print(stream, ZCL_STR_LITERAL(", "))) {
                        return false;
                    }
                }
            }

            if (!Print(stream, ZCL_STR_LITERAL("]"))) {
                return false;
            }
        }

        return true;
    }

    // ============================================================


    // ============================================================
    // @section: Bitsets

    enum t_format_bitset_style : t_i32 {
        ek_bitset_format_style_seq = 0,                // List all bits from LSB to MSB, not divided into bytes.
        ek_bitset_format_style_little_endian = 1 << 0, // Split into bytes, ordered in little endian.
        ek_bitset_format_style_big_endian = 1 << 1     // Split into bytes, ordered in big endian.
    };

    struct t_format_bitset {
        using t_formatting = void;

        t_bitset_rdonly value;
        t_format_bitset_style style;
    };

    inline t_format_bitset FormatBitset(const t_bitset_rdonly value, const t_format_bitset_style style = ek_bitset_format_style_seq) {
        return {.value = value, .style = style};
    }

    inline t_format_bitset Format(const t_bitset_rdonly value) {
        return FormatBitset(value, ek_bitset_format_style_seq);
    }

    t_b8 PrintType(const t_stream stream, const t_format_bitset format);

    // ============================================================


    constexpr t_code_pt k_print_format_spec = '%';
    constexpr t_code_pt k_print_format_esc = '^';

    t_i32 PrintFormatCountSpecs(const t_str_rdonly str);

    inline t_b8 PrintFormat(const t_stream stream, const t_str_rdonly format) {
        ZCL_ASSERT(PrintFormatCountSpecs(format) == 0);

        // Just print the rest of the string.
        return Print(stream, format);
    }

    // Use a single '%' as the format specifier. To actually include a '%' in the output, write "^%". To actually include a '^', write "^^".
    // Returns true iff the operation was successful.
    template <typename tp_arg_type, typename... tp_arg_types_leftover>
    t_b8 PrintFormat(const t_stream stream, const t_str_rdonly format, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover) {
        static_assert(!c_cstr<tp_arg_type>, "C-strings are prohibited for default formatting as a form of error prevention.");

        ZCL_ASSERT(PrintFormatCountSpecs(format) == 1 + sizeof...(args_leftover));

        static_assert(code_pt_check_ascii(k_print_format_spec) && code_pt_check_ascii(k_print_format_esc)); // Assuming this for this algorithm.

        t_b8 escaped = false;

        for (t_i32 i = 0; i < format.bytes.len; i++) {
            if (!escaped) {
                if (format.bytes[i] == k_print_format_esc) {
                    escaped = true;
                    continue;
                } else if (format.bytes[i] == k_print_format_spec) {
                    if constexpr (c_format<tp_arg_type>) {
                        if (!PrintType(stream, arg)) {
                            return false;
                        }
                    } else {
                        if (!PrintType(stream, Format(arg))) {
                            return false;
                        }
                    }

                    const t_str_rdonly format_leftover = {array_slice(format.bytes, i + 1, format.bytes.len)}; // The substring of everything after the format specifier.
                    return PrintFormat(stream, format_leftover, args_leftover...);
                }
            }

            if (!stream_write_item(stream, format.bytes[i])) {
                return false;
            }

            escaped = false;
        }

        return true;
    }


    // ============================================================
    // @section: Logging Helpers

    template <typename... tp_arg_types>
    t_b8 Log(const t_str_rdonly format, const tp_arg_types &...args) {
        t_file_stream std_err = file_stream_create_std_out();

        if (!PrintFormat(std_err, format, args...)) {
            return false;
        }

        if (!Print(std_err, ZCL_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    template <typename... tp_arg_types>
    t_b8 LogError(const t_str_rdonly format, const tp_arg_types &...args) {
        t_file_stream std_err = file_stream_create_std_error();

        if (!Print(std_err, ZCL_STR_LITERAL("Error: "))) {
            return false;
        }

        if (!PrintFormat(std_err, format, args...)) {
            return false;
        }

        if (!Print(std_err, ZCL_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    template <typename... tp_arg_types>
    t_b8 LogErrorType(const t_str_rdonly type_name, const t_str_rdonly format, const tp_arg_types &...args) {
        ZCL_ASSERT(!str_check_empty(type_name));

        t_file_stream std_err = file_stream_create_std_error();

        if (!PrintFormat(std_err, ZCL_STR_LITERAL("% Error: "), type_name)) {
            return false;
        }

        if (!PrintFormat(std_err, format, args...)) {
            return false;
        }

        if (!Print(std_err, ZCL_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    template <typename... tp_arg_types>
    t_b8 LogWarning(const t_str_rdonly format, const tp_arg_types &...args) {
        t_file_stream std_err = file_stream_create_std_error();

        if (!Print(std_err, ZCL_STR_LITERAL("Warning: "))) {
            return false;
        }

        if (!PrintFormat(std_err, format, args...)) {
            return false;
        }

        if (!Print(std_err, ZCL_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    // ============================================================
}
