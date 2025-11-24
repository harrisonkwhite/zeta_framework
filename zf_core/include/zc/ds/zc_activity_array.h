#pragma once

#include <zc/ds/zc_bit_vector.h>

// An "activity array" is an array where each slot has an associated "active" bit indicating whether it is in use.

namespace zf {
    template<typename tp_type>
    struct s_activity_array_rdonly {
        s_array_rdonly<tp_type> slots;
        s_bit_range_rdonly slot_activity;

        constexpr const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(IsSlotActive(*this, index));
            return slots[index];
        }
    };

    template<typename tp_type>
    struct s_activity_array {
        s_array<tp_type> slots;
        s_bit_range slot_activity;

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

    template<typename tp_type> struct s_is_activity_array { static constexpr t_b8 g_val = false; };
    template<typename tp_type> struct s_is_activity_array<s_activity_array<tp_type>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type> struct s_is_activity_array<const s_activity_array<tp_type>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type> struct s_is_activity_array<s_activity_array_rdonly<tp_type>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type> struct s_is_activity_array<const s_activity_array_rdonly<tp_type>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type, t_size tp_len> struct s_is_activity_array<s_static_activity_array<tp_type, tp_len>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type, t_size tp_len> struct s_is_activity_array<const s_static_activity_array<tp_type, tp_len>> { static constexpr t_b8 g_val = true; };

    template<typename tp_type> struct s_is_mut_activity_array { static constexpr t_b8 g_val = false; };
    template<typename tp_type> struct s_is_mut_activity_array<s_activity_array<tp_type>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type, t_size tp_len> struct s_is_mut_activity_array<s_static_activity_array<tp_type, tp_len>> { static constexpr t_b8 g_val = true; };

    template<typename tp_type> concept c_activity_array = s_is_activity_array<tp_type>::g_val;
    template<typename tp_type> concept c_activity_array_mut = s_is_mut_activity_array<tp_type>::g_val;
    template<typename tp_type> concept c_activity_array_rdonly = s_is_activity_array<tp_type>::g_val && !s_is_mut_activity_array<tp_type>::g_val;

    template<c_activity_array tp_type>
    t_b8 IsSlotActive(tp_type& aa, const t_size index) {
        return IsBitSet(aa.slot_activity, index);
    }

    template<c_activity_array_mut tp_type>
    void ActivateSlot(tp_type& aa, const t_size index) {
        ZF_ASSERT(!IsSlotActive(aa, index));
        SetBit(aa.slot_activity, index);
    }

    template<c_activity_array_mut tp_type>
    void DeactivateSlot(tp_type& aa, const t_size index) {
        ZF_ASSERT(IsSlotActive(aa, index));
        UnsetBit(aa.slot_activity, index);
    }

    // Returns -1 if not found.
    template<c_activity_array tp_type>
    t_size IndexOfFirstActiveSlot(tp_type& aa, const t_size from = 0) {
        ZF_ASSERT(from >= 0 && from <= aa.slots.len);
        return IndexOfFirstSetBit(aa.slot_activity, from);
    }

    // Returns -1 if not found.
    template<c_activity_array tp_type>
    t_size IndexOfFirstInactiveSlot(tp_type& aa, const t_size from = 0) {
        ZF_ASSERT(from >= 0 && from <= aa.slots.len);
        return IndexOfFirstUnsetBit(aa.slot_activity, from);
    }

    // Returns the index of the newly taken (activated) slot, or -1 if all slots are already active.
    template<c_activity_array_mut tp_type>
    t_size TakeFirstInactiveSlot(tp_type& aa) {
        const t_size index = IndexOfFirstUnsetBit(aa.slot_activity);

        if (index != -1) {
            ActivateSlot(aa, index);
        }

        return index;
    }

    template<typename tp_type>
    [[nodiscard]] t_b8 MakeActivityArray(s_mem_arena& mem_arena, const t_size len, s_activity_array<tp_type>& o_aa) {
        ZF_ASSERT(len > 0);

        o_aa = {};

        if (!MakeArray(mem_arena, len, o_aa.slots)) {
            return false;
        }

        if (!MakeBitVector(mem_arena, len, o_aa.slot_activity)) {
            return false;
        }

        return true;
    }
}
