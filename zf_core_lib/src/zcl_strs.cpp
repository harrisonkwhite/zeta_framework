#include <zcl/zcl_strs.h>

namespace zcl::strs {
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

    t_b8 check_valid_utf8(const t_str_rdonly str) {
        t_i32 cost = 0;

        for (t_i32 i = 0; i < str.bytes.len; i++) {
            const auto byte_type = k_utf8_byte_type_table[str.bytes[i]];

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

    t_i32 calc_len(const t_str_rdonly str) {
        ZF_ASSERT(check_valid_utf8(str));

        t_i32 i = 0;
        t_i32 len = 0;

        while (i < str.bytes.len) {
            const auto byte_type = k_utf8_byte_type_table[str.bytes[i]];

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

    t_code_pt utf8_bytes_to_code_pt(const t_array_rdonly<t_u8> bytes) {
        ZF_ASSERT(bytes.len >= 1 && bytes.len <= 4);

        t_code_pt result = 0;

        switch (bytes.len) {
        case 1:
            // 0xxxxxxx
            result |= bytes[0] & mem::byte_bitmask_create_range(0, 7);
            break;

        case 2:
            // 110xxxxx 10xxxxxx
            result |= static_cast<t_code_pt>((bytes[0] & mem::byte_bitmask_create_range(0, 5)) << 6);
            result |= bytes[1] & mem::byte_bitmask_create_range(0, 6);
            break;

        case 3:
            // 1110xxxx 10xxxxxx 10xxxxxx
            result |= static_cast<t_code_pt>((bytes[0] & mem::byte_bitmask_create_range(0, 4)) << 12);
            result |= static_cast<t_code_pt>((bytes[1] & mem::byte_bitmask_create_range(0, 6)) << 6);
            result |= bytes[2] & mem::byte_bitmask_create_range(0, 6);
            break;

        case 4:
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            result |= static_cast<t_code_pt>((bytes[0] & mem::byte_bitmask_create_range(0, 3)) << 18);
            result |= static_cast<t_code_pt>((bytes[1] & mem::byte_bitmask_create_range(0, 6)) << 12);
            result |= static_cast<t_code_pt>((bytes[2] & mem::byte_bitmask_create_range(0, 6)) << 6);
            result |= bytes[3] & mem::byte_bitmask_create_range(0, 6);
            break;

        default:
            ZF_UNREACHABLE();
        }

        return result;
    }

    void code_pt_to_utf8_bytes(const t_code_pt cp, t_static_array<t_u8, 4> *const o_bytes, t_i32 *const o_byte_cnt) {
        *o_bytes = {};
        *o_byte_cnt = utf8_byte_cnt(cp);

        switch (*o_byte_cnt) {
        case 1:
            // 0xxxxxxx

            (*o_bytes)[0] |= cp & mem::byte_bitmask_create_range(0, 7);

            break;

        case 2:
            // 110xxxxx 10xxxxxx

            (*o_bytes)[0] = 0b11000000;
            (*o_bytes)[0] |= (cp & (mem::byte_bitmask_create_range(0, 5) << 6)) >> 6;

            (*o_bytes)[1] = 0b10000000;
            (*o_bytes)[1] |= cp & mem::byte_bitmask_create_range(0, 6);

            break;

        case 3:
            // 1110xxxx 10xxxxxx 10xxxxxx

            (*o_bytes)[0] = 0b11100000;
            (*o_bytes)[0] |= (cp & (mem::byte_bitmask_create_range(0, 4) << 12)) >> 12;

            (*o_bytes)[1] = 0b10000000;
            (*o_bytes)[1] |= (cp & (mem::byte_bitmask_create_range(0, 6) << 6)) >> 6;

            (*o_bytes)[2] = 0b10000000;
            (*o_bytes)[2] |= cp & mem::byte_bitmask_create_range(0, 6);

            break;

        case 4:
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

            (*o_bytes)[0] = 0b11110000;
            (*o_bytes)[0] |= (cp & (mem::byte_bitmask_create_range(0, 3) << 18)) >> 18;

            (*o_bytes)[1] = 0b10000000;
            (*o_bytes)[1] |= (cp & (mem::byte_bitmask_create_range(0, 6) << 12)) >> 12;

            (*o_bytes)[2] = 0b10000000;
            (*o_bytes)[2] |= (cp & (mem::byte_bitmask_create_range(0, 6) << 6)) >> 6;

            (*o_bytes)[3] = 0b10000000;
            (*o_bytes)[3] |= cp & mem::byte_bitmask_create_range(0, 6);

            break;

        default:
            ZF_UNREACHABLE();
        }
    }

    t_code_pt find_code_pt_at_byte(const t_str_rdonly str, const t_i32 byte_index) {
        ZF_ASSERT(check_valid_utf8(str));
        ZF_ASSERT(byte_index >= 0 && byte_index < str.bytes.len);

        t_i32 cp_first_byte_index = byte_index;

        do {
            const auto byte_type = k_utf8_byte_type_table[str.bytes[cp_first_byte_index]];

            if (byte_type == ek_utf8_byte_type_continuation) {
                cp_first_byte_index--;
                continue;
            }

            const t_i32 cp_byte_cnt = byte_type - ek_utf8_byte_type_ascii + 1;
            const auto cp_bytes = array_slice(str.bytes, byte_index, byte_index + cp_byte_cnt);
            return utf8_bytes_to_code_pt(cp_bytes);
        } while (true);
    }

    void mark_code_points(const t_str_rdonly str, t_code_pt_bitset *const code_pts) {
        ZF_ASSERT(check_valid_utf8(str));

        ZF_WALK_STR (str, step) {
            mem::bitset_set(*code_pts, step.code_pt); // @todo
        }
    }

    t_b8 walk(const t_str_rdonly str, t_i32 *const byte_index, t_str_walk_step *const o_step) {
        ZF_ASSERT(check_valid_utf8(str));
        ZF_ASSERT(*byte_index >= 0 && *byte_index <= str.bytes.len);

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
                const auto cp_bytes = array_slice(str.bytes, *byte_index, *byte_index + cp_byte_cnt);
                *o_step = {.code_pt = utf8_bytes_to_code_pt(cp_bytes), .byte_index = *byte_index};
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

    t_b8 walk_reverse(const t_str_rdonly str, t_i32 *const byte_index, t_str_walk_step *const o_step) {
        ZF_ASSERT(check_valid_utf8(str));
        ZF_ASSERT(*byte_index >= -1 && *byte_index < str.bytes.len);

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
                const auto cp_bytes = array_slice(str.bytes, *byte_index, *byte_index + cp_byte_cnt);
                *o_step = {.code_pt = utf8_bytes_to_code_pt(cp_bytes), .byte_index = *byte_index};
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
