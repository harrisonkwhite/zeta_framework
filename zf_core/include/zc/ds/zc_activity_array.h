#pragma once

#include <zc/ds/zc_bit_vector.h>

// An "activity array" is an array where each slot has an associated "active" bit indicating whether it is in use.

namespace zf {
    template<typename tp_type>
    struct s_activity_array_rdonly {
        static_assert(!s_is_const<tp_type>::g_value);

        s_array<const tp_type> slots;
        s_bit_vector_rdonly slot_activity;

        constexpr const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(IsSlotActive(*this, index));
            return slots[index];
        }
    };

    template<typename tp_type>
    struct s_activity_array {
        static_assert(!s_is_const<tp_type>::g_value);

        s_array<tp_type> slots;
        s_bit_vector slot_activity;

        constexpr tp_type& operator[](const t_size index) const {
            ZF_ASSERT(IsSlotActive(*this, index));
            return slots[index];
        }

        constexpr operator s_activity_array_rdonly<tp_type>() const {
            return {slots, slot_activity};
        }
    };

    template<typename tp_type, t_size tp_len>
    struct s_static_activity_array {
        static_assert(!s_is_const<tp_type>::g_value);

        static constexpr t_size g_len = tp_len;

        s_static_array<tp_type, tp_len> slots;
        s_static_bit_vector<tp_len> slot_activity;

        constexpr tp_type& operator[](const t_size index) {
            ZF_ASSERT(IsSlotActive(*this, index));
            return slots[index];
        }

        constexpr const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(IsSlotActive(*this, index));
            return slots[index];
        }

        constexpr operator s_activity_array<tp_type>() {
            return {slots, slot_activity};
        }

        constexpr operator s_activity_array_rdonly<tp_type>() const {
            return {slots, slot_activity};
        }
    };

    template<typename tp_type, t_size tp_len>
    s_activity_array<tp_type> ToNonstatic(s_static_activity_array<tp_type, tp_len>& aa) {
        return static_cast<s_activity_array<tp_type>>(aa);
    }

    template<typename tp_type, t_size tp_len>
    s_activity_array_rdonly<tp_type> ToNonstatic(const s_static_activity_array<tp_type, tp_len>& aa) {
        return static_cast<s_activity_array_rdonly<tp_type>>(aa);
    }

    template<typename tp_type>
    t_b8 IsSlotActive(const s_activity_array_rdonly<tp_type>& aa, const t_size index) {
        return IsBitSet(aa.slot_activity, index);
    }

    template<typename tp_type>
    t_b8 IsSlotActive(const s_activity_array<tp_type>& aa, const t_size index) {
        return IsSlotActive(static_cast<s_activity_array_rdonly<tp_type>>(aa), index);
    }

    template<typename tp_type, t_size tp_len>
    t_b8 IsSlotActive(const s_static_activity_array<tp_type, tp_len>& aa, const t_size index) {
        return IsSlotActive(ToNonstatic(aa), index);
    }

    template<typename tp_type>
    void ActivateSlot(const s_activity_array<tp_type>& aa, const t_size index) {
        ZF_ASSERT(!IsSlotActive(aa, index));
        SetBit(aa.slot_activity, index);
    }

    template<typename tp_type, t_size tp_len>
    void ActivateSlot(s_static_activity_array<tp_type, tp_len>& aa, const t_size index) {
        ActivateSlot(ToNonstatic(aa), index);
    }

    template<typename tp_type>
    void DeactivateSlot(const s_activity_array<tp_type>& aa, const t_size index) {
        ZF_ASSERT(IsSlotActive(aa, index));
        UnsetBit(aa.slot_activity, index);
    }

    template<typename tp_type, t_size tp_len>
    void DeactivateSlot(s_static_activity_array<tp_type, tp_len>& aa, const t_size index) {
        DeactivateSlot(ToNonstatic(aa), index);
    }

    // Returns -1 if not found.
    template<typename tp_type>
    t_size IndexOfFirstActiveSlot(const s_activity_array_rdonly<tp_type>& aa, const t_size from = 0) {
        ZF_ASSERT(from >= 0 && from <= aa.slots.len);
        return IndexOfFirstSetBit(aa.slot_activity, from);
    }

    // Returns -1 if not found.
    template<typename tp_type>
    t_size IndexOfFirstActiveSlot(const s_activity_array<tp_type>& aa, const t_size from = 0) {
        return IndexOfFirstActiveSlot(static_cast<s_activity_array_rdonly<tp_type>>(aa), from);
    }

    // Returns -1 if not found.
    template<typename tp_type, t_size tp_len>
    t_size IndexOfFirstActiveSlot(const s_static_activity_array<tp_type, tp_len>& aa, const t_size from = 0) {
        return IndexOfFirstActiveSlot(ToNonstatic(aa), from);
    }

    // Returns -1 if not found.
    template<typename tp_type>
    t_size IndexOfFirstInactiveSlot(const s_activity_array_rdonly<tp_type>& aa, const t_size from = 0) {
        ZF_ASSERT(from >= 0 && from <= aa.slots.len);
        return IndexOfFirstUnsetBit(aa.slot_activity, from);
    }

    // Returns -1 if not found.
    template<typename tp_type>
    t_size IndexOfFirstInactiveSlot(const s_activity_array<tp_type>& aa, const t_size from = 0) {
        return IndexOfFirstInactiveSlot(static_cast<s_activity_array_rdonly<tp_type>>(aa), from);
    }

    // Returns -1 if not found.
    template<typename tp_type, t_size tp_len>
    t_size IndexOfFirstInactiveSlot(const s_static_activity_array<tp_type, tp_len>& aa, const t_size from = 0) {
        return IndexOfFirstInactiveSlot(ToNonstatic(aa), from);
    }

    // Returns the index of the newly taken (activated) slot, or -1 if all slots are already active.
    template<typename tp_type>
    t_size TakeFirstInactiveSlot(const s_activity_array<tp_type>& aa) {
        const t_size index = IndexOfFirstUnsetBit(aa.slot_activity);

        if (index != -1) {
            ActivateSlot(aa, index);
        }

        return index;
    }

    template<typename tp_type, t_size tp_len>
    t_size TakeFirstInactiveSlot(s_static_activity_array<tp_type, tp_len>& aa) {
        return TakeFirstInactiveSlot(ToNonstatic(aa));
    }

    template<typename tp_type>
    t_b8 MakeActivityArray(s_mem_arena& mem_arena, const t_size len, s_activity_array<tp_type>& o_aa) {
        ZF_ASSERT(len > 0);

        s_array<tp_type> slots;
        s_bit_vector slot_activity;

        if (!MakeArray(mem_arena, len, slots)) {
            return false;
        }

        if (!MakeBitVector(mem_arena, len, slot_activity)) {
            return false;
        }

        o_aa = {slots, slot_activity};

        return true;
    }
}
