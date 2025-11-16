#pragma once

#include <zc/ds/zc_bit_vector.h>

// An "activity array" is an array where each slot has an associated "active" bit indicating whether it is in use.

namespace zf {

template<typename tp_type>
    class c_activity_array_ro {
    public:
        constexpr c_activity_array_ro() = default;

        constexpr c_activity_array_ro(const c_array<const tp_type> slots, const c_bit_vector_ro slot_activity)
            : m_slots(slots), m_slot_activity(slot_activity) {
            ZF_ASSERT(slot_activity.BitCount() == slots.Len());
        }

        constexpr c_array<const tp_type> Slots() const {
            return m_slots;
        }

        constexpr c_bit_vector_ro SlotActivity() const {
            return m_slot_activity;
        }

        constexpr const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(IsSlotActive(index));
            return m_slots[index];
        }

        constexpr t_size Len() const {
            return m_slots.Len();
        }

        constexpr t_b8 IsSlotActive(const t_size index) const {
            return IsBitSet(m_slot_activity, index);
        }

        // @todo
#if 0
        t_size IndexOfFirstActiveSlot(const t_size from = 0) const {
            ZF_ASSERT(from >= 0 && from <= m_slots.Len());
            return FindFirstSetBit(m_slot_activity, from);
        }
#endif

    private:
        c_array<const tp_type> m_slots;
        c_bit_vector_ro m_slot_activity;
    };

    template<typename tp_type>
    class c_activity_array_mut {
    public:
        constexpr c_activity_array_mut() = default;

        constexpr c_activity_array_mut(const c_array<tp_type> slots, const c_bit_vector_mut slot_activity)
            : m_slots(slots), m_slot_activity(slot_activity) {
            ZF_ASSERT(slot_activity.BitCount() == slots.Len());
        }

        constexpr c_array<tp_type> Slots() const {
            return m_slots;
        }

        constexpr c_bit_vector_mut SlotActivity() const {
            return m_slot_activity;
        }

        constexpr tp_type& operator[](const t_size index) const {
            ZF_ASSERT(IsSlotActive(index));
            return m_slots[index];
        }

        constexpr t_size Len() const {
            return m_slots.Len();
        }

        constexpr void ActivateSlot(const t_size index) const {
            SetBit(m_slot_activity, index);
        }

        constexpr void DeactivateSlot(const t_size index) const {
            UnsetBit(m_slot_activity, index);
        }

        constexpr t_b8 IsSlotActive(const t_size index) const {
            return IsBitSet(m_slot_activity, index);
        }

        t_size IndexOfFirstActiveSlot(const t_size from = 0) const {
            ZF_ASSERT(from >= 0 && from <= m_slots.Len());
            return FindFirstSetBit(m_slot_activity, from);
        }

        // Returns the index of the newly taken (activated) slot, or -1 if all slots are already active.
        t_size TakeFirstInactiveSlot() const {
            const t_size index = FindFirstUnsetBit(m_slot_activity);

            if (index != -1) {
                ActivateSlot(index);
            }

            return index;
        }

    private:
        c_array<tp_type> m_slots;
        c_bit_vector_mut m_slot_activity;
    };

    template<typename tp_type, t_size tp_len>
    struct s_static_activity_array {
        s_static_array<tp_type, tp_len> slots;
        s_static_bit_vector<tp_len> slot_activity;

        constexpr s_static_activity_array() = default;

        constexpr tp_type& operator[](const t_size index) {
            ZF_ASSERT(IsSlotActive(index));
            return slots[index];
        }

        constexpr const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(IsSlotActive(index));
            return slots[index];
        }

        constexpr t_size Len() const {
            return tp_len;
        }

        constexpr void ActivateSlot(const t_size index) const {
            SetBit(slot_activity, index);
        }

        constexpr void DeactivateSlot(const t_size index) const {
            UnsetBit(slot_activity, index);
        }

        constexpr t_b8 IsSlotActive(const t_size index) const {
            return IsBitSet(slot_activity, index);
        }

        t_size IndexOfFirstActiveSlot(const t_size from = 0) const {
            ZF_ASSERT(from >= 0 && from <= slots.Len());
            return FindFirstSetBit(slot_activity, from);
        }

        // Returns the index of the newly taken (activated) slot, or -1 if all slots are already active.
        t_size TakeFirstInactiveSlot() const {
            const t_size index = FindFirstUnsetBit(slot_activity);

            if (index != -1) {
                ActivateSlot(index);
            }

            return index;
        }

        constexpr c_activity_array_mut<tp_type> ToNonstatic() {
            return {slots, slot_activity};
        }

        constexpr operator c_activity_array_mut<tp_type>() {
            return ToNonstatic();
        }

        constexpr c_activity_array_ro<tp_type> ToNonstatic() const {
            return {slots, slot_activity};
        }

        constexpr operator c_activity_array_ro<tp_type>() const {
            return ToNonstatic();
        }
    };

    template<typename tp_type>
    t_b8 MakeActivityArray(c_mem_arena& mem_arena, const t_size len, c_activity_array_mut<tp_type>& o_arr) {
        ZF_ASSERT(len > 0);

        c_array<tp_type> slots;
        c_bit_vector_mut slot_activity;

        if (!MakeArray(mem_arena, len, slots)) {
            return false;
        }

        if (!MakeBitVector(mem_arena, len, slot_activity)) {
            return false;
        }

        o_arr = {slots, slot_activity};

        return true;
    }
}
