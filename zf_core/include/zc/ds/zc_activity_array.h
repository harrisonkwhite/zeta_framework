#pragma once

#include <zc/ds/zc_bit_vector.h>

// An "activity array" is an array where each slot has an associated "active" bit indicating whether it is in use.
// A "versioned activity array" (VAA) augments this with a version number per slot, so that slot lifetimes can be uniquely identified.

namespace zf {
    template<typename tp_derived_type, typename tp_elem_type>
    class c_activity_array_base {
        static_assert(!s_is_const<tp_elem_type>::sm_value);

    public:
        c_array<tp_elem_type> Slots() const {
            return static_cast<const tp_derived_type*>(this)->Slots();
        }

        c_bit_vector SlotActivity() const {
            return static_cast<const tp_derived_type*>(this)->SlotActivity();
        }

        tp_elem_type& operator[](const t_size index) const {
            ZF_ASSERT(IsSlotActive(index));
            return Slots()[index];
        }

        t_size Len() const {
            return Slots().Len();
        }

        void ActivateSlot(const t_size index) const {
            ZF_ASSERT(!IsSlotActive(index));
            SetBit(SlotActivity(), index);
        }

        void DeactivateSlot(const t_size index) const {
            ZF_ASSERT(IsSlotActive(index));
            UnsetBit(SlotActivity(), index);
        }

        t_b8 IsSlotActive(const t_size index) const {
            return IsBitSet(SlotActivity(), index);
        }

        t_size IndexOfFirstActiveSlot(const t_size from = 0) const {
            ZF_ASSERT(from >= 0 && from <= Slots().Len());
            return FindFirstSetBit(SlotActivity(), from);
        }

        // Returns the index of the newly taken (activated) slot, or -1 if all slots are already active.
        t_size TakeFirstInactiveSlot() const {
            const t_size index = FindFirstUnsetBit(SlotActivity());

            if (index != -1) {
                ActivateSlot(index);
            }

            return index;
        }

    private:
        c_array<tp_elem_type> m_slots;
        c_bit_vector m_slot_activity;
    };

    template<typename tp_type>
    class c_activity_array : public c_activity_array_base<c_activity_array<tp_type>, tp_type> {
    public:
        t_b8 Init(c_mem_arena& mem_arena, const t_size len) {
            ZF_ASSERT(len > 0);

            c_array<tp_type> slots;
            c_bit_vector slot_activity;

            if (!MakeArray(mem_arena, len, slots)) {
                return false;
            }

            if (!MakeBitVector(mem_arena, len, slot_activity)) {
                return false;
            }

            m_slots = slots;
            m_slot_activity = slot_activity;

            return true;
        }

        c_array<tp_type> Slots() const {
            return m_slots;
        }

        c_bit_vector SlotActivity() const {
            return m_slot_activity;
        }

    private:
        c_array<tp_type> m_slots;
        c_bit_vector m_slot_activity;
    };

    template<typename tp_type, t_size tp_len>
    class c_static_activity_array : public c_activity_array_base<c_static_activity_array<tp_type, tp_len>, tp_type> {
    public:
        c_array<tp_type> Slots() const {
            return m_slots;
        }

        c_bit_vector SlotActivity() const {
            return m_slot_activity;
        }

    private:
        s_static_array<tp_type, tp_len> m_slots;
        s_static_bit_vector<tp_len> m_slot_activity;
    };
}
