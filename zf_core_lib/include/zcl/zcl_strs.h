#pragma once

#include <zcl/zcl_basic.h>
#include <zcl/zcl_bits.h>
#include <zcl/zcl_algos.h>
#include <zcl/zcl_hash_maps.h>

namespace zcl {
    // ============================================================
    // @section: Code Points

    constexpr t_i32 k_code_point_count = 1114112;

    using t_code_point = char32_t;
    using t_code_point_bitset = t_static_bitset<k_code_point_count>;

    constexpr t_i32 k_ascii_range_begin = 0;
    constexpr t_i32 k_ascii_range_end = 0x80;

    constexpr t_i32 k_printable_ascii_range_begin = 0x20;
    constexpr t_i32 k_printable_ascii_range_end = 0x7F;

    constexpr t_b8 CodePointCheckASCII(const t_code_point cp) {
        return cp >= k_ascii_range_begin && cp < k_ascii_range_end;
    }

    constexpr t_b8 CodePointCheckPrintableASCII(const t_code_point cp) {
        return cp >= k_printable_ascii_range_begin && cp < k_printable_ascii_range_end;
    }

    // If the code point is valid, result is guaranteed to be 1, 2, 3, or 4.
    constexpr t_i32 CodePointGetUTF8ByteCount(const t_code_point code_pt) {
        if (code_pt <= 0x7F) {
            return 1;
        }

        if (code_pt <= 0x7FF) {
            return 2;
        }

        if (code_pt <= 0xFFFF) {
            return 3;
        }

        if (code_pt <= 0x10FFFF) {
            return 4;
        }

        ZCL_UNREACHABLE();
    }

    // Output byte count will be 1, 2, 3, or 4.
    void CodePointToUTF8Bytes(const t_code_point cp, t_static_array<t_u8, 4> *const o_bytes, t_i32 *const o_byte_cnt);

    // Given array must be of length 1, 2, 3, or 4.
    t_code_point UTF8BytesToCodePoint(const t_array_rdonly<t_u8> bytes);

    // ============================================================


    // ============================================================
    // @section: ZF Strings

    struct t_str_rdonly {
        t_array_rdonly<t_u8> bytes;
    };

    struct t_str_mut {
        t_array_mut<t_u8> bytes;

        operator t_str_rdonly() const {
            return {bytes};
        }
    };

    constexpr t_comparator_bin<t_str_rdonly> k_str_comparator_bin =
        [](const t_str_rdonly &a, const t_str_rdonly &b) {
            return k_array_comparator_bin<t_array_rdonly<t_u8>>(a.bytes, b.bytes);
        };

    // This is an FNV-1a implementation.
    constexpr t_hash_func<t_str_rdonly> k_str_hash_func =
        [](const t_str_rdonly &key) {
            const t_u32 offs_basis = 2166136261u;
            const t_u32 prime = 16777619u;

            t_u32 hash = offs_basis;

            for (t_i32 i = 0; i < key.bytes.len; i++) {
                hash ^= static_cast<t_u8>(key.bytes[i]);
                hash *= prime;
            }

            return static_cast<t_i32>(hash & 0x7FFFFFFFull);
        };

    inline t_b8 StrBytesCheckTerminatedAnywhere(const t_array_rdonly<t_u8> bytes) {
        for (t_i32 i = bytes.len - 1; i >= 0; i--) {
            if (!bytes[i]) {
                return true;
            }
        }

        return false;
    }

    inline t_b8 StrBytesCheckTerminatedOnlyAtEnd(const t_array_rdonly<t_u8> bytes) {
        if (bytes.len == 0 || bytes[bytes.len - 1]) {
            return false;
        }

        for (t_i32 i = bytes.len - 2; i >= 0; i--) {
            if (!bytes[i]) {
                return false;
            }
        }

        return true;
    }

    inline char *StrToCStr(const t_str_mut str) {
        ZCL_ASSERT(StrBytesCheckTerminatedAnywhere(str.bytes));
        return reinterpret_cast<char *>(str.bytes.raw);
    }

    inline const char *StrToCStr(const t_str_rdonly str) {
        ZCL_ASSERT(StrBytesCheckTerminatedAnywhere(str.bytes));
        return reinterpret_cast<const char *>(str.bytes.raw);
    }

    // Allocates a clone of the given string using the memory arena, with a null byte added at the end (even if the string was already terminated).
    inline t_str_mut StrCloneButAddTerminator(const t_str_rdonly str, t_arena *const arena) {
        const t_str_mut clone = {arena_push_array<t_u8>(arena, str.bytes.len + 1)};
        array_copy(str.bytes, clone.bytes);
        clone.bytes[clone.bytes.len - 1] = 0;
        return clone;
    }

    inline t_b8 StrCheckEmpty(const t_str_rdonly str) {
        return str.bytes.len == 0;
    }

    t_b8 StrCheckValidUTF8(const t_str_rdonly str);

    // Calculates the string length in terms of code point count. Reminder that '\0' is treated just like any other ASCII character and does not terminate.
    t_i32 StrCalcLen(const t_str_rdonly str);

    t_code_point StrFindCodePointAtByte(const t_str_rdonly str, const t_i32 byte_index);

    // Sets the bits associated with each unicode code point that appear in the string. No bits get unset.
    void StrMarkCodePoints(const t_str_rdonly str, t_code_point_bitset *const code_pts);

    struct t_str_walk_step {
        t_code_point code_pt;
        t_i32 byte_index;
    };

    // byte_index should be initialized to the index of ANY byte in the code point to start walking from.
    // Returns false iff the walk has ended.
    t_b8 StrWalk(const t_str_rdonly str, t_i32 *const byte_index, t_str_walk_step *const o_step);

    // byte_index should be initialized to the index of ANY byte in the code point to start walking backwards from.
    // Returns false iff the walk has ended.
    t_b8 StrWalkReverse(const t_str_rdonly str, t_i32 *const byte_index, t_str_walk_step *const o_step);

#define ZCL_STR_WALK(str, step)                                                                                        \
    for (zcl::t_i32 ZCL_CONCAT(bi_l, __LINE__) = 0; ZCL_CONCAT(bi_l, __LINE__) != -1; ZCL_CONCAT(bi_l, __LINE__) = -1) \
        for (zcl::t_str_walk_step step; zcl::StrWalk(str, &ZCL_CONCAT(bi_l, __LINE__), &step);)

#define ZCL_STR_WALK_REVERSE(str, step)                                                                                                                            \
    for (zcl::t_i32 ZCL_CONCAT(bi_l, __LINE__) = (str).bytes.len - 1; ZCL_CONCAT(bi_l, __LINE__) != (str).bytes.len; ZCL_CONCAT(bi_l, __LINE__) = (str).bytes.len) \
        for (zcl::t_str_walk_step step; zcl::StrWalkReverse(str, &ZCL_CONCAT(bi_l, __LINE__), &step);)

    inline t_b8 StrsCheckEqual(const t_str_rdonly a, const t_str_rdonly b) {
        return CompareAllBin(a.bytes, b.bytes);
    }

    // ============================================================


    // ============================================================
    // @section: C-Strings

    constexpr t_i32 CStrCalcLen(const char *const c_str) {
        t_i32 len = 0;
        for (; c_str[len]; len++) {}
        return len;
    }

    // Creates a NON-TERMINATED string object from the given TERMINATED C-string.
    // Does a conventional string walk to calculate length.
    inline t_str_mut CStrToStr(char *const c_str) {
        return {{reinterpret_cast<t_u8 *>(c_str), CStrCalcLen(c_str)}};
    }

    // Creates a read-only NON-TERMINATED string object from the given TERMINATED C-string.
    // Does a conventional string walk to calculate length.
    inline t_str_rdonly CStrToStr(const char *const c_str) {
        return {{reinterpret_cast<const t_u8 *>(c_str), CStrCalcLen(c_str)}};
    }

    namespace detail {
        // Hidden object that can only be constructed with a valid string literal at compile time.
        // Implicit cast to ZF-style string has to be done at runtime due to reinterpret cast.
        struct t_c_str_literal {
            t_c_str_literal() = delete;

            template <t_i32 tp_buf_size>
            consteval t_c_str_literal(const char (&buf)[tp_buf_size]) : buf(buf), buf_size(tp_buf_size) {
                if (buf[tp_buf_size - 1]) {
                    throw "Static char array not terminated at end!";
                }

                // Disabled because sometimes it might be useful to manually insert null characters before end.
#if 0
                for (t_i32 i = 0; i < tp_buf_size; i++) {
                    if (i < tp_buf_size - 1 && !buf[i]) {
                        throw "Terminator found in static char array before end!";
                    }
                }
#endif
            }

            operator t_str_rdonly() {
                return {{reinterpret_cast<const t_u8 *>(buf), buf_size - 1}};
            }

        private:
            const char *const buf;
            const t_i32 buf_size;
        };

#define ZCL_STR_LITERAL(c_str_lit) zcl::t_str_rdonly(zcl::detail::t_c_str_literal(c_str_lit))
    }

    // ============================================================
}
