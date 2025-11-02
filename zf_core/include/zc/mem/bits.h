#pragma once

#include <zc/mem/mem.h>
#include <zc/mem/arrays.h>

namespace zf {
    constexpr t_u8 BitMask(const size_t index) {
        return 1 << (index % 8);
    }

    static inline void SetBit(const c_array<t_u8> bytes, const size_t index) {
        bytes[index / 8] |= BitMask(index);
    }

    static inline void UnsetBit(const c_array<t_u8> bytes, const size_t index) {
        bytes[index / 8] &= ~BitMask(index);
    }

    static inline bool IsBitSet(const c_array<const t_u8> bytes, const size_t index) {
        return bytes[index / 8] & BitMask(index);
    }

    int IndexOfFirstSetBit(const c_array<const t_u8> bytes, const t_u8 xor = 0);

    static inline int IndexOfFirstUnsetBit(const c_array<const t_u8> bytes) {
        return IndexOfFirstSetBit(bytes, 0xFF);
    }

    void ShiftLeft(const c_array<t_u8> bytes, size_t amount, const bool rot = false);
    void ShiftRight(const c_array<t_u8> bytes, size_t amount, const bool rot = false);

    void And(const c_array<t_u8> bytes, const c_array<const t_u8> mask);
    void Or(const c_array<t_u8> bytes, const c_array<const t_u8> mask);
    void Xor(const c_array<t_u8> bytes, const c_array<const t_u8> mask);
}
