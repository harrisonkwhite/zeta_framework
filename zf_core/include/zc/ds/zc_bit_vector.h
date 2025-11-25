#pragma once

#include <zc/zc_mem.h>

namespace zf {
    struct s_bit_vector_rdonly {
        s_array_rdonly<t_u8> bytes;
        t_size bit_cnt;

        constexpr operator s_bit_range_rdonly() const {
            return {bytes, 0, bit_cnt};
        }
    };

    struct s_bit_vector {
        s_array<t_u8> bytes;
        t_size bit_cnt;

        constexpr operator s_bit_vector_rdonly() const {
            return {bytes, bit_cnt};
        }

        constexpr operator s_bit_range() const {
            return {bytes, 0, bit_cnt};
        }

        constexpr operator s_bit_range_rdonly() const {
            return {bytes, 0, bit_cnt};
        }
    };

    template<t_size tp_bit_cnt>
    struct s_static_bit_vector {
        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> bytes;

        constexpr operator s_bit_vector() {
            return {bytes, tp_bit_cnt};
        }

        constexpr operator s_bit_vector_rdonly() const {
            return {bytes, tp_bit_cnt};
        }

        constexpr operator s_bit_range() {
            return {bytes, 0, tp_bit_cnt};
        }

        constexpr operator s_bit_range_rdonly() const {
            return {bytes, 0, tp_bit_cnt};
        }
    };

    t_b8 MakeBitVector(s_mem_arena& mem_arena, const t_size bit_cnt, s_bit_vector& o_bv);

    struct s_stream;
    [[nodiscard]] t_b8 SerializeBitVector(s_stream& stream, const s_bit_vector_rdonly bv); // Serializes the bit vector EXCLUDING BYTES BEYOND THE BIT COUNT! For example, a bit vector with a bit count of 10 will only have its first 2 bytes serialized. Excess bits in the final byte are not zeroed out.
    [[nodiscard]] t_b8 DeserializeBitVector(s_stream& stream, s_mem_arena& mem_arena, s_bit_vector& o_bv);
}
