#include <zcl/zcl_printing.h>

namespace zcl {
    t_b8 PrintType(const t_stream_view stream, const t_format_bool format) {
        const t_str_rdonly str_true = ZCL_STR_LITERAL("true");
        const t_str_rdonly str_false = ZCL_STR_LITERAL("false");

        return Print(stream, format.value ? str_true : str_false);
    }

    t_b8 PrintType(const t_stream_view stream, const t_format_str format) {
        return Print(stream, format.value);
    }

    t_b8 PrintType(const t_stream_view stream, const t_format_code_point format) {
        t_static_array<t_u8, 4> code_pt_bytes;
        t_i32 code_pt_byte_cnt;
        CodePointToUTF8Bytes(format.value, &code_pt_bytes, &code_pt_byte_cnt);

        const t_str_rdonly code_pt_str = {ArraySlice(ArrayToNonstatic(&code_pt_bytes), 0, code_pt_byte_cnt)};

        return Print(stream, code_pt_str);
    }

    t_b8 PrintType(const t_stream_view stream, const t_format_v2 format) {
        return Print(stream, ZCL_STR_LITERAL("("))
            && PrintType(stream, FormatFloat(format.value.x, format.precision, format.trim_trailing_zeros))
            && Print(stream, ZCL_STR_LITERAL(", "))
            && PrintType(stream, FormatFloat(format.value.y, format.precision, format.trim_trailing_zeros))
            && Print(stream, ZCL_STR_LITERAL(")"));
    }

    t_b8 PrintType(const t_stream_view stream, const t_format_v2_i format) {
        return Print(stream, ZCL_STR_LITERAL("("))
            && PrintType(stream, FormatInt(format.value.x))
            && Print(stream, ZCL_STR_LITERAL(", "))
            && PrintType(stream, FormatInt(format.value.y))
            && Print(stream, ZCL_STR_LITERAL(")"));
    }

    t_b8 PrintType(const t_stream_view stream, const t_format_bitset format) {
        const auto print_bit = [&](const t_i32 bit_index) {
            const t_str_rdonly str = BitsetCheckSet(format.value, bit_index) ? ZCL_STR_LITERAL("1") : ZCL_STR_LITERAL("0");
            return Print(stream, str);
        };

        const auto print_byte = [&](const t_i32 index) {
            const t_i32 bit_cnt = index == BitsetGetBytes(format.value).len - 1 ? BitsetGetLastByteBitCount(format.value) : 8;

            for (t_i32 i = 7; i >= bit_cnt; i--) {
                Print(stream, ZCL_STR_LITERAL("0"));
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
            for (t_i32 i = 0; i < BitsetGetBytes(format.value).len; i++) {
                if (i > 0) {
                    Print(stream, ZCL_STR_LITERAL(" "));
                }

                print_byte(i);
            }

            break;

        case ek_bitset_format_style_big_endian:
            for (t_i32 i = BitsetGetBytes(format.value).len - 1; i >= 0; i--) {
                print_byte(i);

                if (i > 0) {
                    Print(stream, ZCL_STR_LITERAL(" "));
                }
            }

            break;
        }

        return true;
    }

    t_i32 PrintFormatCountSpecs(const t_str_rdonly str) {
        static_assert(CodePointCheckASCII(k_print_format_spec) && CodePointCheckASCII(k_print_format_esc)); // Assuming this for this algorithm.

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
