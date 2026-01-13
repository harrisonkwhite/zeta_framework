#include <zcl/io/zcl_printing.h>

namespace zcl {
    t_b8 print_type(const t_stream stream, const t_bool_format format) {
        const t_str_rdonly true_str = ZCL_STR_LITERAL("true");
        const t_str_rdonly false_str = ZCL_STR_LITERAL("false");

        return print(stream, format.value ? true_str : false_str);
    }

    t_b8 print_type(const t_stream stream, const t_str_format format) {
        return print(stream, format.value);
    }

    t_b8 print_type(const t_stream stream, const t_code_pt_format format) {
        t_static_array<t_u8, 4> code_pt_bytes;
        t_i32 code_pt_byte_cnt;
        code_pt_to_utf8_bytes(format.value, &code_pt_bytes, &code_pt_byte_cnt);

        const t_str_rdonly code_pt_str = {array_slice(array_to_nonstatic(&code_pt_bytes), 0, code_pt_byte_cnt)};

        return print(stream, code_pt_str);
    }

    t_b8 print_type(const t_stream stream, const t_v2_format format) {
        return print(stream, ZCL_STR_LITERAL("("))
            && print_type(stream, format_float(format.value.x, format.trim_trailing_zeros))
            && print(stream, ZCL_STR_LITERAL(", "))
            && print_type(stream, format_float(format.value.y, format.trim_trailing_zeros))
            && print(stream, ZCL_STR_LITERAL(")"));
    }

    t_b8 print_type(const t_stream stream, const t_v2_i_format format) {
        return print(stream, ZCL_STR_LITERAL("("))
            && print_type(stream, format_int(format.value.x))
            && print(stream, ZCL_STR_LITERAL(", "))
            && print_type(stream, format_int(format.value.y))
            && print(stream, ZCL_STR_LITERAL(")"));
    }

    t_b8 print_type(const t_stream stream, const t_bitset_format format) {
        const auto print_bit = [&](const t_i32 bit_index) {
            const t_str_rdonly str = bitset_check_set(format.value, bit_index) ? ZCL_STR_LITERAL("1") : ZCL_STR_LITERAL("0");
            return print(stream, str);
        };

        const auto print_byte = [&](const t_i32 index) {
            const t_i32 bit_cnt = index == bitset_get_bytes(format.value).len - 1 ? bitset_get_last_byte_bit_cnt(format.value) : 8;

            for (t_i32 i = 7; i >= bit_cnt; i--) {
                print(stream, ZCL_STR_LITERAL("0"));
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
                    print(stream, ZCL_STR_LITERAL(" "));
                }

                print_byte(i);
            }

            break;

        case ek_bitset_format_style_big_endian:
            for (t_i32 i = bitset_get_bytes(format.value).len - 1; i >= 0; i--) {
                print_byte(i);

                if (i > 0) {
                    print(stream, ZCL_STR_LITERAL(" "));
                }
            }

            break;
        }

        return true;
    }

    t_i32 count_format_specs(const t_str_rdonly str) {
        static_assert(code_pt_check_ascii(k_print_format_spec) && code_pt_check_ascii(k_print_format_esc)); // Assuming this for this algorithm.

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
}
