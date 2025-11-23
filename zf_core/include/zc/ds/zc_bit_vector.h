#pragma once

#include <zc/zc_mem.h>

namespace zf {
    constexpr t_u8 ByteBitmask(const t_size bit_index) {
        ZF_ASSERT(bit_index >= 0 && bit_index < 8);
        return static_cast<t_u8>(1 << bit_index);
    }

    constexpr t_u8 ByteBitmask(const t_size begin_bit_index, const t_size end_bit_index) {
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < 8);
        ZF_ASSERT(end_bit_index > begin_bit_index && end_bit_index <= 8);

        const auto bits_at_bottom = static_cast<t_u8>((1 << (end_bit_index - begin_bit_index)) - 1);
        return static_cast<t_u8>(bits_at_bottom << begin_bit_index);
    }

    struct s_bit_vector_rdonly {
        s_array_rdonly<t_u8> bytes;
        t_size bit_cnt;
    };

    struct s_bit_vector {
        s_array<t_u8> bytes;
        t_size bit_cnt;

        constexpr operator s_bit_vector_rdonly() const {
            return {bytes, bit_cnt};
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
    };

    struct s_byte_stream_write {
        s_array<t_u8> bytes;
        t_size bytes_written;
    };

    struct s_byte_stream_read {
        s_array<t_u8> bytes;
        t_size bytes_read;
    };

    template<typename tp_type>
    [[nodiscard]] inline t_b8 SerializeItem(s_byte_stream_write& bs, const tp_type& item) {
        const t_size size = ZF_SIZE_OF(item);

        if (bs.bytes_written + size > bs.bytes.len) {
            return false;
        }

        const auto dest = Slice(bs.bytes, bs.bytes_written, bs.bytes_written + size);
        const auto src = ToBytes(item);
        Copy(dest, src);

        bs.bytes_written += size;

        return true;
    }

    template<typename tp_type>
    [[nodiscard]] inline t_b8 DeserializeItem(s_byte_stream_read& bs, tp_type& o_item) {
        const t_size size = ZF_SIZE_OF(tp_type);

        if (bs.bytes_read + size > bs.bytes.len) {
            return false;
        }

        const auto dest = ToBytes(o_item);
        const auto src = Slice(bs.bytes, bs.bytes_read, bs.bytes_read + size);
        Copy(dest, src);

        bs.bytes_read += size;

        return true;
    }

    template<c_array tp_type>
    [[nodiscard]] inline t_b8 SerializeItems(s_byte_stream_write& bs, tp_type& src_arr) {
        const t_size size = ArraySizeInBytes(src_arr);

        if (bs.bytes_written + size > bs.bytes.len) {
            return false;
        }

        const auto dest = Slice(bs.bytes, bs.bytes_written, bs.bytes_written + size);
        const auto src = ToByteArray(src_arr);
        Copy(dest, src);

        bs.bytes_written += size;

        return true;
    }

    template<c_array tp_type>
    [[nodiscard]] inline t_b8 DeserializeItems(s_byte_stream_read& bs, tp_type& dest_arr, const t_size cnt) {
        ZF_ASSERT(cnt >= 0 && cnt <= ArrayLen(dest_arr));

        for (t_size i = 0; i < cnt; i++) {
            if (!DeserializeItem(bs, dest_arr[i])) {
                return false;
            }
        }

        return true;
    }

    template<c_array tp_type>
    [[nodiscard]] inline t_b8 SerializeArray(s_byte_stream_write& bs, tp_type& arr) {
        if (!SerializeItem(bs, arr.len)) {
            return false;
        }

        if (!SerializeItems(bs, arr)) {
            return false;
        }

        return true;
    }

    template<typename tp_type>
    [[nodiscard]] inline t_b8 DeserializeArray(s_mem_arena& mem_arena, s_byte_stream_read& bs, s_array<tp_type>& o_arr) {
        if (!DeserializeItem(bs, o_arr.len)) {
            return false;
        }

        if (!MakeArray(mem_arena, o_arr.len, o_arr)) {
            return false;
        }

        if (!DeserializeItems(bs, o_arr, o_arr.len)) {
            return false;
        }

        return true;
    }

    // Serializes the bit vector EXCLUDING BYTES BEYOND THE BIT COUNT! For example, a bit vector with a bit count of 10 will only have its first 2 bytes serialized. Excess bits in the final byte are not zeroed out.
    [[nodiscard]] inline t_b8 SerializeBitVector(s_byte_stream_write& bs, const s_bit_vector_rdonly bv) {
        if (!SerializeItem(bs, bv.bit_cnt)) {
            return false;
        }

        const auto bytes_src = Slice(bv.bytes, 0, BitsToBytes(bv.bit_cnt));

        if (!SerializeItems(bs, bytes_src)) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 DeserializeBitVector(s_mem_arena& mem_arena, s_byte_stream_read& bs, s_bit_vector& o_bv) {
        if (!DeserializeItem(bs, o_bv.bit_cnt)) {
            return false;
        }

        if (!MakeArray(mem_arena, BitsToBytes(o_bv.bit_cnt), o_bv.bytes)) {
            return false;
        }

        if (!DeserializeItems(bs, o_bv.bytes, o_bv.bytes.len)) {
            return false;
        }

        return true;
    }

    void ShiftBitsLeft(const s_bit_vector bv, const t_size amount = 1);
    void RotBitsLeft(const s_bit_vector bv, const t_size amount = 1);

    t_b8 MakeBitVector(s_mem_arena& mem_arena, const t_size bit_cnt, s_bit_vector& o_bv);

    // @todo: Right (either arithmetic or logical) shift and right rotate.

    // Returns -1 if not found.
    t_size IndexOfFirstSetBit(const s_bit_vector_rdonly bv, const t_size from = 0, const t_b8 inverted = false); // Returns -1 if all bits are unset.

    // Returns -1 if not found.
    inline t_size IndexOfFirstUnsetBit(const s_bit_vector_rdonly bv, const t_size from = 0) {
        return IndexOfFirstSetBit(bv, from, true);
    }

    t_size CountSetBits(const s_bit_vector_rdonly bv);

    inline t_size CountUnsetBits(const s_bit_vector_rdonly bv) {
        return bv.bit_cnt - CountSetBits(bv);
    }

    constexpr t_b8 IsBitSet(const s_bit_vector_rdonly bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        return bv.bytes[index / 8] & (1 << (index % 8));
    }

    constexpr void SetBit(const s_bit_vector bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.bytes[index / 8] |= (1 << (index % 8));
    }

    constexpr void UnsetBit(const s_bit_vector bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.bytes[index / 8] &= ~(1 << (index % 8));
    }

    inline t_u8 BitVectorLastByteBitmask(const s_bit_vector_rdonly bv) {
        ZF_ASSERT(bv.bit_cnt > 0);

        const t_size bit_cnt = bv.bit_cnt - (8 * (bv.bytes.len - 1));
        return ByteBitmask(0, bit_cnt);
    }

    inline t_u8 BitVectorLastByte(const s_bit_vector_rdonly bv) {
        ZF_ASSERT(bv.bit_cnt > 0);
        return bv.bytes[bv.bytes.len - 1] & BitVectorLastByteBitmask(bv);
    }

    inline t_b8 AreAllBitsSet(const s_bit_vector_rdonly bv) {
        ZF_ASSERT(bv.bit_cnt > 0);

        for (t_size i = 0; i < bv.bytes.len - 1; i++) {
            if (bv.bytes[i] != 0xFF) {
                return false;
            }
        }

        return BitVectorLastByte(bv) == 0xFF;
    }

    inline t_b8 IsAnyBitSet(const s_bit_vector_rdonly bv) {
        for (t_size i = 0; i < bv.bytes.len - 1; i++) {
            if (bv.bytes[i] != 0) {
                return true;
            }
        }

        return BitVectorLastByte(bv) != 0;
    }

    inline t_b8 AreAllBitsUnset(const s_bit_vector_rdonly bv) {
        return !IsAnyBitSet(bv);
    }

    inline t_b8 IsAnyBitUnset(const s_bit_vector_rdonly bv) {
        return !AreAllBitsSet(bv);
    }
}
