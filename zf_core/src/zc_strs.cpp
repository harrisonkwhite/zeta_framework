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

    t_b8 IsValidUTF8Str(const s_str_rdonly str) {
        t_size cost = 0;

        for (t_size i = 0; i < str.chrs.len; i++) {
            const auto byte_type = g_utf8_byte_type_table[str.chrs[i]];

            switch (byte_type) {
                case ek_utf8_byte_type_ascii:
                    if (!str.chrs[i]) {
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

    t_b8 CalcUTF8StrLen(const s_str_rdonly str, t_size& o_len) {
        o_len = 0;

        t_size cost = 0;

        for (t_size i = 0; i < str.chrs.len; i++) {
            const auto byte_type = g_utf8_byte_type_table[str.chrs[i]];

            switch (byte_type) {
                case ek_utf8_byte_type_ascii:
                    if (!str.chrs[i]) {
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

    t_size CalcUTF8StrLenFastButUnsafe(const s_str_rdonly str) {
        ZF_ASSERT(IsValidUTF8Str(str));

        t_size i = 0;
        t_size len = 0;

        while (i < str.chrs.len) {
            const auto byte_type = g_utf8_byte_type_table[str.chrs[i]];

            switch (byte_type) {
                case ek_utf8_byte_type_ascii:
                    if (!str.chrs[i]) {
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







    void AppendBits(const s_bit_range bv, const s_bit_range_rdonly bv_of_bits_to_add) {
        ZF_ASSERT(BitsToBytes(bv.bit_cnt + bv_of_bits_to_add.bit_cnt) <= bv.backing_bytes.len);

        // Basically just append as many bits as we can to the last byte and recurse.
        const t_size bits_to_append_cnt = 8 - (bv.bit_cnt % 8);

        //bv_of_bits_to_add.bytes[0]
    }

    t_u32 UnicodeCodepointFromBytes(const s_array_rdonly<t_u8> bytes) {
        ZF_ASSERT(bytes.len >= 1 && bytes.len <= 4);

        t_u32 res = 0;
        s_bit_range res_bv = ToBytes(res);

        switch (bytes.len) {
        case 1:
            res |= bytes[0] & 127;
            //AppendBits(res_bv, );
            break;

        case 2:
            res |= (bytes[1] & ByteBitmask(0, 6));
            res |= (bytes[0] & ByteBitmask(0, 5)) << 6;
            break;
        }

        return res;
    }

    t_s32 WalkUTF8Str(const s_str_rdonly str, t_size& byte_index) {
        const auto byte_type = g_utf8_byte_type_table[str.chrs[byte_index]];

        t_s32 res = 0;

        switch (byte_type) {
            case ek_utf8_byte_type_ascii:
            case ek_utf8_byte_type_2byte_start:
            case ek_utf8_byte_type_3byte_start:
            case ek_utf8_byte_type_4byte_start:
                {
                    static_assert(ek_utf8_byte_type_4byte_start - ek_utf8_byte_type_ascii + 1 == 4);
                    const t_size chr_len = byte_type - ek_utf8_byte_type_ascii + 1;
                    const auto chr_bytes = Slice(str.chrs, byte_index, byte_index + chr_len);
                    const t_s32 res = UnicodeCodepointFromBytes(ToByteArray(chr_bytes));
                    byte_index += chr_len;
                    return res;
                }

                break;

            case ek_utf8_byte_type_continuation:
            case ek_utf8_byte_type_invalid:
                ZF_ASSERT(false);
                break;
        }

        return res;
    }










}
