#pragma once

#include <cstdio>
#include <zcl/zcl_math.h>
#include <zcl/zcl_algos.h>
#include <zcl/zcl_strs.h>

namespace zf::io {
    // ============================================================
    // @section: Streams

    enum t_stream_type : t_i32 {
        ec_stream_type_invalid,
        ec_stream_type_mem,
        ec_stream_type_file
    };

    enum t_stream_mode : t_i32 {
        ec_stream_mode_read,
        ec_stream_mode_write
    };

    struct t_stream {
        t_stream_type type;

        union {
            struct {
                t_array_mut<t_u8> bytes;
                t_i32 byte_pos;
            } mem;

            struct {
                FILE *file;
            } file;
        } type_data;

        t_stream_mode mode;
    };

    inline t_stream mem_stream_create(const t_array_mut<t_u8> bytes, const t_stream_mode mode, const t_i32 pos = 0) {
        return {.type = ec_stream_type_mem, .type_data = {.mem = {.bytes = bytes, .byte_pos = pos}}, .mode = mode};
    }

    inline t_array_mut<t_u8> mem_stream_get_bytes_written(const t_stream *const stream) {
        ZF_ASSERT(stream->type == ec_stream_type_mem);
        return array_slice(stream->type_data.mem.bytes, 0, stream->type_data.mem.byte_pos);
    }

    inline t_stream file_stream_create(FILE *const file, const t_stream_mode mode) {
        return {.type = ec_stream_type_file, .type_data = {.file = {.file = file}}, .mode = mode};
    }

    inline t_stream get_std_in() { return file_stream_create(stdin, ec_stream_mode_read); }
    inline t_stream get_std_out() { return file_stream_create(stdout, ec_stream_mode_write); }
    inline t_stream get_std_error() { return file_stream_create(stderr, ec_stream_mode_write); }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 stream_read_item(t_stream *const stream, tp_type *const o_item) {
        ZF_ASSERT(stream->mode == ec_stream_mode_read);

        const t_i32 size = ZF_SIZE_OF(tp_type);

        switch (stream->type) {
        case ec_stream_type_mem: {
            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = array_slice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            const auto dest = mem::get_as_bytes(*o_item);
            array_copy(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ec_stream_type_file:
            return fread(o_item, size, 1, stream->type_data.file.file) == 1;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 stream_write_item(t_stream *const stream, const tp_type &item) {
        ZF_ASSERT(stream->mode == ec_stream_mode_write);

        const t_i32 size = ZF_SIZE_OF(tp_type);

        switch (stream->type) {
        case ec_stream_type_mem: {
            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = mem::get_as_bytes(item);
            const auto dest = array_slice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            array_copy(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ec_stream_type_file:
            return fwrite(&item, size, 1, stream->type_data.file.file) == 1;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <c_array_mut tp_type>
    [[nodiscard]] t_b8 stream_read_items_into_array(t_stream *const stream, const tp_type arr, const t_i32 cnt) {
        ZF_ASSERT(stream->mode == ec_stream_mode_read);
        ZF_ASSERT(cnt >= 0 && cnt <= arr.len);

        if (cnt == 0) {
            return true;
        }

        switch (stream->type) {
        case ec_stream_type_mem: {
            const t_i32 size = ZF_SIZE_OF(arr[0]) * cnt;

            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = array_slice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            const auto dest = mem::get_array_as_byte_array(arr);
            array_copy(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ec_stream_type_file:
            return static_cast<t_i32>(fread(arr.raw, sizeof(arr[0]), static_cast<size_t>(cnt), stream->type_data.file.file)) == cnt;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <c_array tp_type>
    [[nodiscard]] t_b8 stream_write_items_of_array(t_stream *const stream, const tp_type arr) {
        ZF_ASSERT(stream->mode == ec_stream_mode_write);

        if (arr.len == 0) {
            return true;
        }

        switch (stream->type) {
        case ec_stream_type_mem: {
            const t_i32 size = array_get_size_in_bytes(arr);

            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = mem::get_array_as_byte_array(arr);
            const auto dest = array_slice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            array_copy(src, dest);

            stream->type_data.mem.byte_pos += size;

            return true;
        }

        case ec_stream_type_file:
            return static_cast<t_i32>(fwrite(arr.raw, sizeof(arr[0]), static_cast<size_t>(arr.len), stream->type_data.file.file)) == arr.len;

        default:
            ZF_UNREACHABLE();
        }
    }

    template <c_array tp_type>
    [[nodiscard]] t_b8 stream_serialize_array(t_stream *const stream, const tp_type arr) {
        if (!stream_write_item(stream, arr.len)) {
            return false;
        }

        if (!stream_write_items_of_array(stream, arr)) {
            return false;
        }

        return true;
    }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 stream_deserialize_array(t_stream *const stream, mem::t_arena *const arr_arena, t_array_mut<tp_type> *const o_arr) {
        t_i32 len;

        if (!stream_read_item(stream, &len)) {
            return false;
        }

        *o_arr = mem::arena_push_array<tp_type>(arr_arena, len);

        if (!stream_read_items_into_array(stream, *o_arr, len)) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 stream_serialize_bitset(t_stream *const stream, const mem::t_bitset_rdonly bv) {
        if (!stream_write_item(stream, bv.bit_cnt)) {
            return false;
        }

        if (!stream_write_items_of_array(stream, mem::bitset_get_bytes(bv))) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 stream_deserialize_bitset(t_stream *const stream, mem::t_arena *const bv_arena, mem::t_bitset_mut *const o_bv) {
        t_i32 bit_cnt;

        if (!stream_read_item(stream, &bit_cnt)) {
            return false;
        }

        *o_bv = mem::bitset_create(bit_cnt, bv_arena);

        if (!stream_read_items_into_array(stream, mem::bitset_get_bytes(*o_bv), mem::bitset_get_bytes(*o_bv).len)) {
            return false;
        }

        return true;
    }

    // ============================================================


    // ============================================================
    // @section: Files and Directories

    enum t_file_access_mode : t_i32 {
        ec_file_access_mode_read,
        ec_file_access_mode_write,
        ec_file_access_mode_append
    };

    [[nodiscard]] t_b8 file_open(const strs::t_str_rdonly file_path, const t_file_access_mode mode, mem::t_arena *const temp_arena, t_stream *const o_stream);
    void file_close(t_stream *const stream);
    t_i32 file_get_size(t_stream *const stream);
    [[nodiscard]] t_b8 file_load_contents(const strs::t_str_rdonly file_path, mem::t_arena *const contents_arena, mem::t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator = false);

    enum t_directory_creation_result : t_i32 {
        ec_directory_creation_result_success,
        ec_directory_creation_result_already_exists,
        ec_directory_creation_result_permission_denied,
        ec_directory_creation_result_path_not_found,
        ec_directory_creation_result_unknown_err
    };

    [[nodiscard]] t_b8 create_directory(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_creation_res = nullptr);
    [[nodiscard]] t_b8 create_directory_and_parents(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res = nullptr);
    [[nodiscard]] t_b8 create_file_and_parent_directories(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res = nullptr);

    enum t_path_type : t_i32 {
        ec_path_type_not_found,
        ec_path_type_file,
        ec_path_type_directory
    };

    t_path_type path_get_type(const strs::t_str_rdonly path, mem::t_arena *const temp_arena);

    strs::t_str_mut get_executable_directory(mem::t_arena *const arena);

    // ============================================================


    // ============================================================
    // @section: Printing

    inline t_b8 print(t_stream *const stream, const strs::t_str_rdonly str) {
        return stream_write_items_of_array(stream, str.bytes);
    }

    inline t_b8 print_format(t_stream *const stream, const strs::t_str_rdonly format);

    template <c_simple tp_arg_type, c_simple... tp_arg_types_leftover>
    t_b8 print_format(t_stream *const stream, const strs::t_str_rdonly format, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover);

    // Type format structs which are to be accepted as format printing arguments need to meet this (i.e. have the tag).
    template <typename tp_type>
    concept c_format = requires { typename tp_type::t_format_tag; };


    // ========================================
    // @subsection: Bool Printing

    struct t_bool_format {
        using t_format_tag = void;
        t_b8 value;
    };

    inline t_bool_format format_bool(const t_b8 value) { return {value}; }
    inline t_bool_format format_default(const t_b8 value) { return {value}; }

    inline t_b8 print_type(t_stream *const stream, const t_bool_format format) {
        const strs::t_str_rdonly true_str = ZF_STR_LITERAL("true");
        const strs::t_str_rdonly false_str = ZF_STR_LITERAL("false");

        return print(stream, format.value ? true_str : false_str);
    }

    // ========================================


    // ========================================
    // @subsection: String Printing

    struct t_str_format {
        using t_format_tag = void;
        strs::t_str_rdonly value;
    };

    inline t_str_format format_str(const strs::t_str_rdonly value) { return {value}; }
    inline t_str_format format_default(const strs::t_str_rdonly value) { return format_str(value); }

    inline t_b8 print_type(t_stream *const stream, const t_str_format format) {
        return print(stream, format.value);
    }

    // @todo: Char printing too?

    // ========================================


    // ========================================
    // @subsection: Integer Printing

    template <c_integral tp_type>
    struct t_integral_format {
        using t_format_tag = void;
        tp_type value;
    };

    template <c_integral tp_type> t_integral_format<tp_type> format_int(const tp_type value) { return {value}; }
    template <c_integral tp_type> t_integral_format<tp_type> format_default(const tp_type value) { return format_int(value); }

    template <c_integral tp_type>
    t_b8 print_type(t_stream *const stream, const t_integral_format<tp_type> format) {
        t_static_array<t_u8, 20> str_bytes = {}; // Maximum possible number of ASCII characters needed to represent a 64-bit integer.
        t_stream str_bytes_stream = mem_stream_create(array_get_as_nonstatic(str_bytes), ec_stream_mode_write);
        t_b8 str_bytes_stream_write_success = true;

        if (format.value < 0) {
            str_bytes_stream_write_success = stream_write_item(&str_bytes_stream, '-');
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        const t_i32 dig_cnt = math::get_digit_cnt(format.value);

        for (t_i32 i = 0; i < dig_cnt; i++) {
            const auto byte = static_cast<t_u8>('0' + math::get_digit_at(format.value, dig_cnt - 1 - i));
            str_bytes_stream_write_success = stream_write_item(&str_bytes_stream, byte);
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        return print(stream, {mem_stream_get_bytes_written(&str_bytes_stream)});
    }

    // ========================================


    // ========================================
    // @subsection: Float Printing

    template <c_floating_point tp_type>
    struct t_float_format {
        using t_format_tag = void;

        tp_type value;
        t_b8 trim_trailing_zeros;
    };

    template <c_floating_point tp_type>
    t_float_format<tp_type> format_float(const tp_type value, const t_b8 trim_trailing_zeros = false) {
        return {value, trim_trailing_zeros};
    }

    template <c_floating_point tp_type>
    t_float_format<tp_type> format_default(const tp_type value) {
        return format_float(value);
    }

    template <c_floating_point tp_type>
    t_b8 print_type(t_stream *const stream, const t_float_format<tp_type> format) {
        t_static_array<t_u8, 400> str_bytes = {}; // Roughly more than how many bytes should ever be needed.

        t_i32 str_bytes_used = snprintf(reinterpret_cast<char *>(str_bytes.raw), str_bytes.g_len, "%f", static_cast<t_f64>(format.value));

        if (str_bytes_used < 0 || str_bytes_used >= str_bytes.g_len) {
            return false;
        }

        if (format.trim_trailing_zeros) {
            const auto str_bytes_relevant = array_slice(array_get_as_nonstatic(str_bytes), 0, str_bytes_used);

            if (array_do_any_equal(str_bytes_relevant, '.')) {
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

        return print(stream, {array_slice(array_get_as_nonstatic(str_bytes), 0, str_bytes_used)});
    }

    // ========================================


    // ========================================
    // @subsection: Hex Printing

    enum t_hex_format_flags : t_i32 {
        ec_hex_format_flags_none = 0,
        ec_hex_format_flags_omit_prefix = 1 << 0,
        ec_hex_format_flags_lower_case = 1 << 1,
        ec_hex_format_flags_allow_odd_digit_cnt = 1 << 2
    };

    constexpr t_i32 g_hex_format_digit_cnt_min = 1;
    constexpr t_i32 g_hex_format_digit_cnt_max = 16;

    template <c_integral_unsigned tp_type>
    struct t_hex_format {
        using t_format_tag = void;

        tp_type value;
        t_hex_format_flags flags;
        t_i32 min_digits; // Will be rounded UP to the next even if this is odd and the flag for allowing an odd digit count is unset.
    };

    template <c_integral_unsigned tp_type>
    t_hex_format<tp_type> format_hex(const tp_type value, const t_hex_format_flags flags = {}, const t_i32 min_digits = g_hex_format_digit_cnt_min) {
        return {value, flags, min_digits};
    }

    inline t_hex_format<t_uintptr> format_hex(const void *const ptr, const t_hex_format_flags flags = {}, const t_i32 min_digits = g_hex_format_digit_cnt_min) {
        return {reinterpret_cast<t_uintptr>(ptr), flags, min_digits};
    }

    inline t_hex_format<t_uintptr> format_default(const void *const ptr) {
        return format_hex(ptr, {}, 2 * ZF_SIZE_OF(t_uintptr));
    }

    template <c_integral_unsigned tp_type>
    t_b8 print_type(t_stream *const stream, const t_hex_format<tp_type> format) {
        ZF_ASSERT(format.min_digits >= g_hex_format_digit_cnt_min && format.min_digits <= g_hex_format_digit_cnt_max);

        t_static_array<t_u8, 2 + g_hex_format_digit_cnt_max> str_bytes = {}; // Can facilitate max number of digits plus the "0x" prefix.
        t_stream str_bytes_stream = mem_stream_create(array_get_as_nonstatic(str_bytes), ec_stream_mode_write);

        t_b8 str_bytes_stream_write_success = true;

        if (!(format.flags & ec_hex_format_flags_omit_prefix)) {
            str_bytes_stream_write_success = stream_write_item(&str_bytes_stream, '0');
            ZF_ASSERT(str_bytes_stream_write_success);

            str_bytes_stream_write_success = stream_write_item(&str_bytes_stream, 'x');
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        const t_i32 str_bytes_digits_begin_pos = str_bytes_stream.type_data.mem.byte_pos;

        const auto dig_to_byte = [flags = format.flags](const t_i32 dig) -> t_u8 {
            if (dig < 10) {
                return static_cast<t_u8>('0' + dig);
            } else {
                if (flags & ec_hex_format_flags_lower_case) {
                    return static_cast<t_u8>('a' + dig - 10);
                } else {
                    return static_cast<t_u8>('A' + dig - 10);
                }
            }
        };

        auto value_mut = format.value;

        t_i32 cnter = 0;
        const t_i32 inner_loop_cnt = (format.flags & ec_hex_format_flags_allow_odd_digit_cnt) ? 1 : 2;

        do {
            for (t_i32 i = 0; i < inner_loop_cnt; i++) {
                const auto byte = dig_to_byte(value_mut % 16);
                str_bytes_stream_write_success = stream_write_item(&str_bytes_stream, byte);
                ZF_ASSERT(str_bytes_stream_write_success);

                value_mut /= 16;

                cnter++;
            }
        } while (value_mut != 0 || cnter < format.min_digits);

        const auto str_bytes_digits = array_slice_from(mem_stream_get_bytes_written(&str_bytes_stream), str_bytes_digits_begin_pos);
        array_reverse(str_bytes_digits);

        return print(stream, {mem_stream_get_bytes_written(&str_bytes_stream)});
    }

    // ========================================


    // ========================================
    // @subsection: V2 Printing

    struct t_v2_format {
        using t_format_tag = void;

        math::t_v2 value;
        t_b8 trim_trailing_zeros;
    };

    inline t_v2_format format_v2(const math::t_v2 value, const t_b8 trim_trailing_zeros = false) { return {value, trim_trailing_zeros}; }
    inline t_v2_format format_default(const math::t_v2 value) { return format_v2(value); }

    inline t_b8 print_type(t_stream *const stream, const t_v2_format format) {
        return print(stream, ZF_STR_LITERAL("("))
            && print_type(stream, format_float(format.value.x, format.trim_trailing_zeros))
            && print(stream, ZF_STR_LITERAL(", "))
            && print_type(stream, format_float(format.value.y, format.trim_trailing_zeros))
            && print(stream, ZF_STR_LITERAL(")"));
    }

    struct t_v2_i_format {
        using t_format_tag = void;

        math::t_v2_i value;
    };

    inline t_v2_i_format format_v2(const math::t_v2_i value) { return {value}; }
    inline t_v2_i_format format_default(const math::t_v2_i value) { return format_v2(value); }

    inline t_b8 print_type(t_stream *const stream, const t_v2_i_format format) {
        return print(stream, ZF_STR_LITERAL("("))
            && print_type(stream, format_int(format.value.x))
            && print(stream, ZF_STR_LITERAL(", "))
            && print_type(stream, format_int(format.value.y))
            && print(stream, ZF_STR_LITERAL(")"));
    }

    // ========================================


    // ========================================
    // @subsection: Array Printing

    template <typename tp_arr_type>
    concept c_formattable_array = c_array<tp_arr_type>
        && requires(const typename tp_arr_type::t_elem &v) { { format_default(v) } -> c_format; };

    template <c_formattable_array tp_arr_type>
    struct t_array_format {
        using t_format_tag = void;

        tp_arr_type value;
        t_b8 one_per_line;
    };

    template <c_formattable_array tp_arr_type>
    t_array_format<tp_arr_type> format_array(const tp_arr_type value, const t_b8 one_per_line = false) {
        return {value, one_per_line};
    }

    template <c_formattable_array tp_arr_type>
    t_array_format<tp_arr_type> format_default(const tp_arr_type value) {
        return {value};
    }

    template <c_formattable_array tp_arr_type>
    t_b8 print_type(t_stream *const stream, const t_array_format<tp_arr_type> format) {
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

    // ========================================


    // ========================================
    // @subsection: Bit Vector Printing

    enum t_bitset_format_style : t_i32 {
        ec_bitset_format_style_seq = 0,                // List all bits from LSB to MSB, not divided into bytes.
        ec_bitset_format_style_little_endian = 1 << 0, // Split into bytes, ordered in little endian.
        ec_bitset_format_style_big_endian = 1 << 1     // Split into bytes, ordered in big endian.
    };

    struct t_bitset_format {
        using t_format_tag = void;

        mem::t_bitset_rdonly value;
        t_bitset_format_style style;
    };

    inline t_bitset_format format_bitset(const mem::t_bitset_rdonly &value, const t_bitset_format_style style) { return {value, style}; }
    inline t_bitset_format format_default(const mem::t_bitset_rdonly &value) { return format_bitset(value, ec_bitset_format_style_seq); }

    inline t_b8 print_type(t_stream *const stream, const t_bitset_format format) {
        const auto print_bit = [&](const t_i32 bit_index) {
            const strs::t_str_rdonly str = mem::is_bit_set(format.value, bit_index) ? ZF_STR_LITERAL("1") : ZF_STR_LITERAL("0");
            return print(stream, str);
        };

        const auto print_byte = [&](const t_i32 index) {
            const t_i32 bit_cnt = index == mem::bitset_get_bytes(format.value).len - 1 ? mem::bitset_get_last_byte_bit_cnt(format.value) : 8;

            for (t_i32 i = 7; i >= bit_cnt; i--) {
                print(stream, ZF_STR_LITERAL("0"));
            }

            for (t_i32 i = bit_cnt - 1; i >= 0; i--) {
                print_bit((index * 8) + i);
            }
        };

        switch (format.style) {
        case ec_bitset_format_style_seq:
            for (t_i32 i = 0; i < format.value.bit_cnt; i++) {
                if (!print_bit(i)) {
                    return false;
                }
            }

            break;

        case ec_bitset_format_style_little_endian:
            for (t_i32 i = 0; i < mem::bitset_get_bytes(format.value).len; i++) {
                if (i > 0) {
                    print(stream, ZF_STR_LITERAL(" "));
                }

                print_byte(i);
            }

            break;

        case ec_bitset_format_style_big_endian:
            for (t_i32 i = mem::bitset_get_bytes(format.value).len - 1; i >= 0; i--) {
                print_byte(i);

                if (i > 0) {
                    print(stream, ZF_STR_LITERAL(" "));
                }
            }

            break;
        }

        return true;
    }

    // ========================================


    // ========================================
    // @subsection: Format Printing

    constexpr strs::t_code_pt g_print_format_spec = '%';
    constexpr strs::t_code_pt g_print_format_esc = '^';

    inline t_i32 count_format_specs(const strs::t_str_rdonly str) {
        static_assert(strs::f_is_code_pt_ascii(g_print_format_spec) && strs::f_is_code_pt_ascii(g_print_format_esc)); // Assuming this for this algorithm.

        t_b8 escaped = false;
        t_i32 cnt = 0;

        for (t_i32 i = 0; i < str.bytes.len; i++) {
            if (!escaped) {
                if (str.bytes[i] == g_print_format_esc) {
                    escaped = true;
                } else if (str.bytes[i] == g_print_format_spec) {
                    cnt++;
                }
            } else {
                escaped = false;
            }
        }

        return cnt;
    }

    inline t_b8 print_format(t_stream *const stream, const strs::t_str_rdonly format) {
        ZF_ASSERT(count_format_specs(format) == 0);

        // Just print the rest of the string.
        return print(stream, format);
    }

    // Use a single '%' as the format specifier. To actually include a '%' in the output, write "^%". To actually include a '^', write "^^".
    // Returns true iff the operation was successful.
    template <c_simple tp_arg_type, c_simple... tp_arg_types_leftover>
    t_b8 print_format(t_stream *const stream, const strs::t_str_rdonly format, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover) {
        static_assert(!c_cstr<tp_arg_type>, "C-strings are prohibited for default formatting as a form of error prevention.");

        ZF_ASSERT(count_format_specs(format) == 1 + sizeof...(args_leftover));

        static_assert(strs::f_is_code_pt_ascii(g_print_format_spec) && strs::f_is_code_pt_ascii(g_print_format_esc)); // Assuming this for this algorithm.

        t_b8 escaped = false;

        for (t_i32 i = 0; i < format.bytes.len; i++) {
            if (!escaped) {
                if (format.bytes[i] == g_print_format_esc) {
                    escaped = true;
                    continue;
                } else if (format.bytes[i] == g_print_format_spec) {
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

    // ========================================


    // ========================================
    // @subsection: Logging Helpers

    template <c_simple... tp_arg_types>
    t_b8 log(const strs::t_str_rdonly format, const tp_arg_types &...args) {
        t_stream std_err = get_std_out();

        if (!print_format(&std_err, format, args...)) {
            return false;
        }

        if (!print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    template <c_simple... tp_arg_types>
    t_b8 log_error(const strs::t_str_rdonly format, const tp_arg_types &...args) {
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

        return true;
    }

    template <c_simple... tp_arg_types>
    t_b8 log_error_type(const strs::t_str_rdonly type_name, const strs::t_str_rdonly format, const tp_arg_types &...args) {
        ZF_ASSERT(!strs::f_is_empty(type_name));

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

        return true;
    }

    template <c_simple... tp_arg_types>
    t_b8 log_warning(const strs::t_str_rdonly format, const tp_arg_types &...args) {
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

        return true;
    }

    // ========================================


    // ============================================================
}
