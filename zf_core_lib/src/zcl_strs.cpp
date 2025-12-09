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

    static_assert(ek_utf8_byte_type_4byte_start - ek_utf8_byte_type_ascii + 1 == 4);

    constexpr s_static_array<e_utf8_byte_type, 256> g_utf8_byte_type_table = {{ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii,
                                                                               ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii,
                                                                               ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii,
                                                                               ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii,
                                                                               ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_ascii, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation,
                                                                               ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation,
                                                                               ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation,
                                                                               ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_continuation, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start,
                                                                               ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_2byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start,
                                                                               ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_3byte_start, ek_utf8_byte_type_4byte_start, ek_utf8_byte_type_4byte_start, ek_utf8_byte_type_4byte_start, ek_utf8_byte_type_4byte_start, ek_utf8_byte_type_4byte_start, ek_utf8_byte_type_4byte_start, ek_utf8_byte_type_4byte_start, ek_utf8_byte_type_4byte_start, ek_utf8_byte_type_invalid, ek_utf8_byte_type_invalid, ek_utf8_byte_type_invalid, ek_utf8_byte_type_invalid, ek_utf8_byte_type_invalid, ek_utf8_byte_type_invalid, ek_utf8_byte_type_invalid, ek_utf8_byte_type_invalid}};

    t_b8 IsValidUTF8Str(const s_str_rdonly str) {
        t_len cost = 0;

        for (t_len i = 0; i < str.bytes.len; i++) {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[i]];

            switch (byte_type) {
            case ek_utf8_byte_type_ascii:
                if (!str.bytes[i]) {
                    return cost == 0;
                }

                [[fallthrough]];

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

        return cost == 0;
    }

    t_len CalcStrLen(const s_str_rdonly str) {
        ZF_ASSERT(IsValidUTF8Str(str));

        t_len i = 0;
        t_len len = 0;

        while (i < str.bytes.len) {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[i]];

            switch (byte_type) {
            case ek_utf8_byte_type_ascii:
                if (!str.bytes[i]) {
                    return len;
                }

                [[fallthrough]];

            case ek_utf8_byte_type_2byte_start:
            case ek_utf8_byte_type_3byte_start:
            case ek_utf8_byte_type_4byte_start:
                i += byte_type - ek_utf8_byte_type_ascii + 1;
                break;

            case ek_utf8_byte_type_continuation:
            case ek_utf8_byte_type_invalid:
                ZF_ASSERT(false);
                break;
            }

            len++;
        }

        return len;
    }

    static t_unicode_code_pt UTF8ChrBytesToCodePoint(const s_array_rdonly<char> bytes) {
        ZF_ASSERT(bytes.len >= 1 && bytes.len <= 4);
        ZF_ASSERT(IsValidUTF8Str({bytes}));

        t_unicode_code_pt res = 0;

        switch (bytes.len) {
        case 1:
            // 0xxxxxxx
            res |= bytes[0] & BitmaskRange(0, 7);
            break;

        case 2:
            // 110xxxxx 10xxxxxx
            res |= static_cast<t_unicode_code_pt>((bytes[0] & BitmaskRange(0, 5)) << 6);
            res |= bytes[1] & BitmaskRange(0, 6);
            break;

        case 3:
            // 1110xxxx 10xxxxxx 10xxxxxx
            res |= static_cast<t_unicode_code_pt>((bytes[0] & BitmaskRange(0, 4)) << 12);
            res |= static_cast<t_unicode_code_pt>((bytes[1] & BitmaskRange(0, 6)) << 6);
            res |= bytes[2] & BitmaskRange(0, 6);
            break;

        case 4:
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            res |= static_cast<t_unicode_code_pt>((bytes[0] & BitmaskRange(0, 3)) << 18);
            res |= static_cast<t_unicode_code_pt>((bytes[1] & BitmaskRange(0, 6)) << 12);
            res |= static_cast<t_unicode_code_pt>((bytes[2] & BitmaskRange(0, 6)) << 6);
            res |= bytes[3] & BitmaskRange(0, 6);
            break;
        }

        return res;
    }

    t_unicode_code_pt StrChrAtByte(const s_str_rdonly str, const t_len byte_index) {
        ZF_ASSERT(IsValidUTF8Str(str));
        ZF_ASSERT(byte_index >= 0 && byte_index < str.bytes.len);

        t_len chr_first_byte_index = byte_index;

        do {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[chr_first_byte_index]];

            if (byte_type == ek_utf8_byte_type_continuation) {
                chr_first_byte_index--;
                continue;
            }

            const t_len chr_byte_cnt = byte_type - ek_utf8_byte_type_ascii + 1;
            const auto chr_bytes = Slice(str.bytes, byte_index, byte_index + chr_byte_cnt);
            return UTF8ChrBytesToCodePoint(chr_bytes);
        } while (true);
    }

    t_b8 WalkStr(const s_str_rdonly str, t_len *const byte_index, s_str_chr_info *const o_chr_info) {
        ZF_ASSERT(IsValidUTF8Str(str));
        ZF_ASSERT(*byte_index >= 0 && *byte_index <= str.bytes.len);

        if (*byte_index == str.bytes.len) {
            return false;
        }

        const auto byte_type = g_utf8_byte_type_table[str.bytes[*byte_index]];

        switch (byte_type) {
        case ek_utf8_byte_type_ascii:
            if (!str.bytes[*byte_index]) {
                return false;
            }

            [[fallthrough]];

        case ek_utf8_byte_type_2byte_start:
        case ek_utf8_byte_type_3byte_start:
        case ek_utf8_byte_type_4byte_start: {
            const t_len chr_len = byte_type - ek_utf8_byte_type_ascii + 1;
            const auto chr_bytes = Slice(str.bytes, *byte_index, *byte_index + chr_len);
            *o_chr_info = {.code_pt = UTF8ChrBytesToCodePoint(chr_bytes), .byte_index = *byte_index};
            *byte_index += chr_len;

            return true;
        }

        default:
            ZF_ASSERT(false);
            return false;
        }
    }

    t_b8 WalkStrReverse(const s_str_rdonly str, t_len *const byte_index, s_str_chr_info *const o_chr_info) {
        ZF_ASSERT(IsValidUTF8Str(str));
        ZF_ASSERT(*byte_index >= -1 && *byte_index < str.bytes.len);

        if (*byte_index == -1) {
            return false;
        }

        while (g_utf8_byte_type_table[str.bytes[*byte_index]] == ek_utf8_byte_type_continuation) {
            (*byte_index)--;
        }

        const auto byte_type = g_utf8_byte_type_table[str.bytes[*byte_index]];

        switch (byte_type) {
        case ek_utf8_byte_type_ascii:
        case ek_utf8_byte_type_2byte_start:
        case ek_utf8_byte_type_3byte_start:
        case ek_utf8_byte_type_4byte_start: {
            const t_len chr_len = byte_type - ek_utf8_byte_type_ascii + 1;
            const auto chr_bytes = Slice(str.bytes, *byte_index, *byte_index + chr_len);
            *o_chr_info = {.code_pt = UTF8ChrBytesToCodePoint(chr_bytes), .byte_index = *byte_index};
            (*byte_index)--;
            return true;
        }

        default:
            ZF_ASSERT(false);
            return false;
        }
    }

    void MarkStrCodePoints(const s_str_rdonly str, t_unicode_code_pt_bit_vec &code_pts) {
        ZF_ASSERT(IsValidUTF8Str(str));

        ZF_WALK_STR(str, chr_info) {
            SetBit(code_pts, chr_info.code_pt);
        }
    }
}
