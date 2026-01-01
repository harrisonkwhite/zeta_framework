#include <zcl/zcl_strs.h>

namespace zf {
    enum e_utf8_byte_type : t_i32 {
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_4byte_start,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_invalid
    };

    static_assert(ek_utf8_byte_type_4byte_start - ek_utf8_byte_type_ascii + 1 == 4); // This is assumed in various algorithms.

    constexpr s_static_array<e_utf8_byte_type, 256> g_utf8_byte_type_table = {{
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

    t_b8 CalcIsStrValidUTF8(const s_str_rdonly str) {
        t_i32 cost = 0;

        for (t_i32 i = 0; i < str.bytes.len; i++) {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[i]];

            switch (byte_type) {
            case ek_utf8_byte_type_ascii:
            case ek_utf8_byte_type_2byte_start:
            case ek_utf8_byte_type_3byte_start:
            case ek_utf8_byte_type_4byte_start:
                if (cost > 0) {
                    return false;
                }

                cost = byte_type - ek_utf8_byte_type_ascii + 1;

                break;

            case ek_utf8_byte_type_continuation:
                if (cost == 0) {
                    return false;
                }

                break;

            case ek_utf8_byte_type_invalid:
                return false;
            }

            cost--;
        }

        return true;
    }

    t_i32 CalcStrLen(const s_str_rdonly str) {
        ZF_ASSERT(CalcIsStrValidUTF8(str));

        t_i32 i = 0;
        t_i32 len = 0;

        while (i < str.bytes.len) {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[i]];

            switch (byte_type) {
            case ek_utf8_byte_type_ascii:
            case ek_utf8_byte_type_2byte_start:
            case ek_utf8_byte_type_3byte_start:
            case ek_utf8_byte_type_4byte_start:
                i += byte_type - ek_utf8_byte_type_ascii + 1;
                break;

            default:
                ZF_UNREACHABLE();
            }

            len++;
        }

        return len;
    }

    static t_code_pt ConvertUTF8BytesToCodePoint(const s_array_rdonly<t_u8> bytes) {
        ZF_ASSERT(bytes.len >= 1 && bytes.len <= 4);

        t_code_pt result = 0;

        switch (bytes.len) {
        case 1:
            // 0xxxxxxx
            result |= bytes[0] & ByteBitmaskRanged(0, 7);
            break;

        case 2:
            // 110xxxxx 10xxxxxx
            result |= static_cast<t_code_pt>((bytes[0] & ByteBitmaskRanged(0, 5)) << 6);
            result |= bytes[1] & ByteBitmaskRanged(0, 6);
            break;

        case 3:
            // 1110xxxx 10xxxxxx 10xxxxxx
            result |= static_cast<t_code_pt>((bytes[0] & ByteBitmaskRanged(0, 4)) << 12);
            result |= static_cast<t_code_pt>((bytes[1] & ByteBitmaskRanged(0, 6)) << 6);
            result |= bytes[2] & ByteBitmaskRanged(0, 6);
            break;

        case 4:
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            result |= static_cast<t_code_pt>((bytes[0] & ByteBitmaskRanged(0, 3)) << 18);
            result |= static_cast<t_code_pt>((bytes[1] & ByteBitmaskRanged(0, 6)) << 12);
            result |= static_cast<t_code_pt>((bytes[2] & ByteBitmaskRanged(0, 6)) << 6);
            result |= bytes[3] & ByteBitmaskRanged(0, 6);
            break;
        }

        return result;
    }

    t_code_pt FindStrCodePointAtByte(const s_str_rdonly str, const t_i32 byte_index) {
        ZF_ASSERT(CalcIsStrValidUTF8(str));
        ZF_ASSERT(byte_index >= 0 && byte_index < str.bytes.len);

        t_i32 cp_first_byte_index = byte_index;

        do {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[cp_first_byte_index]];

            if (byte_type == ek_utf8_byte_type_continuation) {
                cp_first_byte_index--;
                continue;
            }

            const t_i32 cp_byte_cnt = byte_type - ek_utf8_byte_type_ascii + 1;
            const auto cp_bytes = Slice(str.bytes, byte_index, byte_index + cp_byte_cnt);
            return ConvertUTF8BytesToCodePoint(cp_bytes);
        } while (true);
    }

    void MarkStrCodePoints(const s_str_rdonly str, t_code_pt_bit_vec *const code_pts) {
        ZF_ASSERT(CalcIsStrValidUTF8(str));

        ZF_WALK_STR (str, info) {
            SetBit(*code_pts, info.code_pt); // @todo
        }
    }

    t_b8 WalkStr(const s_str_rdonly str, t_i32 *const byte_index, s_str_walk_info *const o_info) {
        ZF_ASSERT(CalcIsStrValidUTF8(str));
        ZF_ASSERT(*byte_index >= 0 && *byte_index <= str.bytes.len);

        if (*byte_index == str.bytes.len) {
            return false;
        }

        while (true) {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[*byte_index]];

            switch (byte_type) {
            case ek_utf8_byte_type_ascii:
            case ek_utf8_byte_type_2byte_start:
            case ek_utf8_byte_type_3byte_start:
            case ek_utf8_byte_type_4byte_start: {
                const t_i32 cp_byte_cnt = byte_type - ek_utf8_byte_type_ascii + 1;
                const auto cp_bytes = Slice(str.bytes, *byte_index, *byte_index + cp_byte_cnt);
                *o_info = {.code_pt = ConvertUTF8BytesToCodePoint(cp_bytes), .byte_index = *byte_index};
                *byte_index += cp_byte_cnt;

                return true;
            }

            case ek_utf8_byte_type_continuation:
                (*byte_index)--;
                break;

            default:
                ZF_UNREACHABLE();
            }
        }
    }

    t_b8 WalkStrReverse(const s_str_rdonly str, t_i32 *const byte_index, s_str_walk_info *const o_info) {
        ZF_ASSERT(CalcIsStrValidUTF8(str));
        ZF_ASSERT(*byte_index >= -1 && *byte_index < str.bytes.len);

        if (*byte_index == -1) {
            return false;
        }

        while (true) {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[*byte_index]];

            switch (byte_type) {
            case ek_utf8_byte_type_ascii:
            case ek_utf8_byte_type_2byte_start:
            case ek_utf8_byte_type_3byte_start:
            case ek_utf8_byte_type_4byte_start: {
                const t_i32 cp_byte_cnt = byte_type - ek_utf8_byte_type_ascii + 1;
                const auto cp_bytes = Slice(str.bytes, *byte_index, *byte_index + cp_byte_cnt);
                *o_info = {.code_pt = ConvertUTF8BytesToCodePoint(cp_bytes), .byte_index = *byte_index};
                (*byte_index)--;

                return true;
            }

            case ek_utf8_byte_type_continuation:
                (*byte_index)--;
                break;

            default:
                ZF_UNREACHABLE();
            }
        }
    }
}
