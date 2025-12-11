#pragma once

#include <zcl/zcl_mem.h>

namespace zf {
    using t_code_pt = char32_t;

    constexpr t_len g_unicode_code_pt_cnt = 1114112;

    using t_unicode_code_pt_bit_vec = s_static_bit_vec<g_unicode_code_pt_cnt>;

    constexpr t_len UnicodeCodePointToByteCnt(const t_code_pt cp) {
        if (cp <= 0x7F) {
            return 1;
        }

        if (cp <= 0x7FF) {
            return 2;
        }

        if (cp <= 0xFFFF) {
            return 3;
        }

        if (cp <= 0x10FFFF) {
            return 4;
        }

        ZF_ASSERT(false);

        return 0;
    }

    constexpr t_len g_ascii_range_begin = 0;
    constexpr t_len g_ascii_range_end = 0x80;

    constexpr t_b8 IsASCII(const t_code_pt cp) {
        return cp >= g_ascii_range_begin && cp < g_ascii_range_end;
    }

    constexpr t_len g_printable_ascii_range_begin = 0x20;
    constexpr t_len g_printable_ascii_range_end = 0x7F;

    constexpr t_b8 IsPrintableASCII(const t_code_pt cp) {
        return cp >= g_printable_ascii_range_begin && cp < g_printable_ascii_range_end;
    }

    constexpr t_b8 IsStrTerminated(const s_array_rdonly<t_u8> bytes) {
        for (t_len i = bytes.Len() - 1; i >= 0; i--) {
            if (!bytes[i]) {
                return true;
            }
        }

        return false;
    }

    struct s_str_rdonly {
        s_array_rdonly<t_u8> bytes = {};

        constexpr s_str_rdonly() = default;
        constexpr s_str_rdonly(const s_array_rdonly<t_u8> bytes) : bytes(bytes) {}

        template <t_len tp_len>
        s_str_rdonly(const char (&raw)[tp_len]) : bytes({reinterpret_cast<const t_u8 *>(raw), tp_len - 1}) {}

        constexpr t_b8 IsEmpty() const {
            return bytes.IsEmpty();
        }

        const char *Cstr() const {
            ZF_ASSERT(IsStrTerminated(bytes));
            return reinterpret_cast<const char *>(bytes.Ptr().Raw());
        }
    };

    struct s_str {
        s_array<t_u8> bytes = {};

        constexpr s_str() = default;
        constexpr s_str(const s_array<t_u8> bytes) : bytes(bytes) {}

        constexpr t_b8 IsEmpty() const {
            return bytes.IsEmpty();
        }

        char *Cstr() const {
            ZF_ASSERT(IsStrTerminated(bytes));
            return reinterpret_cast<char *>(bytes.Ptr().Raw());
        }

        constexpr operator s_str_rdonly() const {
            return {bytes};
        }
    };

    // Allocates a clone of the given string using the memory arena, with a null byte added at the end (even if the string was already terminated).
    [[nodiscard]] inline t_b8 AllocStrCloneWithTerminator(const s_str_rdonly str, s_mem_arena &mem_arena, s_str &o_clone) {
        if (!AllocArray(str.bytes.Len() + 1, mem_arena, o_clone.bytes)) {
            return false;
        }

        str.bytes.CopyTo(o_clone.bytes);

        return true;
    }

    t_b8 IsStrValidUTF8(const s_str_rdonly str);

    // Calculates the length in terms of code point count. Note that '\0' is treated just like any other ASCII character.
    t_len CalcStrLen(const s_str_rdonly str);

    t_code_pt StrCodePointAtByte(const s_str_rdonly str, const t_len byte_index);

    // Sets the bits associated with each unicode code point that appear in the string. No bits get unset.
    void MarkStrCodePoints(const s_str_rdonly str, t_unicode_code_pt_bit_vec &code_pts);

    struct s_str_walk_info {
        t_code_pt code_pt = 0;
        t_len byte_index = 0;
    };

    // byte_index should be initialised to the index of ANY byte in the code point to start walking from.
    // Returns false iff the walk has ended.
    t_b8 WalkStr(const s_str_rdonly str, t_len &byte_index, s_str_walk_info &o_info);

    // byte_index should be initialised to the index of ANY byte in the code point to start walking backwards from.
    // Returns false iff the walk has ended.
    t_b8 WalkStrReverse(const s_str_rdonly str, t_len &byte_index, s_str_walk_info &o_info);

#define ZF_WALK_STR(str, info)                                                                                 \
    for (t_len ZF_CONCAT(bi_l, __LINE__) = 0; ZF_CONCAT(bi_l, __LINE__) != -1; ZF_CONCAT(bi_l, __LINE__) = -1) \
        for (s_str_walk_info info; WalkStr(str, ZF_CONCAT(bi_l, __LINE__), info);)

#define ZF_WALK_STR_REVERSE(str, info)                                                                                                                           \
    for (t_len ZF_CONCAT(bi_l, __LINE__) = (str).bytes.Len() - 1; ZF_CONCAT(bi_l, __LINE__) != (str).bytes.Len(); ZF_CONCAT(bi_l, __LINE__) = (str).bytes.Len()) \
        for (s_str_walk_info info; WalkStrReverse(str, ZF_CONCAT(bi_l, __LINE__), info);)
}
