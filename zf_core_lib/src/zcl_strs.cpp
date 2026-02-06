#include <zcl/zcl_strs.h>

namespace zcl {
    enum t_utf8_byte_type : t_i32 {
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_4byte_start,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_invalid
    };

    static_assert(ek_utf8_byte_type_4byte_start - ek_utf8_byte_type_ascii + 1 == 4); // This is assumed in various algorithms.

    constexpr t_static_array<t_utf8_byte_type, 256> k_utf8_byte_type_table = {{
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_ascii,

        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_continuation,

        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_2byte_start,

        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_3byte_start,

        ek_utf8_byte_type_4byte_start,
        ek_utf8_byte_type_4byte_start,
        ek_utf8_byte_type_4byte_start,
        ek_utf8_byte_type_4byte_start,
        ek_utf8_byte_type_4byte_start,
        ek_utf8_byte_type_4byte_start,
        ek_utf8_byte_type_4byte_start,
        ek_utf8_byte_type_4byte_start,

        ek_utf8_byte_type_invalid,
        ek_utf8_byte_type_invalid,
        ek_utf8_byte_type_invalid,
        ek_utf8_byte_type_invalid,
        ek_utf8_byte_type_invalid,
        ek_utf8_byte_type_invalid,
        ek_utf8_byte_type_invalid,
        ek_utf8_byte_type_invalid,
    }};

    void CodePointToUTF8Bytes(const t_code_point code_pt, t_static_array<t_u8, 4> *const o_bytes, t_i32 *const o_byte_cnt) {
        *o_bytes = {};
        *o_byte_cnt = CodePointGetUTF8ByteCount(code_pt);

        switch (*o_byte_cnt) {
            case 1: {
                // 0xxxxxxx

                (*o_bytes)[0] |= code_pt & ByteBitmaskCreateRange(0, 7);

                break;
            }

            case 2: {
                // 110xxxxx 10xxxxxx

                (*o_bytes)[0] = 0b11000000;
                (*o_bytes)[0] |= (code_pt & (ByteBitmaskCreateRange(0, 5) << 6)) >> 6;

                (*o_bytes)[1] = 0b10000000;
                (*o_bytes)[1] |= code_pt & ByteBitmaskCreateRange(0, 6);

                break;
            }

            case 3: {
                // 1110xxxx 10xxxxxx 10xxxxxx

                (*o_bytes)[0] = 0b11100000;
                (*o_bytes)[0] |= (code_pt & (ByteBitmaskCreateRange(0, 4) << 12)) >> 12;

                (*o_bytes)[1] = 0b10000000;
                (*o_bytes)[1] |= (code_pt & (ByteBitmaskCreateRange(0, 6) << 6)) >> 6;

                (*o_bytes)[2] = 0b10000000;
                (*o_bytes)[2] |= code_pt & ByteBitmaskCreateRange(0, 6);

                break;
            }

            case 4: {
                // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

                (*o_bytes)[0] = 0b11110000;
                (*o_bytes)[0] |= (code_pt & (ByteBitmaskCreateRange(0, 3) << 18)) >> 18;

                (*o_bytes)[1] = 0b10000000;
                (*o_bytes)[1] |= (code_pt & (ByteBitmaskCreateRange(0, 6) << 12)) >> 12;

                (*o_bytes)[2] = 0b10000000;
                (*o_bytes)[2] |= (code_pt & (ByteBitmaskCreateRange(0, 6) << 6)) >> 6;

                (*o_bytes)[3] = 0b10000000;
                (*o_bytes)[3] |= code_pt & ByteBitmaskCreateRange(0, 6);

                break;
            }

            default: {
                ZCL_UNREACHABLE();
            }
        }
    }

    t_code_point UTF8BytesToCodePoint(const t_array_rdonly<t_u8> bytes) {
        ZCL_ASSERT(bytes.len >= 1 && bytes.len <= 4);

        t_code_point result = 0;

        switch (bytes.len) {
            case 1: {
                // 0xxxxxxx
                result |= bytes[0] & ByteBitmaskCreateRange(0, 7);
                break;
            }

            case 2: {
                // 110xxxxx 10xxxxxx
                result |= static_cast<t_code_point>((bytes[0] & ByteBitmaskCreateRange(0, 5)) << 6);
                result |= bytes[1] & ByteBitmaskCreateRange(0, 6);
                break;
            }

            case 3: {
                // 1110xxxx 10xxxxxx 10xxxxxx
                result |= static_cast<t_code_point>((bytes[0] & ByteBitmaskCreateRange(0, 4)) << 12);
                result |= static_cast<t_code_point>((bytes[1] & ByteBitmaskCreateRange(0, 6)) << 6);
                result |= bytes[2] & ByteBitmaskCreateRange(0, 6);
                break;
            }

            case 4: {
                // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                result |= static_cast<t_code_point>((bytes[0] & ByteBitmaskCreateRange(0, 3)) << 18);
                result |= static_cast<t_code_point>((bytes[1] & ByteBitmaskCreateRange(0, 6)) << 12);
                result |= static_cast<t_code_point>((bytes[2] & ByteBitmaskCreateRange(0, 6)) << 6);
                result |= bytes[3] & ByteBitmaskCreateRange(0, 6);
                break;
            }

            default: {
                ZCL_UNREACHABLE();
            }
        }

        return result;
    }

    t_b8 StrCheckValidUTF8(const t_str_rdonly str) {
        t_i32 cost = 0;

        for (t_i32 i = 0; i < str.bytes.len; i++) {
            const auto byte_type = k_utf8_byte_type_table[str.bytes[i]];

            switch (byte_type) {
                case ek_utf8_byte_type_ascii:
                case ek_utf8_byte_type_2byte_start:
                case ek_utf8_byte_type_3byte_start:
                case ek_utf8_byte_type_4byte_start: {
                    if (cost > 0) {
                        return false;
                    }

                    cost = byte_type - ek_utf8_byte_type_ascii + 1;

                    break;
                }

                case ek_utf8_byte_type_continuation: {
                    if (cost == 0) {
                        return false;
                    }

                    break;
                }

                case ek_utf8_byte_type_invalid: {
                    return false;
                }
            }

            cost--;
        }

        return true;
    }

    t_i32 StrCalcLen(const t_str_rdonly str) {
        ZCL_ASSERT(StrCheckValidUTF8(str));

        t_i32 i = 0;
        t_i32 len = 0;

        while (i < str.bytes.len) {
            const auto byte_type = k_utf8_byte_type_table[str.bytes[i]];

            switch (byte_type) {
                case ek_utf8_byte_type_ascii:
                case ek_utf8_byte_type_2byte_start:
                case ek_utf8_byte_type_3byte_start:
                case ek_utf8_byte_type_4byte_start: {
                    i += byte_type - ek_utf8_byte_type_ascii + 1;
                    break;
                }

                default: {
                    ZCL_UNREACHABLE();
                }
            }

            len++;
        }

        return len;
    }

    t_code_point StrFindCodePointAtByte(const t_str_rdonly str, const t_i32 byte_index) {
        ZCL_ASSERT(StrCheckValidUTF8(str));
        ZCL_ASSERT(byte_index >= 0 && byte_index < str.bytes.len);

        t_i32 cp_first_byte_index = byte_index;

        do {
            const auto byte_type = k_utf8_byte_type_table[str.bytes[cp_first_byte_index]];

            if (byte_type == ek_utf8_byte_type_continuation) {
                cp_first_byte_index--;
                continue;
            }

            const t_i32 cp_byte_cnt = byte_type - ek_utf8_byte_type_ascii + 1;
            const auto cp_bytes = ArraySlice(str.bytes, byte_index, byte_index + cp_byte_cnt);
            return UTF8BytesToCodePoint(cp_bytes);
        } while (true);
    }

    t_i32 StrCountCodePoint(const t_str_rdonly str, const t_code_point code_pt) {
        t_i32 result = 0;

        ZCL_STR_WALK (str, step) {
            if (step.code_pt == code_pt) {
                result++;
            }
        }

        return result;
    }

    t_array_mut<t_str_rdonly> StrSplit(const t_str_rdonly str, const t_code_point delimiter, t_arena *const arena) {
        ZCL_ASSERT(StrCheckValidUTF8(str));

        if (StrCheckEmpty(str)) {
            return {};
        }

        const t_i32 delimiter_utf8_byte_cnt = CodePointGetUTF8ByteCount(delimiter);

        const t_i32 split_cnt = 1 + StrCountCodePoint(str, delimiter);

        const auto result = ArenaPushArray<t_str_rdonly>(arena, split_cnt);

        t_i32 split_index = 0;
        t_i32 split_byte_index_begin = 0;
        t_i32 split_byte_index_end_excl;

        ZCL_STR_WALK (str, step) {
            if (step.code_pt == delimiter) {
                split_byte_index_end_excl = step.byte_index;

                result[split_index] = {
                    ArraySlice(str.bytes, split_byte_index_begin, split_byte_index_end_excl),
                };

                split_index++;
                split_byte_index_begin = step.byte_index + delimiter_utf8_byte_cnt;
            }
        }

        result[split_index] = {
            ArraySlice(str.bytes, split_byte_index_begin, str.bytes.len),
        };

        return result;
    }

    void StrMarkCodePoints(const t_str_rdonly str, t_code_point_bitset *const code_pts) {
        ZCL_ASSERT(StrCheckValidUTF8(str));

        ZCL_STR_WALK (str, step) {
            BitsetSet(*code_pts, static_cast<t_i32>(step.code_pt));
        }
    }

    t_b8 StrWalk(const t_str_rdonly str, t_i32 *const byte_index, t_str_walk_step *const o_step) {
        ZCL_ASSERT(StrCheckValidUTF8(str));
        ZCL_ASSERT(*byte_index >= 0 && *byte_index <= str.bytes.len);

        if (*byte_index == str.bytes.len) {
            return false;
        }

        while (true) {
            const auto byte_type = k_utf8_byte_type_table[str.bytes[*byte_index]];

            switch (byte_type) {
                case ek_utf8_byte_type_ascii:
                case ek_utf8_byte_type_2byte_start:
                case ek_utf8_byte_type_3byte_start:
                case ek_utf8_byte_type_4byte_start: {
                    const t_i32 cp_byte_cnt = byte_type - ek_utf8_byte_type_ascii + 1;
                    const auto cp_bytes = ArraySlice(str.bytes, *byte_index, *byte_index + cp_byte_cnt);
                    *o_step = {.code_pt = UTF8BytesToCodePoint(cp_bytes), .byte_index = *byte_index};
                    *byte_index += cp_byte_cnt;

                    return true;
                }

                case ek_utf8_byte_type_continuation: {
                    (*byte_index)--;
                    break;
                }

                default: {
                    ZCL_UNREACHABLE();
                }
            }
        }
    }

    t_b8 StrWalkReverse(const t_str_rdonly str, t_i32 *const byte_index, t_str_walk_step *const o_step) {
        ZCL_ASSERT(StrCheckValidUTF8(str));
        ZCL_ASSERT(*byte_index >= -1 && *byte_index < str.bytes.len);

        if (*byte_index == -1) {
            return false;
        }

        while (true) {
            const auto byte_type = k_utf8_byte_type_table[str.bytes[*byte_index]];

            switch (byte_type) {
                case ek_utf8_byte_type_ascii:
                case ek_utf8_byte_type_2byte_start:
                case ek_utf8_byte_type_3byte_start:
                case ek_utf8_byte_type_4byte_start: {
                    const t_i32 cp_byte_cnt = byte_type - ek_utf8_byte_type_ascii + 1;
                    const auto cp_bytes = ArraySlice(str.bytes, *byte_index, *byte_index + cp_byte_cnt);
                    *o_step = {.code_pt = UTF8BytesToCodePoint(cp_bytes), .byte_index = *byte_index};
                    (*byte_index)--;

                    return true;
                }

                case ek_utf8_byte_type_continuation: {
                    (*byte_index)--;
                    break;
                }

                default: {
                    ZCL_UNREACHABLE();
                }
            }
        }
    }
}
