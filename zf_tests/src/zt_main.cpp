#include <zcl.h>

namespace zf {
    constexpr t_i32 g_mem_arena_size = zf::Megabytes(20);

    static t_b8 TestBits(s_mem_arena &mem_arena) {
        s_bit_vec bv = AllocBitVec(32, mem_arena);

        return true;
    }

    static t_b8 TestList(s_mem_arena &mem_arena) {
        s_list<t_i32> list;

        return true;
    }

    static t_b8 TestHashMap(s_mem_arena &mem_arena) {
        // Create a hash map with an identity hash function.
        auto hm = CreateHashMap<t_i32, t_i32>([](const t_i32 &key) { return key; }, mem_arena);

        // The hash map must be active after creation.
        ZF_REQUIRE(hm.IsActive());

        // The hash map must start empty.
        ZF_REQUIRE(hm.EntryCount() == 0);

        // The capacity must match the default.
        ZF_REQUIRE(hm.Cap() == g_hash_map_cap_default);

        // Insert a single element.
        hm.Put(2, 3);

        // The entry count must increase.
        ZF_REQUIRE(hm.EntryCount() == 1);

        // Retrieve the inserted value.
        {
            s_ptr<t_i32> val;
            const t_b8 found = hm.Find(2, val);
            ZF_REQUIRE(found);
            ZF_REQUIRE(*val == 3);
        }

        // Updating an existing key must not change the entry count.
        hm.Put(2, 5);
        ZF_REQUIRE(hm.EntryCount() == 1);

        // The value must be updated.
        {
            s_ptr<t_i32> val;
            const t_b8 found = hm.Find(2, val);
            ZF_REQUIRE(found);
            ZF_REQUIRE(*val == 5);
        }

        // Removing a non-existent key must fail.
        {
            const t_b8 removed = hm.Remove(1234);
            ZF_REQUIRE(!removed);
            ZF_REQUIRE(hm.EntryCount() == 1);
        }

        // Removing an existing key must succeed.
        {
            const t_b8 removed = hm.Remove(2);
            ZF_REQUIRE(removed);
            ZF_REQUIRE(hm.EntryCount() == 0);
        }

        // The removed key must no longer be found.
        {
            s_ptr<t_i32> val;
            const t_b8 found = hm.Find(2, val);
            ZF_REQUIRE(!found);
        }

        // Insert several keys that land in different buckets.
        for (t_i32 i = 0; i < 16; i++) {
            hm.Put(i, i * 10);
        }

        // The entry count must match the number of inserts.
        ZF_REQUIRE(hm.EntryCount() == 16);

        // All inserted keys must be retrievable.
        for (t_i32 i = 0; i < 16; i++) {
            s_ptr<t_i32> val;
            const t_b8 found = hm.Find(i, val);
            ZF_REQUIRE(found);
            ZF_REQUIRE(*val == i * 10);
        }

        // Remove every second key.
        for (t_i32 i = 0; i < 16; i += 2) {
            const t_b8 removed = hm.Remove(i);
            ZF_REQUIRE(removed);
        }

        // The entry count must reflect removals.
        ZF_REQUIRE(hm.EntryCount() == 8);

        // Removed keys must not be found.
        for (t_i32 i = 0; i < 16; i += 2) {
            s_ptr<t_i32> val;
            const t_b8 found = hm.Find(i, val);
            ZF_REQUIRE(!found);
        }

        // Remaining keys must still be found.
        for (t_i32 i = 1; i < 16; i += 2) {
            s_ptr<t_i32> val;
            const t_b8 found = hm.Find(i, val);
            ZF_REQUIRE(found);
            ZF_REQUIRE(*val == i * 10);
        }

        // Re-insert removed keys.
        for (t_i32 i = 0; i < 16; i += 2) {
            hm.Put(i, i * 100);
        }

        // The entry count must return to the full size.
        ZF_REQUIRE(hm.EntryCount() == 16);

        // Re-inserted values must be correct.
        for (t_i32 i = 0; i < 16; i++) {
            s_ptr<t_i32> val;
            const t_b8 found = hm.Find(i, val);
            ZF_REQUIRE(found);

            if ((i & 1) == 0) {
                ZF_REQUIRE(*val == i * 100);
            } else {
                ZF_REQUIRE(*val == i * 10);
            }
        }

        // Create a hash map that forces collisions.
        auto hm_collision = CreateHashMap<t_i32, t_i32>([](const t_i32 &) { return 0; }, mem_arena, 2);

        // Insert many colliding keys.
        for (t_i32 i = 0; i < 32; i++) {
            hm_collision.Put(i, i + 1);
        }

        // The entry count must include all colliding keys.
        ZF_REQUIRE(hm_collision.EntryCount() == 32);

        // All colliding keys must be retrievable.
        for (t_i32 i = 0; i < 32; i++) {
            s_ptr<t_i32> val;
            const t_b8 found = hm_collision.Find(i, val);
            ZF_REQUIRE(found);
            ZF_REQUIRE(*val == i + 1);
        }

        // Remove keys from the collision chain.
        for (t_i32 i = 0; i < 32; i += 3) {
            const t_b8 removed = hm_collision.Remove(i);
            ZF_REQUIRE(removed);
        }

        // The entry count must reflect removals.
        ZF_REQUIRE(hm_collision.EntryCount() == 32 - 11);

        // Removed collision keys must not be found.
        for (t_i32 i = 0; i < 32; i += 3) {
            ZF_REQUIRE(!hm_collision.Exists(i));
        }

        // Remaining collision keys must still be retrievable.
        for (t_i32 i = 0; i < 32; i++) {
            if ((i % 3) != 0) {
                s_ptr<t_i32> val;
                const t_b8 found = hm_collision.Find(i, val);
                ZF_REQUIRE(found);
                ZF_REQUIRE(val == i + 1);
            }
        }

        // Test loading entries into arrays.
        {
            s_array<t_i32> keys;
            s_array<t_i32> vals;

            hm.LoadEntries(mem_arena, keys, vals);

            // The loaded array size must match the entry count.
            ZF_REQUIRE(keys.Len() == hm.EntryCount());
            ZF_REQUIRE(vals.Len() == hm.EntryCount());

            // Each loaded key must be retrievable from the map.
            for (t_i32 i = 0; i < keys.Len(); i++) {
                s_ptr<t_i32> val;
                const t_b8 found = hm.Find(keys[i], val);
                ZF_REQUIRE(found);
                ZF_REQUIRE(val == vals[i]);
            }
        }

        return true;
    }

    struct s_test {
        s_cstr_literal title;
        t_b8 (*func)(s_mem_arena &mem_arena) = nullptr;
    };

    constexpr s_static_array<s_test, 3> g_tests = {
        {.title = "Bits", .func = TestBits},
        {.title = "List", .func = TestList},
        {.title = "Hash Map", .func = TestHashMap},
    };

    static void RunTests() {
        s_mem_arena mem_arena = CreateMemArena(g_mem_arena_size);
        ZF_DEFER({ mem_arena.Release(); });

        for (t_i32 i = 0; i < g_tests.g_len; i++) {
            Log(s_cstr_literal("Running test \"%\"..."), g_tests[i].title);
            g_tests[i].func(mem_arena);
        }
    }
}

int main(const int arg_cnt, const char *const *const args) {
    zf::RunTests();
}
