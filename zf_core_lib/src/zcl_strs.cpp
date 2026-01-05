#include <zcl/zcl_strs.h>

namespace zf::strs {
    enum t_utf8_byte_type : t_i32 {
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_4byte_start,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_invalid
    };

    static_assert(ec_utf8_byte_type_4byte_start - ec_utf8_byte_type_ascii + 1 == 4); // This is assumed in various algorithms.

    static const t_static_array<t_utf8_byte_type, 256> g_utf8_byte_type_table = {{
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,
        ec_utf8_byte_type_ascii,

        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,
        ec_utf8_byte_type_continuation,

        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,
        ec_utf8_byte_type_2byte_start,

        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,
        ec_utf8_byte_type_3byte_start,

        ec_utf8_byte_type_4byte_start,
        ec_utf8_byte_type_4byte_start,
        ec_utf8_byte_type_4byte_start,
        ec_utf8_byte_type_4byte_start,
        ec_utf8_byte_type_4byte_start,
        ec_utf8_byte_type_4byte_start,
        ec_utf8_byte_type_4byte_start,
        ec_utf8_byte_type_4byte_start,

        ec_utf8_byte_type_invalid,
        ec_utf8_byte_type_invalid,
        ec_utf8_byte_type_invalid,
        ec_utf8_byte_type_invalid,
        ec_utf8_byte_type_invalid,
        ec_utf8_byte_type_invalid,
        ec_utf8_byte_type_invalid,
        ec_utf8_byte_type_invalid,
    }};

    t_b8 str_is_valid_utf8(const t_str_rdonly str) {
        t_i32 cost = 0;

        for (t_i32 i = 0; i < str.bytes.len; i++) {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[i]];

            switch (byte_type) {
            case ec_utf8_byte_type_ascii:
            case ec_utf8_byte_type_2byte_start:
            case ec_utf8_byte_type_3byte_start:
            case ec_utf8_byte_type_4byte_start:
                if (cost > 0) {
                    return false;
                }

                cost = byte_type - ec_utf8_byte_type_ascii + 1;

                break;

            case ec_utf8_byte_type_continuation:
                if (cost == 0) {
                    return false;
                }

                break;

            case ec_utf8_byte_type_invalid:
                return false;
            }

            cost--;
        }

        return true;
    }

    t_i32 str_get_len(const t_str_rdonly str) {
        ZF_ASSERT(str_is_valid_utf8(str));

        t_i32 i = 0;
        t_i32 len = 0;

        while (i < str.bytes.len) {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[i]];

            switch (byte_type) {
            case ec_utf8_byte_type_ascii:
            case ec_utf8_byte_type_2byte_start:
            case ec_utf8_byte_type_3byte_start:
            case ec_utf8_byte_type_4byte_start:
                i += byte_type - ec_utf8_byte_type_ascii + 1;
                break;

            default:
                ZF_UNREACHABLE();
            }

            len++;
        }

        return len;
    }

    static t_code_pt convert_utf8_bytes_to_code_pt(const t_array_rdonly<t_u8> bytes) {
        ZF_ASSERT(bytes.len >= 1 && bytes.len <= 4);

        t_code_pt result = 0;

        switch (bytes.len) {
        case 1:
            // 0xxxxxxx
            result |= bytes[0] & mem::create_byte_bitmask_range(0, 7);
            break;

        case 2:
            // 110xxxxx 10xxxxxx
            result |= static_cast<t_code_pt>((bytes[0] & mem::create_byte_bitmask_range(0, 5)) << 6);
            result |= bytes[1] & mem::create_byte_bitmask_range(0, 6);
            break;

        case 3:
            // 1110xxxx 10xxxxxx 10xxxxxx
            result |= static_cast<t_code_pt>((bytes[0] & mem::create_byte_bitmask_range(0, 4)) << 12);
            result |= static_cast<t_code_pt>((bytes[1] & mem::create_byte_bitmask_range(0, 6)) << 6);
            result |= bytes[2] & mem::create_byte_bitmask_range(0, 6);
            break;

        case 4:
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            result |= static_cast<t_code_pt>((bytes[0] & mem::create_byte_bitmask_range(0, 3)) << 18);
            result |= static_cast<t_code_pt>((bytes[1] & mem::create_byte_bitmask_range(0, 6)) << 12);
            result |= static_cast<t_code_pt>((bytes[2] & mem::create_byte_bitmask_range(0, 6)) << 6);
            result |= bytes[3] & mem::create_byte_bitmask_range(0, 6);
            break;

        default:
            ZF_UNREACHABLE();
        }

        return result;
    }

    t_code_pt str_find_code_pt_at_byte(const t_str_rdonly str, const t_i32 byte_index) {
        ZF_ASSERT(str_is_valid_utf8(str));
        ZF_ASSERT(byte_index >= 0 && byte_index < str.bytes.len);

        t_i32 cp_first_byte_index = byte_index;

        do {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[cp_first_byte_index]];

            if (byte_type == ec_utf8_byte_type_continuation) {
                cp_first_byte_index--;
                continue;
            }

            const t_i32 cp_byte_cnt = byte_type - ec_utf8_byte_type_ascii + 1;
            const auto cp_bytes = array_slice(str.bytes, byte_index, byte_index + cp_byte_cnt);
            return convert_utf8_bytes_to_code_pt(cp_bytes);
        } while (true);
    }

    void str_mark_code_points(const t_str_rdonly str, t_code_pt_bitset *const code_pts) {
        ZF_ASSERT(str_is_valid_utf8(str));

        ZF_WALK_STR (str, step) {
            mem::set_bit(*code_pts, step.code_pt); // @todo
        }
    }

    t_b8 str_walk(const t_str_rdonly str, t_i32 *const byte_index, t_str_walk_step *const o_step) {
        ZF_ASSERT(str_is_valid_utf8(str));
        ZF_ASSERT(*byte_index >= 0 && *byte_index <= str.bytes.len);

        if (*byte_index == str.bytes.len) {
            return false;
        }

        while (true) {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[*byte_index]];

            switch (byte_type) {
            case ec_utf8_byte_type_ascii:
            case ec_utf8_byte_type_2byte_start:
            case ec_utf8_byte_type_3byte_start:
            case ec_utf8_byte_type_4byte_start: {
                const t_i32 cp_byte_cnt = byte_type - ec_utf8_byte_type_ascii + 1;
                const auto cp_bytes = array_slice(str.bytes, *byte_index, *byte_index + cp_byte_cnt);
                *o_step = {.code_pt = convert_utf8_bytes_to_code_pt(cp_bytes), .byte_index = *byte_index};
                *byte_index += cp_byte_cnt;

                return true;
            }

            case ec_utf8_byte_type_continuation:
                (*byte_index)--;
                break;

            default:
                ZF_UNREACHABLE();
            }
        }
    }

    t_b8 str_walk_reverse(const t_str_rdonly str, t_i32 *const byte_index, t_str_walk_step *const o_step) {
        ZF_ASSERT(str_is_valid_utf8(str));
        ZF_ASSERT(*byte_index >= -1 && *byte_index < str.bytes.len);

        if (*byte_index == -1) {
            return false;
        }

        while (true) {
            const auto byte_type = g_utf8_byte_type_table[str.bytes[*byte_index]];

            switch (byte_type) {
            case ec_utf8_byte_type_ascii:
            case ec_utf8_byte_type_2byte_start:
            case ec_utf8_byte_type_3byte_start:
            case ec_utf8_byte_type_4byte_start: {
                const t_i32 cp_byte_cnt = byte_type - ec_utf8_byte_type_ascii + 1;
                const auto cp_bytes = array_slice(str.bytes, *byte_index, *byte_index + cp_byte_cnt);
                *o_step = {.code_pt = convert_utf8_bytes_to_code_pt(cp_bytes), .byte_index = *byte_index};
                (*byte_index)--;

                return true;
            }

            case ec_utf8_byte_type_continuation:
                (*byte_index)--;
                break;

            default:
                ZF_UNREACHABLE();
            }
        }
    }
}
