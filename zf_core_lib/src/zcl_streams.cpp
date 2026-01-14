#include <zcl/zcl_streams.h>

namespace zcl {
    t_b8 bitset_serialize(const t_stream stream, const t_bitset_rdonly bs) {
        if (!stream_write_item(stream, bs.bit_cnt)) {
            return false;
        }

        if (!stream_write_items_of_array(stream, BitsetGetBytes(bs))) {
            return false;
        }

        return true;
    }

    t_b8 bitset_deserialize(const t_stream stream, t_arena *const bs_arena, t_bitset_mut *const o_bs) {
        t_i32 bit_cnt;

        if (!stream_read_item(stream, &bit_cnt)) {
            return false;
        }

        *o_bs = BitsetCreate(bit_cnt, bs_arena);

        if (!stream_read_items_into_array(stream, BitsetGetBytes(*o_bs), BitsetGetBytes(*o_bs).len)) {
            return false;
        }

        return true;
    }
}
