#include "zc_mem.h"

#include "zc_bit_vector.h"

namespace zf {
    template<typename tp_key_type, typename tp_value_type>
    class c_unordered_map {
    public:
        bool Init(c_mem_arena& mem_arena, const int init_bucket_cnt, const int init_backing_store_cap, int (* const hash_code_generator)(const tp_key_type& key)) {
            *this = {};

            m_mem_arena = &mem_arena;
            m_hash_code_generator = hash_code_generator;

            m_keys = mem_arena.PushArray<tp_key_type>();

            if (m_keys.IsEmpty()) {
                return false;
            }

            m_vals = mem_arena.PushArray<tp_value_type>();

            if (m_vals.IsEmpty()) {
                return false;
            }

            if (!m_buckets_in_use.Init(mem_arena, init_bucket_cnt)) {
                return false;
            }

            m_initted = true;

            return true;
        }

        void Put(const tp_key_type& key, const tp_value_type& val) {
            assert(m_initted);

            const int bucket_index = Wrap(m_hash_code_generator(key), 0, m_keys.Len());

            if (!m_buckets_in_use.IsBitSet(bucket_index)) {
                m_keys[bucket_index] = key;
                m_vals[bucket_index] = val;

                m_buckets_in_use.SetBit(bucket_index);

                // @todo: Resize the array if too many bits are set, to maintain ideal maximum load factor?
            } else {
                // @todo: Go into some backing array.
            }
        }

    private:
        bool m_initted = false;

        c_mem_arena* m_mem_arena = nullptr;
        int (*m_hash_code_generator)(const tp_key_type& key) = nullptr;

        c_array<tp_key_type> m_keys;
        c_array<tp_value_type> m_vals;
        c_bit_vector m_buckets_in_use;
    };
}
