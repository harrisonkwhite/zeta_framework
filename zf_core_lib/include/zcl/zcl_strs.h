#pragma once

#include <zcl/zcl_mem.h>

namespace zf {
    using t_unicode_code_pt = char32_t;

    constexpr t_len g_unicode_code_pt_cnt = 1114112;

    using t_unicode_code_pt_bit_vec = s_static_bit_vec<g_unicode_code_pt_cnt>;

    constexpr t_len UnicodeCodePointToByteCnt(const t_unicode_code_pt cp) {
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

    constexpr t_b8 IsASCII(const t_unicode_code_pt cp) {
        return cp >= g_ascii_range_begin && cp < g_ascii_range_end;
    }

    constexpr t_len g_printable_ascii_range_begin = 0x20;
    constexpr t_len g_printable_ascii_range_end = 0x7F;

    constexpr t_b8 IsPrintableASCII(const t_unicode_code_pt cp) {
        return cp >= g_printable_ascii_range_begin && cp < g_printable_ascii_range_end;
    }

    struct s_str_chr_info {
        t_unicode_code_pt code_pt = 0;
        t_len byte_index = 0;
    };

    struct s_str_rdonly {
        s_array_rdonly<char> bytes = {};

        constexpr s_str_rdonly() = default;
        constexpr s_str_rdonly(const s_array_rdonly<char> bytes) : bytes(bytes) {}

        template <t_len tp_len>
        consteval s_str_rdonly(const char (&raw)[tp_len]) : bytes({raw, tp_len}) {}

        constexpr const char *Raw() const {
            return bytes.Ptr().Raw();
        }

        t_b8 IsValid() const;
        t_len CalcLen() const;
        t_unicode_code_pt CodePointAtByte(const t_len byte_index) const;

        // Sets the bits associated with each unicode code point that appear in the string. No bits get unset.
        void MarkCodePoints(t_unicode_code_pt_bit_vec *const code_pts) const;

        // byte_index should be initialised to the index of ANY byte in the code point to start walking from.
        // Returns false iff the walk has ended.
        t_b8 Walk(t_len *const byte_index, s_str_chr_info *const o_chr_info) const;

        // byte_index should be initialised to the index of ANY byte in the code point to start walking backwards from.
        // Returns false iff the walk has ended.
        t_b8 WalkReverse(t_len *const byte_index, s_str_chr_info *const o_chr_info) const;
    };

    struct s_str {
        s_array<char> bytes = {};

        constexpr operator s_str_rdonly() const {
            return {bytes};
        }

        constexpr char *Raw() const {
            return bytes.Ptr().Raw();
        }

        t_b8 IsValid() const {
            return static_cast<s_str_rdonly>(*this).IsValid();
        }

        t_len CalcLen() const {
            return static_cast<s_str_rdonly>(*this).CalcLen();
        }

        t_unicode_code_pt CodePointAtByte(const t_len byte_index) const {
            return static_cast<s_str_rdonly>(*this).CodePointAtByte(byte_index);
        }

        // Sets the bits associated with each unicode code point that appear in the string.
        void MarkCodePoints(t_unicode_code_pt_bit_vec *const code_pts) const {
            static_cast<s_str_rdonly>(*this).MarkCodePoints(code_pts);
        }

        // byte_index should be initialised to the index of ANY byte in the code point to start walking from.
        // Returns false iff the walk has ended.
        t_b8 Walk(t_len *const byte_index, s_str_chr_info *const o_chr_info) const {
            return static_cast<s_str_rdonly>(*this).Walk(byte_index, o_chr_info);
        }

        // byte_index should be initialised to the index of ANY byte in the code point to start walking backwards from.
        // Returns false iff the walk has ended.
        t_b8 WalkReverse(t_len *const byte_index, s_str_chr_info *const o_chr_info) const {
            return static_cast<s_str_rdonly>(*this).WalkReverse(byte_index, o_chr_info);
        }
    };

#define ZF_WALK_STR(str, chr_info)                                                                             \
    for (t_len ZF_CONCAT(bi_l, __LINE__) = 0; ZF_CONCAT(bi_l, __LINE__) != -1; ZF_CONCAT(bi_l, __LINE__) = -1) \
        for (s_str_chr_info chr_info; (str).Walk(&ZF_CONCAT(bi_l, __LINE__), &chr_info);)

#define ZF_WALK_STR_REVERSE(str, chr_info)                                                                                                                       \
    for (t_len ZF_CONCAT(bi_l, __LINE__) = (str).bytes.Len() - 1; ZF_CONCAT(bi_l, __LINE__) != (str).bytes.Len(); ZF_CONCAT(bi_l, __LINE__) = (str).bytes.Len()) \
        for (s_str_chr_info chr_info; (str).WalkReverse(&ZF_CONCAT(bi_l, __LINE__), &chr_info);)
}
