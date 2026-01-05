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

    inline t_stream f_create_mem_stream(const t_array_mut<t_u8> bytes, const t_stream_mode mode, const t_i32 pos = 0) {
        return {.type = ec_stream_type_mem, .type_data = {.mem = {.bytes = bytes, .byte_pos = pos}}, .mode = mode};
    }

    inline t_array_mut<t_u8> f_get_mem_stream_bytes_written(const t_stream *const stream) {
        ZF_ASSERT(stream->type == ec_stream_type_mem);
        return f_array_slice(stream->type_data.mem.bytes, 0, stream->type_data.mem.byte_pos);
    }

    inline t_stream f_create_file_stream(FILE *const file, const t_stream_mode mode) {
        return {.type = ec_stream_type_file, .type_data = {.file = {.file = file}}, .mode = mode};
    }

    inline t_stream f_get_std_in() { return f_create_file_stream(stdin, ec_stream_mode_read); }
    inline t_stream f_get_std_out() { return f_create_file_stream(stdout, ec_stream_mode_write); }
    inline t_stream f_get_std_error() { return f_create_file_stream(stderr, ec_stream_mode_write); }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 f_read_item(t_stream *const stream, tp_type *const o_item) {
        ZF_ASSERT(stream->mode == ec_stream_mode_read);

        const t_i32 size = ZF_SIZE_OF(tp_type);

        switch (stream->type) {
        case ec_stream_type_mem: {
            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = f_array_slice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            const auto dest = mem::f_get_as_bytes(*o_item);
            f_algos_copy_all(src, dest);

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
    [[nodiscard]] t_b8 f_write_item(t_stream *const stream, const tp_type &item) {
        ZF_ASSERT(stream->mode == ec_stream_mode_write);

        const t_i32 size = ZF_SIZE_OF(tp_type);

        switch (stream->type) {
        case ec_stream_type_mem: {
            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = mem::f_get_as_bytes(item);
            const auto dest = f_array_slice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            f_algos_copy_all(src, dest);

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
    [[nodiscard]] t_b8 f_read_items_into_array(t_stream *const stream, const tp_type arr, const t_i32 cnt) {
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

            const auto src = f_array_slice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            const auto dest = mem::f_get_array_as_byte_array(arr);
            f_algos_copy_all(src, dest);

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
    [[nodiscard]] t_b8 f_write_items_of_array(t_stream *const stream, const tp_type arr) {
        ZF_ASSERT(stream->mode == ec_stream_mode_write);

        if (arr.len == 0) {
            return true;
        }

        switch (stream->type) {
        case ec_stream_type_mem: {
            const t_i32 size = f_array_get_size_in_bytes(arr);

            if (stream->type_data.mem.byte_pos + size > stream->type_data.mem.bytes.len) {
                return false;
            }

            const auto src = mem::f_get_array_as_byte_array(arr);
            const auto dest = f_array_slice(stream->type_data.mem.bytes, stream->type_data.mem.byte_pos, stream->type_data.mem.byte_pos + size);
            f_algos_copy_all(src, dest);

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
    [[nodiscard]] t_b8 f_serialize_array(t_stream *const stream, const tp_type arr) {
        if (!f_write_item(stream, arr.len)) {
            return false;
        }

        if (!f_write_items_of_array(stream, arr)) {
            return false;
        }

        return true;
    }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 f_deserialize_array(t_stream *const stream, mem::t_arena *const arr_arena, t_array_mut<tp_type> *const o_arr) {
        t_i32 len;

        if (!f_read_item(stream, &len)) {
            return false;
        }

        *o_arr = mem::f_arena_push_array<tp_type>(arr_arena, len);

        if (!f_read_items_into_array(stream, *o_arr, len)) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 f_serialize_bit_vec(t_stream *const stream, const mem::t_bitset_rdonly bv) {
        if (!f_write_item(stream, bv.bit_cnt)) {
            return false;
        }

        if (!f_write_items_of_array(stream, mem::f_bitset_get_bytes(bv))) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 f_deserialize_bit_vec(t_stream *const stream, mem::t_arena *const bv_arena, mem::t_bitset_mut *const o_bv) {
        t_i32 bit_cnt;

        if (!f_read_item(stream, &bit_cnt)) {
            return false;
        }

        *o_bv = mem::f_bitset_create(bit_cnt, bv_arena);

        if (!f_read_items_into_array(stream, mem::f_bitset_get_bytes(*o_bv), mem::f_bitset_get_bytes(*o_bv).len)) {
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

    [[nodiscard]] t_b8 f_open_file(const t_str_rdonly file_path, const t_file_access_mode mode, mem::t_arena *const temp_arena, t_stream *const o_stream);
    void f_close_file(t_stream *const stream);
    t_i32 f_calc_file_size(t_stream *const stream);
    [[nodiscard]] t_b8 f_load_file_contents(const t_str_rdonly file_path, mem::t_arena *const contents_arena, mem::t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator = false);

    enum t_directory_creation_result : t_i32 {
        ec_directory_creation_result_success,
        ec_directory_creation_result_already_exists,
        ec_directory_creation_result_permission_denied,
        ec_directory_creation_result_path_not_found,
        ec_directory_creation_result_unknown_err
    };

    [[nodiscard]] t_b8 f_create_directory(const t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_creation_res = nullptr);
    [[nodiscard]] t_b8 f_create_directory_and_parents(const t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res = nullptr);
    [[nodiscard]] t_b8 f_create_file_and_parent_directories(const t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res = nullptr);

    enum t_path_type : t_i32 {
        ec_path_type_not_found,
        ec_path_type_file,
        ec_path_type_directory
    };

    t_path_type f_get_path_type(const t_str_rdonly path, mem::t_arena *const temp_arena);

    t_str_mut f_get_executable_directory(mem::t_arena *const arena);

    // ============================================================


    // ============================================================
    // @section: Printing

    inline t_b8 f_print(t_stream *const stream, const t_str_rdonly str) {
        return f_write_items_of_array(stream, str.bytes);
    }

    inline t_b8 f_print_fmt(t_stream *const stream, const t_str_rdonly fmt);

    template <c_simple tp_arg_type, c_simple... tp_arg_types_leftover>
    t_b8 f_print_fmt(t_stream *const stream, const t_str_rdonly fmt, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover);

    // Type format structs which are to be accepted as format printing arguments need to meet this (i.e. have the tag).
    template <typename tp_type>
    concept c_fmt = requires { typename tp_type::t_fmt_tag; };


    // ========================================
    // @subsection: Bool Printing

    struct t_bool_fmt {
        using t_fmt_tag = void;
        t_b8 value;
    };

    inline t_bool_fmt f_fmt_bool(const t_b8 value) { return {value}; }
    inline t_bool_fmt f_fmt_default(const t_b8 value) { return {value}; }

    inline t_b8 f_print_type(t_stream *const stream, const t_bool_fmt fmt) {
        const t_str_rdonly true_str = ZF_STR_LITERAL("true");
        const t_str_rdonly false_str = ZF_STR_LITERAL("false");

        return f_print(stream, fmt.value ? true_str : false_str);
    }

    // ========================================


    // ========================================
    // @subsection: String Printing

    struct t_str_fmt {
        using t_fmt_tag = void;
        t_str_rdonly value;
    };

    inline t_str_fmt f_fmt_str(const t_str_rdonly value) { return {value}; }
    inline t_str_fmt f_fmt_default(const t_str_rdonly value) { return f_fmt_str(value); }

    inline t_b8 f_print_type(t_stream *const stream, const t_str_fmt fmt) {
        return f_print(stream, fmt.value);
    }

    // @todo: Char printing too?

    // ========================================


    // ========================================
    // @subsection: Integer Printing

    template <c_integral tp_type>
    struct t_integral_fmt {
        using t_fmt_tag = void;
        tp_type value;
    };

    template <c_integral tp_type> t_integral_fmt<tp_type> f_fmt_int(const tp_type value) { return {value}; }
    template <c_integral tp_type> t_integral_fmt<tp_type> f_fmt_default(const tp_type value) { return f_fmt_int(value); }

    template <c_integral tp_type>
    t_b8 f_print_type(t_stream *const stream, const t_integral_fmt<tp_type> fmt) {
        t_static_array<t_u8, 20> str_bytes = {}; // Maximum possible number of ASCII characters needed to represent a 64-bit integer.
        t_stream str_bytes_stream = f_create_mem_stream(f_array_get_as_nonstatic(str_bytes), ec_stream_mode_write);
        t_b8 str_bytes_stream_write_success = true;

        if (fmt.value < 0) {
            str_bytes_stream_write_success = f_write_item(&str_bytes_stream, '-');
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        const t_i32 dig_cnt = f_math_calc_digit_cnt(fmt.value);

        for (t_i32 i = 0; i < dig_cnt; i++) {
            const auto byte = static_cast<t_u8>('0' + f_math_determine_digit_at(fmt.value, dig_cnt - 1 - i));
            str_bytes_stream_write_success = f_write_item(&str_bytes_stream, byte);
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        return f_print(stream, {f_get_mem_stream_bytes_written(&str_bytes_stream)});
    }

    // ========================================


    // ========================================
    // @subsection: Float Printing

    template <c_floating_point tp_type>
    struct t_float_fmt {
        using t_fmt_tag = void;

        tp_type value;
        t_b8 trim_trailing_zeros;
    };

    template <c_floating_point tp_type>
    t_float_fmt<tp_type> f_fmt_float(const tp_type value, const t_b8 trim_trailing_zeros = false) {
        return {value, trim_trailing_zeros};
    }

    template <c_floating_point tp_type>
    t_float_fmt<tp_type> f_fmt_default(const tp_type value) {
        return f_fmt_float(value);
    }

    template <c_floating_point tp_type>
    t_b8 f_print_type(t_stream *const stream, const t_float_fmt<tp_type> fmt) {
        t_static_array<t_u8, 400> str_bytes = {}; // Roughly more than how many bytes should ever be needed.

        t_i32 str_bytes_used = snprintf(reinterpret_cast<char *>(str_bytes.raw), str_bytes.g_len, "%f", static_cast<t_f64>(fmt.value));

        if (str_bytes_used < 0 || str_bytes_used >= str_bytes.g_len) {
            return false;
        }

        if (fmt.trim_trailing_zeros) {
            const auto str_bytes_relevant = f_array_slice(f_array_get_as_nonstatic(str_bytes), 0, str_bytes_used);

            if (f_algos_do_any_equal(str_bytes_relevant, '.')) {
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

        return f_print(stream, {f_array_slice(f_array_get_as_nonstatic(str_bytes), 0, str_bytes_used)});
    }

    // ========================================


    // ========================================
    // @subsection: Hex Printing

    enum t_hex_fmt_flags : t_i32 {
        ec_hex_fmt_flags_none = 0,
        ec_hex_fmt_flags_omit_prefix = 1 << 0,
        ec_hex_fmt_flags_lower_case = 1 << 1,
        ec_hex_fmt_flags_allow_odd_digit_cnt = 1 << 2
    };

    constexpr t_i32 g_hex_fmt_digit_cnt_min = 1;
    constexpr t_i32 g_hex_fmt_digit_cnt_max = 16;

    template <c_integral_unsigned tp_type>
    struct t_hex_fmt {
        using t_fmt_tag = void;

        tp_type value;
        t_hex_fmt_flags flags;
        t_i32 min_digits; // Will be rounded UP to the next even if this is odd and the flag for allowing an odd digit count is unset.
    };

    template <c_integral_unsigned tp_type>
    t_hex_fmt<tp_type> f_fmt_hex(const tp_type value, const t_hex_fmt_flags flags = {}, const t_i32 min_digits = g_hex_fmt_digit_cnt_min) {
        return {value, flags, min_digits};
    }

    inline t_hex_fmt<t_uintptr> f_fmt_hex(const void *const ptr, const t_hex_fmt_flags flags = {}, const t_i32 min_digits = g_hex_fmt_digit_cnt_min) {
        return {reinterpret_cast<t_uintptr>(ptr), flags, min_digits};
    }

    inline t_hex_fmt<t_uintptr> f_fmt_default(const void *const ptr) {
        return f_fmt_hex(ptr, {}, 2 * ZF_SIZE_OF(t_uintptr));
    }

    template <c_integral_unsigned tp_type>
    t_b8 f_print_type(t_stream *const stream, const t_hex_fmt<tp_type> fmt) {
        ZF_ASSERT(fmt.min_digits >= g_hex_fmt_digit_cnt_min && fmt.min_digits <= g_hex_fmt_digit_cnt_max);

        t_static_array<t_u8, 2 + g_hex_fmt_digit_cnt_max> str_bytes = {}; // Can facilitate max number of digits plus the "0x" prefix.
        t_stream str_bytes_stream = f_create_mem_stream(f_array_get_as_nonstatic(str_bytes), ec_stream_mode_write);

        t_b8 str_bytes_stream_write_success = true;

        if (!(fmt.flags & ec_hex_fmt_flags_omit_prefix)) {
            str_bytes_stream_write_success = f_write_item(&str_bytes_stream, '0');
            ZF_ASSERT(str_bytes_stream_write_success);

            str_bytes_stream_write_success = f_write_item(&str_bytes_stream, 'x');
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        const t_i32 str_bytes_digits_begin_pos = str_bytes_stream.type_data.mem.byte_pos;

        const auto dig_to_byte = [flags = fmt.flags](const t_i32 dig) -> t_u8 {
            if (dig < 10) {
                return static_cast<t_u8>('0' + dig);
            } else {
                if (flags & ec_hex_fmt_flags_lower_case) {
                    return static_cast<t_u8>('a' + dig - 10);
                } else {
                    return static_cast<t_u8>('A' + dig - 10);
                }
            }
        };

        auto value_mut = fmt.value;

        t_i32 cnter = 0;
        const t_i32 inner_loop_cnt = (fmt.flags & ec_hex_fmt_flags_allow_odd_digit_cnt) ? 1 : 2;

        do {
            for (t_i32 i = 0; i < inner_loop_cnt; i++) {
                const auto byte = dig_to_byte(value_mut % 16);
                str_bytes_stream_write_success = f_write_item(&str_bytes_stream, byte);
                ZF_ASSERT(str_bytes_stream_write_success);

                value_mut /= 16;

                cnter++;
            }
        } while (value_mut != 0 || cnter < fmt.min_digits);

        const auto str_bytes_digits = f_array_slice_from(f_get_mem_stream_bytes_written(&str_bytes_stream), str_bytes_digits_begin_pos);
        f_algos_reverse(str_bytes_digits);

        return f_print(stream, {f_get_mem_stream_bytes_written(&str_bytes_stream)});
    }

    // ========================================


    // ========================================
    // @subsection: V2 Printing

    struct t_v2_fmt {
        using t_fmt_tag = void;

        t_v2 value;
        t_b8 trim_trailing_zeros;
    };

    inline t_v2_fmt f_fmt_v2(const t_v2 value, const t_b8 trim_trailing_zeros = false) { return {value, trim_trailing_zeros}; }
    inline t_v2_fmt f_fmt_default(const t_v2 value) { return f_fmt_v2(value); }

    inline t_b8 f_print_type(t_stream *const stream, const t_v2_fmt fmt) {
        return f_print(stream, ZF_STR_LITERAL("("))
            && f_print_type(stream, f_fmt_float(fmt.value.x, fmt.trim_trailing_zeros))
            && f_print(stream, ZF_STR_LITERAL(", "))
            && f_print_type(stream, f_fmt_float(fmt.value.y, fmt.trim_trailing_zeros))
            && f_print(stream, ZF_STR_LITERAL(")"));
    }

    struct t_v2_i_fmt {
        using t_fmt_tag = void;

        t_v2_i value;
    };

    inline t_v2_i_fmt f_fmt_v2(const t_v2_i value) { return {value}; }
    inline t_v2_i_fmt f_fmt_default(const t_v2_i value) { return f_fmt_v2(value); }

    inline t_b8 f_print_type(t_stream *const stream, const t_v2_i_fmt fmt) {
        return f_print(stream, ZF_STR_LITERAL("("))
            && f_print_type(stream, f_fmt_int(fmt.value.x))
            && f_print(stream, ZF_STR_LITERAL(", "))
            && f_print_type(stream, f_fmt_int(fmt.value.y))
            && f_print(stream, ZF_STR_LITERAL(")"));
    }

    // ========================================


    // ========================================
    // @subsection: Array Printing

    template <typename tp_arr_type>
    concept c_formattable_array = c_array<tp_arr_type>
        && requires(const typename tp_arr_type::t_elem &v) { { f_fmt_default(v) } -> c_fmt; };

    template <c_formattable_array tp_arr_type>
    struct t_array_fmt {
        using t_fmt_tag = void;

        tp_arr_type value;
        t_b8 one_per_line;
    };

    template <c_formattable_array tp_arr_type>
    t_array_fmt<tp_arr_type> f_fmt_array(const tp_arr_type value, const t_b8 one_per_line = false) {
        return {value, one_per_line};
    }

    template <c_formattable_array tp_arr_type>
    t_array_fmt<tp_arr_type> f_fmt_default(const tp_arr_type value) {
        return {value};
    }

    template <c_formattable_array tp_arr_type>
    t_b8 f_print_type(t_stream *const stream, const t_array_fmt<tp_arr_type> fmt) {
        if (fmt.one_per_line) {
            for (t_i32 i = 0; i < fmt.value.len; i++) {
                if (!f_print_fmt(stream, ZF_STR_LITERAL("[%] %%"), i, fmt.value[i], i < fmt.value.len - 1 ? ZF_STR_LITERAL("\n") : ZF_STR_LITERAL(""))) {
                    return false;
                }
            }
        } else {
            if (!f_print(stream, ZF_STR_LITERAL("["))) {
                return false;
            }

            for (t_i32 i = 0; i < fmt.value.len; i++) {
                if (!f_print_fmt(stream, ZF_STR_LITERAL("%"), fmt.value[i])) {
                    return false;
                }

                if (i < fmt.value.len - 1) {
                    if (!f_print(stream, ZF_STR_LITERAL(", "))) {
                        return false;
                    }
                }
            }

            if (!f_print(stream, ZF_STR_LITERAL("]"))) {
                return false;
            }
        }

        return true;
    }

    // ========================================


    // ========================================
    // @subsection: Bit Vector Printing

    enum t_bit_vec_fmt_style : t_i32 {
        ec_bit_vec_fmt_style_seq = 0,                // List all bits from LSB to MSB, not divided into bytes.
        ec_bit_vec_fmt_style_little_endian = 1 << 0, // Split into bytes, ordered in little endian.
        ec_bit_vec_fmt_style_big_endian = 1 << 1     // Split into bytes, ordered in big endian.
    };

    struct t_bit_vec_fmt {
        using t_fmt_tag = void;

        mem::t_bitset_rdonly value;
        t_bit_vec_fmt_style style;
    };

    inline t_bit_vec_fmt f_fmt_bit_vec(const mem::t_bitset_rdonly &value, const t_bit_vec_fmt_style style) { return {value, style}; }
    inline t_bit_vec_fmt f_fmt_default(const mem::t_bitset_rdonly &value) { return f_fmt_bit_vec(value, ec_bit_vec_fmt_style_seq); }

    inline t_b8 f_print_type(t_stream *const stream, const t_bit_vec_fmt fmt) {
        const auto print_bit = [&](const t_i32 bit_index) {
            const t_str_rdonly str = mem::f_is_bit_set(fmt.value, bit_index) ? ZF_STR_LITERAL("1") : ZF_STR_LITERAL("0");
            return f_print(stream, str);
        };

        const auto print_byte = [&](const t_i32 index) {
            const t_i32 bit_cnt = index == mem::f_bitset_get_bytes(fmt.value).len - 1 ? mem::f_bitset_get_last_byte_bit_cnt(fmt.value) : 8;

            for (t_i32 i = 7; i >= bit_cnt; i--) {
                f_print(stream, ZF_STR_LITERAL("0"));
            }

            for (t_i32 i = bit_cnt - 1; i >= 0; i--) {
                print_bit((index * 8) + i);
            }
        };

        switch (fmt.style) {
        case ec_bit_vec_fmt_style_seq:
            for (t_i32 i = 0; i < fmt.value.bit_cnt; i++) {
                if (!print_bit(i)) {
                    return false;
                }
            }

            break;

        case ec_bit_vec_fmt_style_little_endian:
            for (t_i32 i = 0; i < mem::f_bitset_get_bytes(fmt.value).len; i++) {
                if (i > 0) {
                    f_print(stream, ZF_STR_LITERAL(" "));
                }

                print_byte(i);
            }

            break;

        case ec_bit_vec_fmt_style_big_endian:
            for (t_i32 i = mem::f_bitset_get_bytes(fmt.value).len - 1; i >= 0; i--) {
                print_byte(i);

                if (i > 0) {
                    f_print(stream, ZF_STR_LITERAL(" "));
                }
            }

            break;
        }

        return true;
    }

    // ========================================


    // ========================================
    // @subsection: Format Printing

    constexpr t_code_pt g_print_fmt_spec = '%';
    constexpr t_code_pt g_print_fmt_esc = '^';

    inline t_i32 f_cnt_fmt_specs(const t_str_rdonly str) {
        static_assert(f_strs_is_code_pt_ascii(g_print_fmt_spec) && f_strs_is_code_pt_ascii(g_print_fmt_esc)); // Assuming this for this algorithm.

        t_b8 escaped = false;
        t_i32 cnt = 0;

        for (t_i32 i = 0; i < str.bytes.len; i++) {
            if (!escaped) {
                if (str.bytes[i] == g_print_fmt_esc) {
                    escaped = true;
                } else if (str.bytes[i] == g_print_fmt_spec) {
                    cnt++;
                }
            } else {
                escaped = false;
            }
        }

        return cnt;
    }

    inline t_b8 f_print_fmt(t_stream *const stream, const t_str_rdonly fmt) {
        ZF_ASSERT(f_cnt_fmt_specs(fmt) == 0);

        // Just print the rest of the string.
        return f_print(stream, fmt);
    }

    // Use a single '%' as the format specifier. To actually include a '%' in the output, write "^%". To actually include a '^', write "^^".
    // Returns true iff the operation was successful.
    template <c_simple tp_arg_type, c_simple... tp_arg_types_leftover>
    t_b8 f_print_fmt(t_stream *const stream, const t_str_rdonly fmt, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover) {
        static_assert(!c_cstr<tp_arg_type>, "C-strings are prohibited for default formatting as a form of error prevention.");

        ZF_ASSERT(f_cnt_fmt_specs(fmt) == 1 + sizeof...(args_leftover));

        static_assert(f_strs_is_code_pt_ascii(g_print_fmt_spec) && f_strs_is_code_pt_ascii(g_print_fmt_esc)); // Assuming this for this algorithm.

        t_b8 escaped = false;

        for (t_i32 i = 0; i < fmt.bytes.len; i++) {
            if (!escaped) {
                if (fmt.bytes[i] == g_print_fmt_esc) {
                    escaped = true;
                    continue;
                } else if (fmt.bytes[i] == g_print_fmt_spec) {
                    if constexpr (c_fmt<tp_arg_type>) {
                        if (!f_print_type(stream, arg)) {
                            return false;
                        }
                    } else {
                        if (!f_print_type(stream, f_fmt_default(arg))) {
                            return false;
                        }
                    }

                    const t_str_rdonly fmt_leftover = {f_array_slice(fmt.bytes, i + 1, fmt.bytes.len)}; // The substring of everything after the format specifier.
                    return f_print_fmt(stream, fmt_leftover, args_leftover...);
                }
            }

            if (!f_write_item(stream, fmt.bytes[i])) {
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
    t_b8 f_log(const t_str_rdonly fmt, const tp_arg_types &...args) {
        t_stream std_err = f_get_std_out();

        if (!f_print_fmt(&std_err, fmt, args...)) {
            return false;
        }

        if (!f_print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    template <c_simple... tp_arg_types>
    t_b8 f_log_error(const t_str_rdonly fmt, const tp_arg_types &...args) {
        t_stream std_err = f_get_std_error();

        if (!f_print(&std_err, ZF_STR_LITERAL("Error: "))) {
            return false;
        }

        if (!f_print_fmt(&std_err, fmt, args...)) {
            return false;
        }

        if (!f_print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    template <c_simple... tp_arg_types>
    t_b8 f_log_error_type(const t_str_rdonly type_name, const t_str_rdonly fmt, const tp_arg_types &...args) {
        ZF_ASSERT(!f_strs_is_empty(type_name));

        t_stream std_err = f_get_std_error();

        if (!f_print_fmt(&std_err, ZF_STR_LITERAL("% Error: "), type_name)) {
            return false;
        }

        if (!f_print_fmt(&std_err, fmt, args...)) {
            return false;
        }

        if (!f_print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    template <c_simple... tp_arg_types>
    t_b8 f_log_warning(const t_str_rdonly fmt, const tp_arg_types &...args) {
        t_stream std_err = f_get_std_error();

        if (!f_print(&std_err, ZF_STR_LITERAL("Warning: "))) {
            return false;
        }

        if (!f_print_fmt(&std_err, fmt, args...)) {
            return false;
        }

        if (!f_print(&std_err, ZF_STR_LITERAL("\n"))) {
            return false;
        }

        return true;
    }

    // ========================================


    // ============================================================
}
