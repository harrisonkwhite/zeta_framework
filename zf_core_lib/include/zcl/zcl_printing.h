#pragma once

#include <zcl/zcl_basic.h>
#include <zcl/zcl_file_sys.h>
#include <zcl/zcl_math.h>

namespace zcl::io {
    // ============================================================
    // @section: Types and Constants

    // Type format structs which are to be accepted as format printing arguments need to meet this (i.e. have the tag).
    template <typename tp_type>
    concept c_format = requires { typename tp_type::t_format_tag; };

    struct t_bool_format {
        using t_format_tag = void;

        t_b8 value;
    };

    struct t_str_format {
        using t_format_tag = void;

        strs::t_str_rdonly value;
    };

    struct t_code_pt_format {
        using t_format_tag = void;

        strs::t_code_pt value;
    };

    template <c_integral tp_type>
    struct t_integral_format {
        using t_format_tag = void;

        tp_type value;
    };

    template <c_floating_point tp_type>
    struct t_float_format {
        using t_format_tag = void;

        tp_type value;
        t_i32 precision;
        t_b8 trim_trailing_zeros;
    };

    enum t_hex_format_flags : t_i32 {
        ek_hex_format_flags_none = 0,
        ek_hex_format_flags_omit_prefix = 1 << 0,
        ek_hex_format_flags_lower_case = 1 << 1,
        ek_hex_format_flags_allow_odd_digit_cnt = 1 << 2
    };

    constexpr t_i32 k_hex_format_digit_cnt_min = 1;
    constexpr t_i32 k_hex_format_digit_cnt_max = 16;

    template <c_integral_unsigned tp_type>
    struct t_hex_format {
        using t_format_tag = void;

        tp_type value;
        t_hex_format_flags flags;
        t_i32 min_digits; // Will be rounded UP to the next even if this is odd and the flag for allowing an odd digit count is unset.
    };

    struct t_v2_format {
        using t_format_tag = void;

        t_v2 value;
        t_b8 trim_trailing_zeros;
    };

    struct t_v2_i_format {
        using t_format_tag = void;

        t_v2_i value;
    };

    template <typename tp_arr_type>
    concept c_formattable_array = c_array<tp_arr_type>
        && requires(const typename tp_arr_type::t_elem &v) { { format_default(v) } -> c_format; };

    template <c_formattable_array tp_arr_type>
    struct t_array_format {
        using t_format_tag = void;

        tp_arr_type value;
        t_b8 one_per_line;
    };

    enum t_bitset_format_style : t_i32 {
        ek_bitset_format_style_seq = 0,                // List all bits from LSB to MSB, not divided into bytes.
        ek_bitset_format_style_little_endian = 1 << 0, // Split into bytes, ordered in little endian.
        ek_bitset_format_style_big_endian = 1 << 1     // Split into bytes, ordered in big endian.
    };

    struct t_bitset_format {
        using t_format_tag = void;

        t_bitset_rdonly value;
        t_bitset_format_style style;
    };

    constexpr strs::t_code_pt k_print_format_spec = '%';
    constexpr strs::t_code_pt k_print_format_esc = '^';

    // ============================================================


    // ============================================================
    // @section: Functions

    inline t_b8 print(const t_stream stream, const strs::t_str_rdonly str) {
        return stream_write_items_of_array(stream, str.bytes);
    }

    inline t_b8 print_format(const t_stream stream, const strs::t_str_rdonly format);

    template <typename tp_arg_type, typename... tp_arg_types_leftover>
    t_b8 print_format(const t_stream stream, const strs::t_str_rdonly format, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover);

    inline t_bool_format format_bool(const t_b8 value) {
        return {.value = value};
    }

    template <typename tp_type>
        requires c_same<t_cvref_removed<tp_type>, t_b8>
    inline t_bool_format format_default(const tp_type value) {
        return format_bool(value);
    }

    inline t_b8 print_type(const t_stream stream, const t_bool_format format) {
        const strs::t_str_rdonly true_str = ZF_STR_LITERAL("true");
        const strs::t_str_rdonly false_str = ZF_STR_LITERAL("false");

        return print(stream, format.value ? true_str : false_str);
    }

    inline t_str_format format_str(const strs::t_str_rdonly value) { return {.value = value}; }
    inline t_str_format format_default(const strs::t_str_rdonly value) { return format_str(value); }

    inline t_b8 print_type(const t_stream stream, const t_str_format format) {
        return print(stream, format.value);
    }

    inline t_code_pt_format format_code_pt(const strs::t_code_pt value) { return {.value = value}; }
    inline t_code_pt_format format_default(const strs::t_code_pt value) { return format_code_pt(value); }

    inline t_b8 print_type(const t_stream stream, const t_code_pt_format format) {
        t_static_array<t_u8, 4> code_pt_bytes;
        t_i32 code_pt_byte_cnt;
        strs::code_pt_to_utf8_bytes(format.value, &code_pt_bytes, &code_pt_byte_cnt);

        const strs::t_str_rdonly code_pt_str = {array_slice(array_to_nonstatic(&code_pt_bytes), 0, code_pt_byte_cnt)};

        return print(stream, code_pt_str);
    }

    template <c_integral tp_type> t_integral_format<tp_type> format_int(const tp_type value) { return {.value = value}; }
    template <c_integral tp_type> t_integral_format<tp_type> format_default(const tp_type value) { return format_int(value); }

    template <c_integral tp_type>
    t_b8 print_type(const t_stream stream, const t_integral_format<tp_type> format) {
        t_static_array<t_u8, 20> str_bytes = {}; // Maximum possible number of ASCII characters needed to represent a 64-bit integer.
        t_mem_stream str_bytes_stream = mem_stream_create(array_to_nonstatic(&str_bytes), ek_stream_mode_write);
        t_b8 str_bytes_stream_write_success = true;

        if (format.value < 0) {
            str_bytes_stream_write_success = stream_write_item(str_bytes_stream, '-');
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        const t_i32 dig_cnt = calc_digit_cnt(format.value);

        for (t_i32 i = 0; i < dig_cnt; i++) {
            const auto byte = static_cast<t_u8>('0' + calc_digit_at(format.value, dig_cnt - 1 - i));
            str_bytes_stream_write_success = stream_write_item(str_bytes_stream, byte);
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        return print(stream, {mem_stream_get_bytes_written(&str_bytes_stream)});
    }

    template <c_floating_point tp_type>
    t_float_format<tp_type> format_float(const tp_type value, const t_i32 precision = 6, const t_b8 trim_trailing_zeros = false) {
        return {
            .value = value,
            .precision = precision,
            .trim_trailing_zeros = trim_trailing_zeros,
        };
    }

    template <c_floating_point tp_type>
    t_float_format<tp_type> format_default(const tp_type value) { return format_float(value); }

    template <c_floating_point tp_type>
    t_b8 print_type(const t_stream stream, const t_float_format<tp_type> format) {
        ZF_ASSERT(format.precision > 0);

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

        return print(stream, {array_slice(array_to_nonstatic(&str_bytes), 0, str_bytes_used)});
    }

    template <c_integral_unsigned tp_type>
    t_hex_format<tp_type> format_hex(const tp_type value, const t_hex_format_flags flags = {}, const t_i32 min_digits = k_hex_format_digit_cnt_min) {
        return {
            .value = value,
            .flags = flags,
            .min_digits = min_digits,
        };
    }

    inline t_hex_format<t_uintptr> format_hex(const void *const ptr, const t_hex_format_flags flags = {}, const t_i32 min_digits = k_hex_format_digit_cnt_min) {
        return {
            .value = reinterpret_cast<t_uintptr>(ptr),
            .flags = flags,
            .min_digits = min_digits,
        };
    }

    inline t_hex_format<t_uintptr> format_default(const void *const ptr) {
        return format_hex(ptr, {}, 2 * ZF_SIZE_OF(t_uintptr));
    }

    template <c_integral_unsigned tp_type>
    t_b8 print_type(const t_stream stream, const t_hex_format<tp_type> format) {
        ZF_ASSERT(format.min_digits >= k_hex_format_digit_cnt_min && format.min_digits <= k_hex_format_digit_cnt_max);

        t_static_array<t_u8, 2 + k_hex_format_digit_cnt_max> str_bytes = {}; // Can facilitate max number of digits plus the "0x" prefix.
        t_mem_stream str_bytes_stream = mem_stream_create(array_to_nonstatic(&str_bytes), ek_stream_mode_write);

        t_b8 str_bytes_stream_write_success = true;

        if (!(format.flags & ek_hex_format_flags_omit_prefix)) {
            str_bytes_stream_write_success = stream_write_item(str_bytes_stream, '0');
            ZF_ASSERT(str_bytes_stream_write_success);

            str_bytes_stream_write_success = stream_write_item(str_bytes_stream, 'x');
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        const t_i32 str_bytes_digits_begin_pos = str_bytes_stream.byte_pos;

        const auto dig_to_byte = [flags = format.flags](const t_i32 dig) -> t_u8 {
            if (dig < 10) {
                return static_cast<t_u8>('0' + dig);
            } else {
                if (flags & ek_hex_format_flags_lower_case) {
                    return static_cast<t_u8>('a' + dig - 10);
                } else {
                    return static_cast<t_u8>('A' + dig - 10);
                }
            }
        };

        auto value_mut = format.value;

        t_i32 cnter = 0;
        const t_i32 inner_loop_cnt = (format.flags & ek_hex_format_flags_allow_odd_digit_cnt) ? 1 : 2;

        do {
            for (t_i32 i = 0; i < inner_loop_cnt; i++) {
                const auto byte = dig_to_byte(value_mut % 16);
                str_bytes_stream_write_success = stream_write_item(str_bytes_stream, byte);
                ZF_ASSERT(str_bytes_stream_write_success);

                value_mut /= 16;

                cnter++;
            }
        } while (value_mut != 0 || cnter < format.min_digits);

        const auto str_bytes_digits = array_slice_from(mem_stream_get_bytes_written(&str_bytes_stream), str_bytes_digits_begin_pos);
        array_reverse(str_bytes_digits);

        return print(stream, {mem_stream_get_bytes_written(&str_bytes_stream)});
    }


    inline t_v2_format format_v2(const t_v2 value, const t_b8 trim_trailing_zeros = false) {
        return {.value = value, .trim_trailing_zeros = trim_trailing_zeros};
    }

    inline t_v2_format format_default(const t_v2 value) { return format_v2(value); }

    inline t_b8 print_type(const t_stream stream, const t_v2_format format) {
        return print(stream, ZF_STR_LITERAL("("))
            && print_type(stream, format_float(format.value.x, format.trim_trailing_zeros))
            && print(stream, ZF_STR_LITERAL(", "))
            && print_type(stream, format_float(format.value.y, format.trim_trailing_zeros))
            && print(stream, ZF_STR_LITERAL(")"));
    }


    inline t_v2_i_format format_v2(const t_v2_i value) { return {.value = value}; }
    inline t_v2_i_format format_default(const t_v2_i value) { return format_v2(value); }

    inline t_b8 print_type(const t_stream stream, const t_v2_i_format format) {
        return print(stream, ZF_STR_LITERAL("("))
            && print_type(stream, format_int(format.value.x))
            && print(stream, ZF_STR_LITERAL(", "))
            && print_type(stream, format_int(format.value.y))
            && print(stream, ZF_STR_LITERAL(")"));
    }

    template <c_formattable_array tp_arr_type>
    t_array_format<tp_arr_type> format_array(const tp_arr_type value, const t_b8 one_per_line = false) {
        return {.value = value, .one_per_line = one_per_line};
    }

    template <c_formattable_array tp_arr_type>
    t_array_format<tp_arr_type> format_default(const tp_arr_type value) { return format_array(value); }

    template <c_formattable_array tp_arr_type>
    t_b8 print_type(const t_stream stream, const t_array_format<tp_arr_type> format) {
        if (format.one_per_line) {
            for (t_i32 i = 0; i < format.value.len; i++) {
                if (!print_format(stream, ZF_STR_LITERAL("[%] %%"), i, format.value[i], i < format.value.len - 1 ? ZF_STR_LITERAL("\n") : ZF_STR_LITERAL(""))) {
                    return false;
                }
            }
        } else {
            if (!print(stream, ZF_STR_LITERAL("["))) {
                return false;
            }

            for (t_i32 i = 0; i < format.value.len; i++) {
                if (!print_format(stream, ZF_STR_LITERAL("%"), format.value[i])) {
                    return false;
                }

                if (i < format.value.len - 1) {
                    if (!print(stream, ZF_STR_LITERAL(", "))) {
                        return false;
                    }
                }
            }

            if (!print(stream, ZF_STR_LITERAL("]"))) {
                return false;
            }
        }

        return true;
    }

    inline t_bitset_format format_bitset(const t_bitset_rdonly &value, const t_bitset_format_style style) {
        return {.value = value, .style = style};
    }

    inline t_bitset_format format_default(const t_bitset_rdonly &value) { return format_bitset(value, ek_bitset_format_style_seq); }

    inline t_b8 print_type(const t_stream stream, const t_bitset_format format) {
        const auto print_bit = [&](const t_i32 bit_index) {
            const strs::t_str_rdonly str = check_set(format.value, bit_index) ? ZF_STR_LITERAL("1") : ZF_STR_LITERAL("0");
            return print(stream, str);
        };

        const auto print_byte = [&](const t_i32 index) {
            const t_i32 bit_cnt = index == bitset_get_bytes(format.value).len - 1 ? bitset_get_last_byte_bit_cnt(format.value) : 8;

            for (t_i32 i = 7; i >= bit_cnt; i--) {
                print(stream, ZF_STR_LITERAL("0"));
            }

            for (t_i32 i = bit_cnt - 1; i >= 0; i--) {
                print_bit((index * 8) + i);
            }
        };

        switch (format.style) {
        case ek_bitset_format_style_seq:
            for (t_i32 i = 0; i < format.value.bit_cnt; i++) {
                if (!print_bit(i)) {
                    return false;
                }
            }

            break;

        case ek_bitset_format_style_little_endian:
            for (t_i32 i = 0; i < bitset_get_bytes(format.value).len; i++) {
                if (i > 0) {
                    print(stream, ZF_STR_LITERAL(" "));
                }

                print_byte(i);
            }

            break;

        case ek_bitset_format_style_big_endian:
            for (t_i32 i = bitset_get_bytes(format.value).len - 1; i >= 0; i--) {
                print_byte(i);

                if (i > 0) {
                    print(stream, ZF_STR_LITERAL(" "));
                }
            }

            break;
        }

        return true;
    }

    inline t_i32 count_format_specs(const strs::t_str_rdonly str) {
        static_assert(strs::code_pt_check_ascii(k_print_format_spec) && strs::code_pt_check_ascii(k_print_format_esc)); // Assuming this for this algorithm.

        t_b8 escaped = false;
        t_i32 cnt = 0;

        for (t_i32 i = 0; i < str.bytes.len; i++) {
            if (!escaped) {
                if (str.bytes[i] == k_print_format_esc) {
                    escaped = true;
                } else if (str.bytes[i] == k_print_format_spec) {
                    cnt++;
                }
            } else {
                escaped = false;
            }
        }

        return cnt;
    }

    inline t_b8 print_format(const t_stream stream, const strs::t_str_rdonly format) {
        ZF_ASSERT(count_format_specs(format) == 0);

        // Just print the rest of the string.
        return print(stream, format);
    }

    // Use a single '%' as the format specifier. To actually include a '%' in the output, write "^%". To actually include a '^', write "^^".
    // Returns true iff the operation was successful.
    template <typename tp_arg_type, typename... tp_arg_types_leftover>
    t_b8 print_format(const t_stream stream, const strs::t_str_rdonly format, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover) {
        static_assert(!c_cstr<tp_arg_type>, "C-strings are prohibited for default formatting as a form of error prevention.");

        ZF_ASSERT(count_format_specs(format) == 1 + sizeof...(args_leftover));

        static_assert(strs::code_pt_check_ascii(k_print_format_spec) && strs::code_pt_check_ascii(k_print_format_esc)); // Assuming this for this algorithm.

        t_b8 escaped = false;

        for (t_i32 i = 0; i < format.bytes.len; i++) {
            if (!escaped) {
                if (format.bytes[i] == k_print_format_esc) {
                    escaped = true;
                    continue;
                } else if (format.bytes[i] == k_print_format_spec) {
                    if constexpr (c_format<tp_arg_type>) {
                        if (!print_type(stream, arg)) {
                            return false;
                        }
                    } else {
                        if (!print_type(stream, format_default(arg))) {
                            return false;
                        }
                    }

                    const strs::t_str_rdonly format_leftover = {array_slice(format.bytes, i + 1, format.bytes.len)}; // The substring of everything after the format specifier.
                    return print_format(stream, format_leftover, args_leftover...);
                }
            }

            if (!stream_write_item(stream, format.bytes[i])) {
                return false;
            }

            escaped = false;
        }

        return true;
    }

    template <typename... tp_arg_types>
    t_b8 log(const strs::t_str_rdonly format, const tp_arg_types &...args) {
#if 0
        file_sys::t_file_stream std_err = get_std_out();

        if (!print_format(&std_err, format, args...)) {
            return false;
        }

        if (!print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }
#endif

        return true;
    }

    template <typename... tp_arg_types>
    t_b8 log_error(const strs::t_str_rdonly format, const tp_arg_types &...args) {
#if 0
        t_stream std_err = get_std_error();

        if (!print(&std_err, ZF_STR_LITERAL("Error: "))) {
            return false;
        }

        if (!print_format(&std_err, format, args...)) {
            return false;
        }

        if (!print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }
#endif

        return true;
    }

    template <typename... tp_arg_types>
    t_b8 log_error_type(const strs::t_str_rdonly type_name, const strs::t_str_rdonly format, const tp_arg_types &...args) {
#if 0
        ZF_ASSERT(!strs::check_empty(type_name));

        t_stream std_err = get_std_error();

        if (!print_format(&std_err, ZF_STR_LITERAL("% Error: "), type_name)) {
            return false;
        }

        if (!print_format(&std_err, format, args...)) {
            return false;
        }

        if (!print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }
#endif

        return true;
    }

    template <typename... tp_arg_types>
    t_b8 log_warning(const strs::t_str_rdonly format, const tp_arg_types &...args) {
#if 0
        t_stream std_err = get_std_error();

        if (!print(&std_err, ZF_STR_LITERAL("Warning: "))) {
            return false;
        }

        if (!print_format(&std_err, format, args...)) {
            return false;
        }

        if (!print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }
#endif

        return true;
    }

    // ============================================================
}
