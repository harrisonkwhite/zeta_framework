#pragma once

#include <cstdio>
#include <zcl/zcl_math.h>
#include <zcl/zcl_algos.h>
#include <zcl/zcl_strs.h>

namespace zf {
    // ============================================================
    // @section: Streams

    enum e_stream_type : t_i32 {
        ek_stream_type_invalid,
        ek_stream_type_mem,
        ek_stream_type_file
    };

    enum e_stream_mode : t_i32 {
        ek_stream_mode_read,
        ek_stream_mode_write
    };

    class c_stream {
    public:
        c_stream() = default;

        c_stream(const c_array_mut<t_u8> bytes, const e_stream_mode mode, const t_i32 pos = 0)
            : m_type(ek_stream_type_mem), m_type_data({.mem = {.bytes_raw = bytes.Raw(), .byte_cnt = bytes.Len(), .byte_index = pos}}), m_mode(mode) { ZF_ASSERT(pos >= 0 && pos <= bytes.Len()); }

        c_stream(FILE *const file, const e_stream_mode mode)
            : m_type(ek_stream_type_file), m_type_data({.file = {.file = file}}), m_mode(mode) { ZF_ASSERT(file); }

        e_stream_type Type() const {
            return m_type;
        }

        e_stream_mode Mode() const {
            ZF_ASSERT(m_type != ek_stream_type_invalid);
            return m_mode;
        }

        t_i32 Pos() const {
            ZF_ASSERT(m_type == ek_stream_type_mem);
            return m_type_data.mem.byte_index;
        }

        c_array_mut<t_u8> Written() const {
            ZF_ASSERT(m_type == ek_stream_type_mem);
            return {m_type_data.mem.bytes_raw, m_type_data.mem.byte_index};
        }

        FILE *File() const {
            ZF_ASSERT(m_type == ek_stream_type_file);
            return m_type_data.file.file;
        }

        template <typename tp_type>
        [[nodiscard]] t_b8 ReadItem(tp_type *const o_item) {
            ZF_ASSERT(m_mode == ek_stream_mode_read);

            constexpr t_i32 size = ZF_SIZE_OF(tp_type);

            switch (m_type) {
            case ek_stream_type_mem: {
                if (m_type_data.mem.byte_index + size > m_type_data.mem.byte_cnt) {
                    return false;
                }

                const auto dest = ToBytes(*o_item);
                const auto src = c_array_rdonly<t_u8>{m_type_data.mem.bytes_raw + m_type_data.mem.byte_index, size};
                Copy(dest, src);

                m_type_data.mem.byte_index += size;

                return true;
            }

            case ek_stream_type_file:
                return fread(o_item, size, 1, m_type_data.file.file) == 1;

            default:
                ZF_UNREACHABLE();
            }
        }

        template <typename tp_type>
        [[nodiscard]] t_b8 WriteItem(const tp_type &item) {
            ZF_ASSERT(m_mode == ek_stream_mode_write);

            constexpr t_i32 size = ZF_SIZE_OF(tp_type);

            switch (m_type) {
            case ek_stream_type_mem: {
                if (m_type_data.mem.byte_index + size > m_type_data.mem.byte_cnt) {
                    return false;
                }

                const auto dest = c_array_mut<t_u8>{m_type_data.mem.bytes_raw + m_type_data.mem.byte_index, size};
                const auto src = ToBytes(item);
                Copy(dest, src);

                m_type_data.mem.byte_index += size;

                return true;
            }

            case ek_stream_type_file:
                return fwrite(&item, size, 1, m_type_data.file.file) == 1;

            default:
                ZF_UNREACHABLE();
            }
        }

        template <co_array_nonstatic_mut tp_type>
        [[nodiscard]] t_b8 ReadItemsIntoArray(const tp_type arr, const t_i32 cnt) {
            ZF_ASSERT(m_mode == ek_stream_mode_read);
            ZF_ASSERT(cnt >= 0 && cnt <= arr.Len());

            if (cnt == 0) {
                return true;
            }

            switch (m_type) {
            case ek_stream_type_mem: {
                const t_i32 size = ZF_SIZE_OF(arr[0]) * cnt;

                if (m_type_data.mem.byte_index + size > m_type_data.mem.byte_cnt) {
                    return false;
                }

                const auto dest = arr.AsByteArray();
                const auto src = c_array_rdonly<t_u8>{m_type_data.mem.bytes_raw + m_type_data.mem.byte_index, size};
                Copy(dest, src);

                m_type_data.mem.byte_index += size;

                return true;
            }

            case ek_stream_type_file:
                return static_cast<t_i32>(fread(arr.Raw(), sizeof(arr[0]), static_cast<size_t>(cnt), m_type_data.file.file)) == cnt;

            default:
                ZF_UNREACHABLE();
            }
        }

        template <co_array_nonstatic tp_type>
        [[nodiscard]] t_b8 WriteItemsOfArray(const tp_type arr) {
            ZF_ASSERT(m_mode == ek_stream_mode_write);

            if (arr.Len() == 0) {
                return true;
            }

            switch (m_type) {
            case ek_stream_type_mem: {
                const t_i32 size = arr.SizeInBytes();

                if (m_type_data.mem.byte_index + size > m_type_data.mem.byte_cnt) {
                    return false;
                }

                const auto dest = c_array_mut<t_u8>{m_type_data.mem.bytes_raw + m_type_data.mem.byte_index, size};
                const auto src = arr.AsByteArray();
                Copy(dest, src);

                m_type_data.mem.byte_index += size;

                return true;
            }

            case ek_stream_type_file:
                return static_cast<t_i32>(fwrite(arr.Raw(), sizeof(arr[0]), static_cast<size_t>(arr.Len()), m_type_data.file.file)) == arr.Len();

            default:
                ZF_UNREACHABLE();
            }
        }

    private:
        e_stream_type m_type = ek_stream_type_invalid;

        union {
            struct {
                t_u8 *bytes_raw;
                t_i32 byte_cnt;
                t_i32 byte_index;
            } mem;

            struct {
                FILE *file;
            } file;
        } m_type_data = {};

        e_stream_mode m_mode = {};
    };

    inline c_stream StdIn() { return {stdin, ek_stream_mode_read}; }
    inline c_stream StdOut() { return {stdout, ek_stream_mode_write}; }
    inline c_stream StdError() { return {stderr, ek_stream_mode_write}; }

    template <co_array_nonstatic tp_type>
    [[nodiscard]] t_b8 SerializeArray(c_stream *const stream, const tp_type arr) {
        if (!stream->WriteItem(arr.Len())) {
            return false;
        }

        if (!stream->WriteItemsOfArray(arr)) {
            return false;
        }

        return true;
    }

    template <typename tp_type>
    [[nodiscard]] t_b8 DeserializeArray(c_stream *const stream, c_arena *const arr_arena, c_array_mut<tp_type> *const o_arr) {
        t_i32 len;

        if (!stream->ReadItem(&len)) {
            return false;
        }

        *o_arr = AllocArray<tp_type>(len, arr_arena);

        if (!stream->ReadItemsIntoArray(*o_arr, len)) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 SerializeBitVec(c_stream *const stream, const c_bit_vec_rdonly bv) {
        if (!stream->WriteItem(bv.BitCount())) {
            return false;
        }

        if (!stream->WriteItemsOfArray(bv.Bytes())) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 DeserializeBitVec(c_stream *const stream, c_arena *const bv_arena, c_bit_vec_mut *const o_bv) {
        t_i32 bit_cnt;

        if (!stream->ReadItem(&bit_cnt)) {
            return false;
        }

        *o_bv = CreateBitVec(bit_cnt, bv_arena);

        if (!stream->ReadItemsIntoArray(o_bv->Bytes(), o_bv->Bytes().Len())) {
            return false;
        }

        return true;
    }

    // ============================================================


    // ============================================================
    // @section: Files and Directories

    enum e_file_access_mode : t_i32 {
        ek_file_access_mode_read,
        ek_file_access_mode_write,
        ek_file_access_mode_append
    };

    [[nodiscard]] t_b8 OpenFile(const s_str_rdonly file_path, const e_file_access_mode mode, c_arena *const temp_arena, c_stream *const o_stream);
    void CloseFile(c_stream *const stream);
    t_i32 CalcFileSize(c_stream *const stream);
    [[nodiscard]] t_b8 LoadFileContents(const s_str_rdonly file_path, c_arena *const contents_arena, c_arena *const temp_arena, c_array_mut<t_u8> *const o_contents, const t_b8 add_terminator = false);

    enum e_directory_creation_result : t_i32 {
        ek_directory_creation_result_success,
        ek_directory_creation_result_already_exists,
        ek_directory_creation_result_permission_denied,
        ek_directory_creation_result_path_not_found,
        ek_directory_creation_result_unknown_err
    };

    [[nodiscard]] t_b8 CreateDirec(const s_str_rdonly path, c_arena *const temp_arena, e_directory_creation_result *const o_creation_res = nullptr); // This DOES NOT create non-existent parent directories.
    [[nodiscard]] t_b8 CreateDirectoryAndParents(const s_str_rdonly path, c_arena *const temp_arena, e_directory_creation_result *const o_dir_creation_res = nullptr);
    [[nodiscard]] t_b8 CreateFileAndParentDirs(const s_str_rdonly path, c_arena *const temp_arena, e_directory_creation_result *const o_dir_creation_res = nullptr);

    enum e_path_type : t_i32 {
        ek_path_type_not_found,
        ek_path_type_file,
        ek_path_type_directory
    };

    e_path_type CheckPathType(const s_str_rdonly path, c_arena *const temp_arena);

    s_str LoadExecutableDirectory(c_arena *const arena);

    // ============================================================


    // ============================================================
    // @section: Printing

    inline t_b8 Print(c_stream *const stream, const s_str_rdonly str) {
        return stream->WriteItemsOfArray(str.bytes);
    }

    inline t_b8 PrintFormat(c_stream *const stream, const s_str_rdonly fmt);

    template <typename tp_arg_type, typename... tp_arg_types_leftover>
    t_b8 PrintFormat(c_stream *const stream, const s_str_rdonly fmt, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover);

    // Type format structs which are to be accepted as format printing arguments need to meet this (i.e. have the tag).
    template <typename tp_type>
    concept co_fmt = requires { typename tp_type::t_fmt_tag; };


    // ========================================
    // @subsection: Bool Printing

    struct s_bool_fmt {
        using t_fmt_tag = void;
        t_b8 val = false;
    };

    inline s_bool_fmt FormatBool(const t_b8 val) { return {val}; }
    inline s_bool_fmt FormatDefault(const t_b8 val) { return {val}; }

    inline t_b8 PrintType(c_stream *const stream, const s_bool_fmt fmt) {
        const s_str_rdonly true_str = c_cstr_literal("true");
        const s_str_rdonly false_str = c_cstr_literal("false");

        return Print(stream, fmt.val ? true_str : false_str);
    }

    // ========================================


    // ========================================
    // @subsection: String Printing

    struct s_str_fmt {
        using t_fmt_tag = void;
        s_str_rdonly val;
    };

    inline s_str_fmt FormatStr(const s_str_rdonly val) { return {val}; }
    inline s_str_fmt FormatDefault(const s_str_rdonly val) { return FormatStr(val); }

    inline t_b8 PrintType(c_stream *const stream, const s_str_fmt fmt) {
        return Print(stream, fmt.val);
    }
    // ========================================


    // ========================================
    // @subsection: Integer Printing

    template <co_integral tp_type>
    struct s_integral_fmt {
        using t_fmt_tag = void;
        tp_type val;
    };

    template <co_integral tp_type> s_integral_fmt<tp_type> FormatInt(const tp_type val) { return {val}; }
    template <co_integral tp_type> s_integral_fmt<tp_type> FormatDefault(const tp_type val) { return FormatInt(val); }

    template <co_integral tp_type>
    t_b8 PrintType(c_stream *const stream, const s_integral_fmt<tp_type> fmt) {
        s_static_array<t_u8, 20> str_bytes = {}; // Maximum possible number of ASCII characters needed to represent a 64-bit integer.
        c_stream str_bytes_stream = {str_bytes, ek_stream_mode_write};
        t_b8 str_bytes_stream_write_success = true;

        if (fmt.val < 0) {
            str_bytes_stream_write_success = str_bytes_stream.WriteItem('-');
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        const t_i32 dig_cnt = CalcDigitCnt(fmt.val);

        for (t_i32 i = 0; i < dig_cnt; i++) {
            const auto chr = static_cast<char>('0' + FindDigitAt(fmt.val, dig_cnt - 1 - i));
            str_bytes_stream_write_success = str_bytes_stream.WriteItem(chr);
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        return Print(stream, {str_bytes_stream.Written()});
    }

    // ========================================


    // ========================================
    // @subsection: Float Printing

    template <co_floating_point tp_type>
    struct s_float_fmt {
        using t_fmt_tag = void;

        tp_type val;
        t_b8 trim_trailing_zeros;
    };

    template <co_floating_point tp_type>
    s_float_fmt<tp_type> FormatFloat(const tp_type val, const t_b8 trim_trailing_zeros = false) {
        return {val, trim_trailing_zeros};
    }

    template <co_floating_point tp_type> s_float_fmt<tp_type> FormatDefault(const tp_type val) {
        return FormatFloat(val);
    }

    template <co_floating_point tp_type>
    t_b8 PrintType(c_stream *const stream, const s_float_fmt<tp_type> fmt) {
        s_static_array<t_u8, 400> str_bytes = {}; // Roughly more than how many bytes should ever be needed.

        t_i32 str_bytes_used = snprintf(reinterpret_cast<char *>(str_bytes.raw), str_bytes.g_len, "%f", static_cast<t_f64>(fmt.val));

        if (str_bytes_used < 0 || str_bytes_used >= str_bytes.g_len) {
            return false;
        }

        if (fmt.trim_trailing_zeros) {
            const auto str_bytes_relevant = str_bytes.AsNonstatic().Slice(0, str_bytes_used);

            if (DoAnyEqual(str_bytes_relevant, '.')) {
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

        return Print(stream, {str_bytes.AsNonstatic().Slice(0, str_bytes_used)});
    }

    // ========================================


    // ========================================
    // @subsection: Hex Printing

    enum e_hex_fmt_flags : t_i32 {
        ek_hex_fmt_flags_none = 0,
        ek_hex_fmt_flags_omit_prefix = 1 << 0,
        ek_hex_fmt_flags_lower_case = 1 << 1,
        ek_hex_fmt_flags_allow_odd_digit_cnt = 1 << 2
    };

    constexpr t_i32 g_hex_fmt_digit_cnt_min = 1;
    constexpr t_i32 g_hex_fmt_digit_cnt_max = 16;

    template <co_unsigned_integral tp_type>
    struct s_hex_fmt {
        using t_fmt_tag = void;

        tp_type val;
        e_hex_fmt_flags flags;
        t_i32 min_digits; // Will be rounded UP to the next even if this is odd and the flag for allowing an odd digit count is unset.
    };

    template <co_unsigned_integral tp_type>
    s_hex_fmt<tp_type> FormatHex(const tp_type val, const e_hex_fmt_flags flags = {}, const t_i32 min_digits = g_hex_fmt_digit_cnt_min) {
        return {val, flags, min_digits};
    }

    // @todo: Check both of these functions below still work.

    inline s_hex_fmt<t_uintptr> FormatHex(const void *const ptr, const e_hex_fmt_flags flags = {}, const t_i32 min_digits = g_hex_fmt_digit_cnt_min) {
        return {reinterpret_cast<t_uintptr>(ptr), flags, min_digits};
    }

    inline s_hex_fmt<t_uintptr> FormatDefault(const void *const ptr) {
        return FormatHex(ptr, {}, 2 * ZF_SIZE_OF(t_uintptr));
    }

    template <co_unsigned_integral tp_type>
    t_b8 PrintType(c_stream *const stream, const s_hex_fmt<tp_type> fmt) {
        ZF_ASSERT(fmt.min_digits >= g_hex_fmt_digit_cnt_min && fmt.min_digits <= g_hex_fmt_digit_cnt_max);

        s_static_array<t_u8, 2 + g_hex_fmt_digit_cnt_max> str_bytes = {}; // Can facilitate max number of digits plus the "0x" prefix.
        c_stream str_bytes_stream = {str_bytes, ek_stream_mode_write};

        t_b8 str_bytes_stream_write_success = true;

        if (!(fmt.flags & ek_hex_fmt_flags_omit_prefix)) {
            str_bytes_stream_write_success = str_bytes_stream.WriteItem('0');
            ZF_ASSERT(str_bytes_stream_write_success);

            str_bytes_stream_write_success = str_bytes_stream.WriteItem('x');
            ZF_ASSERT(str_bytes_stream_write_success);
        }

        const t_i32 str_bytes_digits_begin_pos = str_bytes_stream.Pos();

        const auto dig_to_byte = [flags = fmt.flags](const t_i32 dig) -> t_u8 {
            if (dig < 10) {
                return static_cast<t_u8>('0' + dig);
            } else {
                if (flags & ek_hex_fmt_flags_lower_case) {
                    return static_cast<t_u8>('a' + dig - 10);
                } else {
                    return static_cast<t_u8>('A' + dig - 10);
                }
            }
        };

        auto val_mut = fmt.val;

        t_i32 cnter = 0;
        const t_i32 inner_loop_cnt = (fmt.flags & ek_hex_fmt_flags_allow_odd_digit_cnt) ? 1 : 2;

        do {
            for (t_i32 i = 0; i < inner_loop_cnt; i++) {
                const t_u8 byte = dig_to_byte(val_mut % 16);
                str_bytes_stream_write_success = str_bytes_stream.WriteItem(byte);
                ZF_ASSERT(str_bytes_stream_write_success);

                val_mut /= 16;

                cnter++;
            }
        } while (val_mut != 0 || cnter < fmt.min_digits);

        const auto str_bytes_digits = str_bytes_stream.Written().SliceFrom(str_bytes_digits_begin_pos);
        Reverse(str_bytes_digits);

        return Print(stream, {str_bytes_stream.Written()});
    }

    // ========================================


    // ========================================
    // @subsection: V2 Printing

    struct s_v2_fmt {
        using t_fmt_tag = void;

        s_v2 val;
        t_b8 trim_trailing_zeros;
    };

    constexpr s_v2_fmt FormatV2(const s_v2 val, const t_b8 trim_trailing_zeros = false) { return {val, trim_trailing_zeros}; }
    constexpr s_v2_fmt FormatDefault(const s_v2 val) { return FormatV2(val); }

    inline t_b8 PrintType(c_stream *const stream, const s_v2_fmt fmt) {
        return Print(stream, c_cstr_literal("("))
            && PrintType(stream, FormatFloat(fmt.val.x, fmt.trim_trailing_zeros))
            && Print(stream, c_cstr_literal(", "))
            && PrintType(stream, FormatFloat(fmt.val.y, fmt.trim_trailing_zeros))
            && Print(stream, c_cstr_literal(")"));
    }

    struct s_v2_i_fmt {
        using t_fmt_tag = void;

        s_v2_i val;
    };

    constexpr s_v2_i_fmt FormatV2(const s_v2_i val) { return {val}; }
    constexpr s_v2_i_fmt FormatDefault(const s_v2_i val) { return FormatV2(val); }

    inline t_b8 PrintType(c_stream *const stream, const s_v2_i_fmt fmt) {
        return Print(stream, c_cstr_literal("("))
            && PrintType(stream, FormatInt(fmt.val.x))
            && Print(stream, c_cstr_literal(", "))
            && PrintType(stream, FormatInt(fmt.val.y))
            && Print(stream, c_cstr_literal(")"));
    }

    // ========================================


    // ========================================
    // @subsection: Array Printing

    template <typename tp_arr_type>
    concept co_formattable_array = co_array_nonstatic<tp_arr_type>
        && requires(const typename tp_arr_type::t_elem &v) { { FormatDefault(v) } -> co_fmt; };

    template <co_formattable_array tp_arr_type>
    struct s_array_fmt {
        using t_fmt_tag = void;

        tp_arr_type val;
        t_b8 one_per_line;
    };

    template <co_formattable_array tp_arr_type>
    s_array_fmt<tp_arr_type> FormatArray(const tp_arr_type val, const t_b8 one_per_line = false) {
        return {val, one_per_line};
    }

    template <co_formattable_array tp_arr_type>
    s_array_fmt<tp_arr_type> FormatDefault(const tp_arr_type val) {
        return {val};
    }

    template <co_formattable_array tp_arr_type>
    t_b8 PrintType(c_stream *const stream, const s_array_fmt<tp_arr_type> fmt) {
        if (fmt.one_per_line) {
            for (t_i32 i = 0; i < fmt.val.Len(); i++) {
                if (!PrintFormat(stream, c_cstr_literal("[%] %%"), i, fmt.val[i], i < fmt.val.Len() - 1 ? c_cstr_literal("\n") : c_cstr_literal(""))) {
                    return false;
                }
            }
        } else {
            if (!Print(stream, c_cstr_literal("["))) {
                return false;
            }

            for (t_i32 i = 0; i < fmt.val.Len(); i++) {
                if (!PrintFormat(stream, c_cstr_literal("%"), fmt.val[i])) {
                    return false;
                }

                if (i < fmt.val.Len() - 1) {
                    if (!Print(stream, c_cstr_literal(", "))) {
                        return false;
                    }
                }
            }

            if (!Print(stream, c_cstr_literal("]"))) {
                return false;
            }
        }

        return true;
    }

    // ========================================


    // ========================================
    // @subsection: Bit Vector Printing

    enum e_bit_vec_fmt_style : t_i32 {
        ek_bit_vec_fmt_style_seq = 0,                // List all bits from LSB to MSB, not divided into bytes.
        ek_bit_vec_fmt_style_little_endian = 1 << 0, // Split into bytes, ordered in little endian.
        ek_bit_vec_fmt_style_big_endian = 1 << 1     // Split into bytes, ordered in big endian.
    };

    struct s_bit_vec_fmt {
        using t_fmt_tag = void;

        c_bit_vec_rdonly val;
        e_bit_vec_fmt_style style;
    };

    inline s_bit_vec_fmt FormatBitVec(const c_bit_vec_rdonly &val, const e_bit_vec_fmt_style style) { return {val, style}; }
    inline s_bit_vec_fmt FormatDefault(const c_bit_vec_rdonly &val) { return FormatBitVec(val, ek_bit_vec_fmt_style_seq); }

    inline t_b8 PrintType(c_stream *const stream, const s_bit_vec_fmt fmt) {
        const auto print_bit = [&](const t_i32 bit_index) {
            const s_str_rdonly str = IsBitSet(fmt.val, bit_index) ? c_cstr_literal("1") : c_cstr_literal("0");
            return Print(stream, str);
        };

        const auto print_byte = [&](const t_i32 index) {
            const t_i32 bit_cnt = index == fmt.val.Bytes().Len() - 1 ? fmt.val.LastByteBitCount() : 8;

            for (t_i32 i = 7; i >= bit_cnt; i--) {
                Print(stream, c_cstr_literal("0"));
            }

            for (t_i32 i = bit_cnt - 1; i >= 0; i--) {
                print_bit((index * 8) + i);
            }
        };

        switch (fmt.style) {
        case ek_bit_vec_fmt_style_seq:
            for (t_i32 i = 0; i < fmt.val.BitCount(); i++) {
                if (!print_bit(i)) {
                    return false;
                }
            }

            break;

        case ek_bit_vec_fmt_style_little_endian:
            for (t_i32 i = 0; i < fmt.val.Bytes().Len(); i++) {
                if (i > 0) {
                    Print(stream, c_cstr_literal(" "));
                }

                print_byte(i);
            }

            break;

        case ek_bit_vec_fmt_style_big_endian:
            for (t_i32 i = fmt.val.Bytes().Len() - 1; i >= 0; i--) {
                print_byte(i);

                if (i > 0) {
                    Print(stream, c_cstr_literal(" "));
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

    constexpr t_i32 CountFormatSpecifiers(const s_str_rdonly str) {
        static_assert(IsASCII(g_print_fmt_spec) && IsASCII(g_print_fmt_esc)); // Assuming this for this algorithm.

        t_b8 escaped = false;
        t_i32 cnt = 0;

        for (t_i32 i = 0; i < str.bytes.Len(); i++) {
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

    inline t_b8 PrintFormat(c_stream *const stream, const s_str_rdonly fmt) {
        ZF_ASSERT(CountFormatSpecifiers(fmt) == 0);

        // Just print the rest of the string.
        return Print(stream, fmt);
    }

    // Use a single '%' as the format specifier. To actually include a '%' in the output, write "^%". To actually include a '^', write "^^".
    // Returns true iff the operation was successful.
    template <typename tp_arg_type, typename... tp_arg_types_leftover>
    t_b8 PrintFormat(c_stream *const stream, const s_str_rdonly fmt, const tp_arg_type &arg, const tp_arg_types_leftover &...args_leftover) {
        ZF_ASSERT(CountFormatSpecifiers(fmt) == 1 + sizeof...(args_leftover));

        static_assert(IsASCII(g_print_fmt_spec) && IsASCII(g_print_fmt_esc)); // Assuming this for this algorithm.

        t_b8 escaped = false;

        for (t_i32 i = 0; i < fmt.bytes.Len(); i++) {
            if (!escaped) {
                if (fmt.bytes[i] == g_print_fmt_esc) {
                    escaped = true;
                    continue;
                } else if (fmt.bytes[i] == g_print_fmt_spec) {
                    if constexpr (co_fmt<tp_arg_type>) {
                        if (!PrintType(stream, arg)) {
                            return false;
                        }
                    } else {
                        if (!PrintType(stream, FormatDefault(arg))) {
                            return false;
                        }
                    }

                    const s_str_rdonly fmt_leftover = {fmt.bytes.Slice(i + 1, fmt.bytes.Len())}; // The substring of everything after the format specifier.
                    return PrintFormat(stream, fmt_leftover, args_leftover...);
                }
            }

            if (!stream->WriteItem(fmt.bytes[i])) {
                return false;
            }

            escaped = false;
        }

        return true;
    }

    // ========================================


    // ========================================
    // @subsection: Logging Helpers

    template <typename... tp_arg_types>
    t_b8 Log(const s_str_rdonly fmt, const tp_arg_types &...args) {
        c_stream std_err = StdOut();

        if (!PrintFormat(&std_err, fmt, args...)) {
            return false;
        }

        if (!Print(&std_err, c_cstr_literal("\n"))) {
            return false;
        }

        return true;
    }

    template <typename... tp_arg_types>
    t_b8 LogError(const s_str_rdonly fmt, const tp_arg_types &...args) {
        c_stream std_err = StdError();

        if (!Print(&std_err, c_cstr_literal("Error: "))) {
            return false;
        }

        if (!PrintFormat(&std_err, fmt, args...)) {
            return false;
        }

        if (!Print(&std_err, c_cstr_literal("\n"))) {
            return false;
        }

        return true;
    }

    template <typename... tp_arg_types>
    t_b8 LogErrorType(const s_str_rdonly type_name, const s_str_rdonly fmt, const tp_arg_types &...args) {
        ZF_ASSERT(!IsStrEmpty(type_name));

        c_stream std_err = StdError();

        if (!PrintFormat(&std_err, c_cstr_literal("% Error: "), type_name)) {
            return false;
        }

        if (!PrintFormat(&std_err, fmt, args...)) {
            return false;
        }

        if (!Print(&std_err, c_cstr_literal("\n"))) {
            return false;
        }

        return true;
    }

    template <typename... tp_arg_types>
    t_b8 LogWarning(const s_str_rdonly fmt, const tp_arg_types &...args) {
        c_stream std_err = StdError();

        if (!Print(&std_err, c_cstr_literal("Warning: "))) {
            return false;
        }

        if (!PrintFormat(&std_err, fmt, args...)) {
            return false;
        }

        if (!Print(&std_err, c_cstr_literal("\n"))) {
            return false;
        }

        return true;
    }

    // ========================================


    // ============================================================
}
