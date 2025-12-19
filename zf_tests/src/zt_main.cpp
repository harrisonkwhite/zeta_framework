#include <zcl.h>

namespace zf {
    constexpr t_len g_mem_arena_size = zf::Megabytes(20);

    static t_b8 TestBits(s_mem_arena &mem_arena) {
        s_bit_vec bv = AllocBitVec(32, mem_arena);

        return true;
    }

    static t_b8 TestList(s_mem_arena &mem_arena) {
        s_list<t_i32> list;

        return true;
    }

    static t_b8 TestHashMap(s_mem_arena &mem_arena) {
        auto hm = CreateHashMap<t_i32, t_i32>([](const t_i32 &key) { return static_cast<t_len>(key); }, mem_arena);
        ZF_REQUIRE(hm.IsActive());
        ZF_REQUIRE(hm.EntryCount() == 0);
        ZF_REQUIRE(hm.Cap() == g_hash_map_cap_default);

        hm.Put(2, 3);
        ZF_REQUIRE(hm.EntryCount() == 1);
        ZF_REQUIRE(hm.Cap() == g_hash_map_cap_default);

        t_i32 val;
        const t_b8 found = hm.Get(2, &val);
        ZF_REQUIRE(found && val == 3);

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

        for (t_len i = 0; i < g_tests.g_len; i++) {
            Log(s_cstr_literal("Running test \"%\"..."), g_tests[i].title);
            g_tests[i].func(mem_arena);
        }
    }
}

int main(const int arg_cnt, const char *const *const args) {
    zf::RunTests();
}
