#include <zc/zc_strs.h>

#include <zc/ds/zc_bit_vector.h>

namespace zf {
    enum e_utf8_byte_type : t_s32 {
        ek_utf8_byte_type_ascii,
        ek_utf8_byte_type_2byte_start,
        ek_utf8_byte_type_3byte_start,
        ek_utf8_byte_type_4byte_start,
        ek_utf8_byte_type_continuation,
        ek_utf8_byte_type_invalid
    };

    static constexpr s_static_array<e_utf8_byte_type, 256> g_utf8_byte_type_table = {{
        ek_utf8_byte_type_ascii, // 0000 0000
        ek_utf8_byte_type_ascii, // 0000 0001
        ek_utf8_byte_type_ascii, // 0000 0010
        ek_utf8_byte_type_ascii, // 0000 0011
        ek_utf8_byte_type_ascii, // 0000 0100
        ek_utf8_byte_type_ascii, // 0000 0101
        ek_utf8_byte_type_ascii, // 0000 0110
        ek_utf8_byte_type_ascii, // 0000 0111
        ek_utf8_byte_type_ascii, // 0000 1000
        ek_utf8_byte_type_ascii, // 0000 1001
        ek_utf8_byte_type_ascii, // 0000 1010
        ek_utf8_byte_type_ascii, // 0000 1011
        ek_utf8_byte_type_ascii, // 0000 1100
        ek_utf8_byte_type_ascii, // 0000 1101
        ek_utf8_byte_type_ascii, // 0000 1110
        ek_utf8_byte_type_ascii, // 0000 1111
        ek_utf8_byte_type_ascii, // 0001 0000
        ek_utf8_byte_type_ascii, // 0001 0001
        ek_utf8_byte_type_ascii, // 0001 0010
        ek_utf8_byte_type_ascii, // 0001 0011
        ek_utf8_byte_type_ascii, // 0001 0100
        ek_utf8_byte_type_ascii, // 0001 0101
        ek_utf8_byte_type_ascii, // 0001 0110
        ek_utf8_byte_type_ascii, // 0001 0111
        ek_utf8_byte_type_ascii, // 0001 1000
        ek_utf8_byte_type_ascii, // 0001 1001
        ek_utf8_byte_type_ascii, // 0001 1010
        ek_utf8_byte_type_ascii, // 0001 1011
        ek_utf8_byte_type_ascii, // 0001 1100
        ek_utf8_byte_type_ascii, // 0001 1101
        ek_utf8_byte_type_ascii, // 0001 1110
        ek_utf8_byte_type_ascii, // 0001 1111
        ek_utf8_byte_type_ascii, // 0010 0000
        ek_utf8_byte_type_ascii, // 0010 0001
        ek_utf8_byte_type_ascii, // 0010 0010
        ek_utf8_byte_type_ascii, // 0010 0011
        ek_utf8_byte_type_ascii, // 0010 0100
        ek_utf8_byte_type_ascii, // 0010 0101
        ek_utf8_byte_type_ascii, // 0010 0110
        ek_utf8_byte_type_ascii, // 0010 0111
        ek_utf8_byte_type_ascii, // 0010 1000
        ek_utf8_byte_type_ascii, // 0010 1001
        ek_utf8_byte_type_ascii, // 0010 1010
        ek_utf8_byte_type_ascii, // 0010 1011
        ek_utf8_byte_type_ascii, // 0010 1100
        ek_utf8_byte_type_ascii, // 0010 1101
        ek_utf8_byte_type_ascii, // 0010 1110
        ek_utf8_byte_type_ascii, // 0010 1111
        ek_utf8_byte_type_ascii, // 0011 0000
        ek_utf8_byte_type_ascii, // 0011 0001
        ek_utf8_byte_type_ascii, // 0011 0010
        ek_utf8_byte_type_ascii, // 0011 0011
        ek_utf8_byte_type_ascii, // 0011 0100
        ek_utf8_byte_type_ascii, // 0011 0101
        ek_utf8_byte_type_ascii, // 0011 0110
        ek_utf8_byte_type_ascii, // 0011 0111
        ek_utf8_byte_type_ascii, // 0011 1000
        ek_utf8_byte_type_ascii, // 0011 1001
        ek_utf8_byte_type_ascii, // 0011 1010
        ek_utf8_byte_type_ascii, // 0011 1011
        ek_utf8_byte_type_ascii, // 0011 1100
        ek_utf8_byte_type_ascii, // 0011 1101
        ek_utf8_byte_type_ascii, // 0011 1110
        ek_utf8_byte_type_ascii, // 0011 1111
        ek_utf8_byte_type_ascii, // 0100 0000
        ek_utf8_byte_type_ascii, // 0100 0001
        ek_utf8_byte_type_ascii, // 0100 0010
        ek_utf8_byte_type_ascii, // 0100 0011
        ek_utf8_byte_type_ascii, // 0100 0100
        ek_utf8_byte_type_ascii, // 0100 0101
        ek_utf8_byte_type_ascii, // 0100 0110
        ek_utf8_byte_type_ascii, // 0100 0111
        ek_utf8_byte_type_ascii, // 0100 1000
        ek_utf8_byte_type_ascii, // 0100 1001
        ek_utf8_byte_type_ascii, // 0100 1010
        ek_utf8_byte_type_ascii, // 0100 1011
        ek_utf8_byte_type_ascii, // 0100 1100
        ek_utf8_byte_type_ascii, // 0100 1101
        ek_utf8_byte_type_ascii, // 0100 1110
        ek_utf8_byte_type_ascii, // 0100 1111
        ek_utf8_byte_type_ascii, // 0101 0000
        ek_utf8_byte_type_ascii, // 0101 0001
        ek_utf8_byte_type_ascii, // 0101 0010
        ek_utf8_byte_type_ascii, // 0101 0011
        ek_utf8_byte_type_ascii, // 0101 0100
        ek_utf8_byte_type_ascii, // 0101 0101
        ek_utf8_byte_type_ascii, // 0101 0110
        ek_utf8_byte_type_ascii, // 0101 0111
        ek_utf8_byte_type_ascii, // 0101 1000
        ek_utf8_byte_type_ascii, // 0101 1001
        ek_utf8_byte_type_ascii, // 0101 1010
        ek_utf8_byte_type_ascii, // 0101 1011
        ek_utf8_byte_type_ascii, // 0101 1100
        ek_utf8_byte_type_ascii, // 0101 1101
        ek_utf8_byte_type_ascii, // 0101 1110
        ek_utf8_byte_type_ascii, // 0101 1111
        ek_utf8_byte_type_ascii, // 0110 0000
        ek_utf8_byte_type_ascii, // 0110 0001
        ek_utf8_byte_type_ascii, // 0110 0010
        ek_utf8_byte_type_ascii, // 0110 0011
        ek_utf8_byte_type_ascii, // 0110 0100
        ek_utf8_byte_type_ascii, // 0110 0101
        ek_utf8_byte_type_ascii, // 0110 0110
        ek_utf8_byte_type_ascii, // 0110 0111
        ek_utf8_byte_type_ascii, // 0110 1000
        ek_utf8_byte_type_ascii, // 0110 1001
        ek_utf8_byte_type_ascii, // 0110 1010
        ek_utf8_byte_type_ascii, // 0110 1011
        ek_utf8_byte_type_ascii, // 0110 1100
        ek_utf8_byte_type_ascii, // 0110 1101
        ek_utf8_byte_type_ascii, // 0110 1110
        ek_utf8_byte_type_ascii, // 0110 1111
        ek_utf8_byte_type_ascii, // 0111 0000
        ek_utf8_byte_type_ascii, // 0111 0001
        ek_utf8_byte_type_ascii, // 0111 0010
        ek_utf8_byte_type_ascii, // 0111 0011
        ek_utf8_byte_type_ascii, // 0111 0100
        ek_utf8_byte_type_ascii, // 0111 0101
        ek_utf8_byte_type_ascii, // 0111 0110
        ek_utf8_byte_type_ascii, // 0111 0111
        ek_utf8_byte_type_ascii, // 0111 1000
        ek_utf8_byte_type_ascii, // 0111 1001
        ek_utf8_byte_type_ascii, // 0111 1010
        ek_utf8_byte_type_ascii, // 0111 1011
        ek_utf8_byte_type_ascii, // 0111 1100
        ek_utf8_byte_type_ascii, // 0111 1101
        ek_utf8_byte_type_ascii, // 0111 1110
        ek_utf8_byte_type_ascii, // 0111 1111

        ek_utf8_byte_type_continuation, // 1000 0000
        ek_utf8_byte_type_continuation, // 1000 0001
        ek_utf8_byte_type_continuation, // 1000 0010
        ek_utf8_byte_type_continuation, // 1000 0011
        ek_utf8_byte_type_continuation, // 1000 0100
        ek_utf8_byte_type_continuation, // 1000 0101
        ek_utf8_byte_type_continuation, // 1000 0110
        ek_utf8_byte_type_continuation, // 1000 0111
        ek_utf8_byte_type_continuation, // 1000 1000
        ek_utf8_byte_type_continuation, // 1000 1001
        ek_utf8_byte_type_continuation, // 1000 1010
        ek_utf8_byte_type_continuation, // 1000 1011
        ek_utf8_byte_type_continuation, // 1000 1100
        ek_utf8_byte_type_continuation, // 1000 1101
        ek_utf8_byte_type_continuation, // 1000 1110
        ek_utf8_byte_type_continuation, // 1000 1111
        ek_utf8_byte_type_continuation, // 1001 0000
        ek_utf8_byte_type_continuation, // 1001 0001
        ek_utf8_byte_type_continuation, // 1001 0010
        ek_utf8_byte_type_continuation, // 1001 0011
        ek_utf8_byte_type_continuation, // 1001 0100
        ek_utf8_byte_type_continuation, // 1001 0101
        ek_utf8_byte_type_continuation, // 1001 0110
        ek_utf8_byte_type_continuation, // 1001 0111
        ek_utf8_byte_type_continuation, // 1001 1000
        ek_utf8_byte_type_continuation, // 1001 1001
        ek_utf8_byte_type_continuation, // 1001 1010
        ek_utf8_byte_type_continuation, // 1001 1011
        ek_utf8_byte_type_continuation, // 1001 1100
        ek_utf8_byte_type_continuation, // 1001 1101
        ek_utf8_byte_type_continuation, // 1001 1110
        ek_utf8_byte_type_continuation, // 1001 1111
        ek_utf8_byte_type_continuation, // 1010 0000
        ek_utf8_byte_type_continuation, // 1010 0001
        ek_utf8_byte_type_continuation, // 1010 0010
        ek_utf8_byte_type_continuation, // 1010 0011
        ek_utf8_byte_type_continuation, // 1010 0100
        ek_utf8_byte_type_continuation, // 1010 0101
        ek_utf8_byte_type_continuation, // 1010 0110
        ek_utf8_byte_type_continuation, // 1010 0111
        ek_utf8_byte_type_continuation, // 1010 1000
        ek_utf8_byte_type_continuation, // 1010 1001
        ek_utf8_byte_type_continuation, // 1010 1010
        ek_utf8_byte_type_continuation, // 1010 1011
        ek_utf8_byte_type_continuation, // 1010 1100
        ek_utf8_byte_type_continuation, // 1010 1101
        ek_utf8_byte_type_continuation, // 1010 1110
        ek_utf8_byte_type_continuation, // 1010 1111
        ek_utf8_byte_type_continuation, // 1011 0000
        ek_utf8_byte_type_continuation, // 1011 0001
        ek_utf8_byte_type_continuation, // 1011 0010
        ek_utf8_byte_type_continuation, // 1011 0011
        ek_utf8_byte_type_continuation, // 1011 0100
        ek_utf8_byte_type_continuation, // 1011 0101
        ek_utf8_byte_type_continuation, // 1011 0110
        ek_utf8_byte_type_continuation, // 1011 0111
        ek_utf8_byte_type_continuation, // 1011 1000
        ek_utf8_byte_type_continuation, // 1011 1001
        ek_utf8_byte_type_continuation, // 1011 1010
        ek_utf8_byte_type_continuation, // 1011 1011
        ek_utf8_byte_type_continuation, // 1011 1100
        ek_utf8_byte_type_continuation, // 1011 1101
        ek_utf8_byte_type_continuation, // 1011 1110
        ek_utf8_byte_type_continuation, // 1011 1111

        ek_utf8_byte_type_2byte_start, // 1100 0000
        ek_utf8_byte_type_2byte_start, // 1100 0001
        ek_utf8_byte_type_2byte_start, // 1100 0010
        ek_utf8_byte_type_2byte_start, // 1100 0011
        ek_utf8_byte_type_2byte_start, // 1100 0100
        ek_utf8_byte_type_2byte_start, // 1100 0101
        ek_utf8_byte_type_2byte_start, // 1100 0110
        ek_utf8_byte_type_2byte_start, // 1100 0111
        ek_utf8_byte_type_2byte_start, // 1100 1000
        ek_utf8_byte_type_2byte_start, // 1100 1001
        ek_utf8_byte_type_2byte_start, // 1100 1010
        ek_utf8_byte_type_2byte_start, // 1100 1011
        ek_utf8_byte_type_2byte_start, // 1100 1100
        ek_utf8_byte_type_2byte_start, // 1100 1101
        ek_utf8_byte_type_2byte_start, // 1100 1110
        ek_utf8_byte_type_2byte_start, // 1100 1111
        ek_utf8_byte_type_2byte_start, // 1101 0000
        ek_utf8_byte_type_2byte_start, // 1101 0001
        ek_utf8_byte_type_2byte_start, // 1101 0010
        ek_utf8_byte_type_2byte_start, // 1101 0011
        ek_utf8_byte_type_2byte_start, // 1101 0100
        ek_utf8_byte_type_2byte_start, // 1101 0101
        ek_utf8_byte_type_2byte_start, // 1101 0110
        ek_utf8_byte_type_2byte_start, // 1101 0111
        ek_utf8_byte_type_2byte_start, // 1101 1000
        ek_utf8_byte_type_2byte_start, // 1101 1001
        ek_utf8_byte_type_2byte_start, // 1101 1010
        ek_utf8_byte_type_2byte_start, // 1101 1011
        ek_utf8_byte_type_2byte_start, // 1101 1100
        ek_utf8_byte_type_2byte_start, // 1101 1101
        ek_utf8_byte_type_2byte_start, // 1101 1110
        ek_utf8_byte_type_2byte_start, // 1101 1111

        ek_utf8_byte_type_3byte_start, // 1110 0000
        ek_utf8_byte_type_3byte_start, // 1110 0001
        ek_utf8_byte_type_3byte_start, // 1110 0010
        ek_utf8_byte_type_3byte_start, // 1110 0011
        ek_utf8_byte_type_3byte_start, // 1110 0100
        ek_utf8_byte_type_3byte_start, // 1110 0101
        ek_utf8_byte_type_3byte_start, // 1110 0110
        ek_utf8_byte_type_3byte_start, // 1110 0111
        ek_utf8_byte_type_3byte_start, // 1110 1000
        ek_utf8_byte_type_3byte_start, // 1110 1001
        ek_utf8_byte_type_3byte_start, // 1110 1010
        ek_utf8_byte_type_3byte_start, // 1110 1011
        ek_utf8_byte_type_3byte_start, // 1110 1100
        ek_utf8_byte_type_3byte_start, // 1110 1101
        ek_utf8_byte_type_3byte_start, // 1110 1110
        ek_utf8_byte_type_3byte_start, // 1110 1111

        ek_utf8_byte_type_4byte_start, // 1111 0000
        ek_utf8_byte_type_4byte_start, // 1111 0001
        ek_utf8_byte_type_4byte_start, // 1111 0010
        ek_utf8_byte_type_4byte_start, // 1111 0011
        ek_utf8_byte_type_4byte_start, // 1111 0100
        ek_utf8_byte_type_4byte_start, // 1111 0101
        ek_utf8_byte_type_4byte_start, // 1111 0110
        ek_utf8_byte_type_4byte_start, // 1111 0111

        ek_utf8_byte_type_invalid, // 1111 1000
        ek_utf8_byte_type_invalid, // 1111 1001
        ek_utf8_byte_type_invalid, // 1111 1010
        ek_utf8_byte_type_invalid, // 1111 1011
        ek_utf8_byte_type_invalid, // 1111 1100
        ek_utf8_byte_type_invalid, // 1111 1101
        ek_utf8_byte_type_invalid, // 1111 1110
        ek_utf8_byte_type_invalid // 1111 1111
    }};

    t_b8 IsValidUTF8Str(const s_str_utf8_rdonly str) {
        t_size cost = 0;

        for (t_size i = 0; i < str.bytes.len; i++) {
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

                    static_assert(ek_utf8_byte_type_4byte_start - ek_utf8_byte_type_ascii + 1 == 4);
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

    t_b8 CalcUTF8StrLen(const s_str_utf8_rdonly str, t_size& o_len) {
        o_len = 0;

        t_size cost = 0;

        for (t_size i = 0; i < str.bytes.len; i++) {
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

                    static_assert(ek_utf8_byte_type_4byte_start - ek_utf8_byte_type_ascii + 1 == 4);
                    cost = byte_type - ek_utf8_byte_type_ascii + 1;

                    o_len++;

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

    t_size CalcUTF8StrLenFastButUnsafe(const s_str_utf8_rdonly str) {
        ZF_ASSERT(IsValidUTF8Str(str));

        t_size i = 0;
        t_size len = 0;

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
                    static_assert(ek_utf8_byte_type_4byte_start - ek_utf8_byte_type_ascii + 1 == 4);
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

    t_u32 UTF8ChrBytesToCodePoint(const s_array_rdonly<t_u8> bytes) {
        ZF_ASSERT(bytes.len >= 1 && bytes.len <= 4);
        ZF_ASSERT(IsValidUTF8Str({bytes}));

        t_u32 res = 0;

        switch (bytes.len) {
        case 1:
            // 0xxxxxxx
            res |= bytes[0] & ByteBitmask(0, 7);
            break;

        case 2:
            // 110xxxxx 10xxxxxx
            res |= static_cast<t_u32>((bytes[0] & ByteBitmask(0, 5)) << 6);
            res |= bytes[1] & ByteBitmask(0, 6);
            break;

        case 3:
            // 1110xxxx 10xxxxxx 10xxxxxx
            res |= static_cast<t_u32>((bytes[0] & ByteBitmask(0, 4)) << 12);
            res |= static_cast<t_u32>((bytes[1] & ByteBitmask(0, 6)) << 6);
            res |= bytes[2] & ByteBitmask(0, 6);
            break;

        case 4:
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            res |= static_cast<t_u32>((bytes[0] & ByteBitmask(0, 3)) << 18);
            res |= static_cast<t_u32>((bytes[1] & ByteBitmask(0, 6)) << 12);
            res |= static_cast<t_u32>((bytes[2] & ByteBitmask(0, 6)) << 6);
            res |= bytes[3] & ByteBitmask(0, 6);
            break;
        }

        return res;
    }

    t_b8 WalkUTF8Str(const s_str_utf8_rdonly str, t_size& pos, t_u32& o_code_pt) {
        ZF_ASSERT(IsValidUTF8Str(str));

        if (pos == str.bytes.len) {
            return false;
        }

        const auto byte_type = g_utf8_byte_type_table[str.bytes[pos]];

        switch (byte_type) {
            case ek_utf8_byte_type_ascii:
                if (!str.bytes[pos]) {
                    return false;
                }

                [[fallthrough]];

            case ek_utf8_byte_type_2byte_start:
            case ek_utf8_byte_type_3byte_start:
            case ek_utf8_byte_type_4byte_start:
                {
                    static_assert(ek_utf8_byte_type_4byte_start - ek_utf8_byte_type_ascii + 1 == 4);
                    const t_size chr_len = byte_type - ek_utf8_byte_type_ascii + 1;
                    const auto chr_bytes = Slice(str.bytes, pos, pos + chr_len);
                    o_code_pt = UTF8ChrBytesToCodePoint(chr_bytes);
                    pos += chr_len;
                    return true;
                }

            case ek_utf8_byte_type_continuation:
            case ek_utf8_byte_type_invalid:
                ZF_ASSERT(false);
                return true;
        }
    }
}
