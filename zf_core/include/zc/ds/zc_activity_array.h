#pragma once

#include <zc/ds/zc_bit_vector.h>

// An "activity array" is an array where each slot has an associated "active" bit indicating whether it is in use.
// A "versioned activity array" (VAA) augments this with a version number per slot, so that slot lifetimes can be uniquely identified.

namespace zf {
    template<typename tp_type>
    struct s_activity_array {
        s_array<tp_type> slots;
        s_bit_vector slot_activity;

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(IsSlotActive(*this, index));
            return slots[index];
        }
    };

    template<typename tp_type, t_size tp_len>
    struct s_static_activity_array {
        s_static_array<tp_type, tp_len> slots;
        s_static_bit_vector<tp_len> slot_activity;

        constexpr operator s_activity_array<tp_type>() {
            return {slots, slot_activity};
        }
    };

    template<typename tp_type>
    t_b8 IsSlotActive(const s_activity_array<tp_type>& aa, const t_size index) {
        return IsBitSet(aa.SlotActivity(), index);
    }

    template<typename tp_type, t_size tp_len>
    t_b8 IsSlotActive(const s_static_activity_array<tp_type, tp_len>& aa, const t_size index) {
        return IsSlotActive(aa.ToNonstatic(), index);
    }

    template<typename tp_type>
    void ActivateSlot(const s_activity_array<tp_type>& aa, const t_size index) {
        ZF_ASSERT(!IsSlotActive(aa, index));
        SetBit(aa.SlotActivity(), index);
    }

    template<typename tp_type, t_size tp_len>
    void ActivateSlot(const s_static_activity_array<tp_type, tp_len>& aa, const t_size index) {
        ActivateSlot(aa.ToNonstatic(), index);
    }

    template<typename tp_type>
    void DeactivateSlot(const s_activity_array<tp_type>& aa, const t_size index) {
        ZF_ASSERT(IsSlotActive(aa, index));
        UnsetBit(aa.SlotActivity(), index);
    }

    template<typename tp_type, t_size tp_len>
    void DeactivateSlot(const s_static_activity_array<tp_type, tp_len>& aa, const t_size index) {
        DeactivateSlot(aa.ToNonstatic(), index);
    }

    template<typename tp_type>
    t_size IndexOfFirstActiveSlot(const s_activity_array<tp_type>& aa, const t_size from = 0) {
        ZF_ASSERT(from >= 0 && from <= aa.Slots().Len());
        return FindFirstSetBit(aa.SlotActivity(), from);
    }

    template<typename tp_type, t_size tp_len>
    t_size IndexOfFirstActiveSlot(const s_static_activity_array<tp_type, tp_len>& aa, const t_size from = 0) {
        return IndexOfFirstActiveSlot(aa.ToNonstatic(), from);
    }

    // Returns the index of the newly taken (activated) slot, or -1 if all slots are already active.
    template<typename tp_type>
    t_size TakeFirstInactiveSlot(const s_activity_array<tp_type>& aa) {
        const t_size index = FindFirstUnsetBit(aa.SlotActivity());

        if (index != -1) {
            ActivateSlot(aa, index);
        }

        return index;
    }

    template<typename tp_type, t_size tp_len>
    t_size TakeFirstInactiveSlot(const s_static_activity_array<tp_type, tp_len>& aa) {
        return TakeFirstInactiveSlot(aa.ToNonstatic());
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
