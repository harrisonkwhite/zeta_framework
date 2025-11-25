#include <zc/ds/zc_bit_vector.h>

namespace zf {
    t_b8 MakeBitVector(s_mem_arena& mem_arena, const t_size bit_cnt, s_bit_vector& o_bv) {
        ZF_ASSERT(bit_cnt > 0);

        o_bv = {
            .bit_cnt = bit_cnt
        };

        return MakeArray(mem_arena, BitsToBytes(bit_cnt), o_bv.bytes);
    }

    t_b8 SerializeBitVector(s_stream& stream, const s_bit_vector_rdonly bv) {
        if (!StreamWriteItem(stream, bv.bit_cnt)) {
            return false;
        }

        const auto bytes_src = Slice(bv.bytes, 0, BitsToBytes(bv.bit_cnt));

        if (!StreamWriteItemsOfArray(stream, bytes_src)) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeBitVector(s_stream& stream, s_mem_arena& mem_arena, s_bit_vector& o_bv) {
        if (!StreamReadItem(stream, o_bv.bit_cnt)) {
            return false;
        }

        if (o_bv.bit_cnt > 0) {
            if (!MakeArray(mem_arena, BitsToBytes(o_bv.bit_cnt), o_bv.bytes)) {
                return false;
            }

            if (!StreamReadItemsIntoArray(stream, o_bv.bytes, o_bv.bytes.len)) {
                return false;
            }
        }

        return true;
    }
}
