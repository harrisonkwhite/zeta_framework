#include <zcl.h>

namespace zf {
    constexpr t_len g_mem_arena_size = zf::Megabytes(20);

    static void TestHashMap(s_mem_arena &mem_arena) {
#if 0
        auto names_to_ages = CreateHashMap<s_str_rdonly, t_i32>(g_str_hash_func, mem_arena, 1024, g_str_bin_comparator);
        ZF_REQUIRE(names_to_ages.IsActive());
        ZF_REQUIRE(names_to_ages.EntryCount() == 0);
        ZF_REQUIRE(names_to_ages.Cap() == 1024);
#endif

        // names_to_ages.Put(s_cstr_literal("Harry"), 22);
        // ZF_REQUIRE(names_to_ages.Get(s_cstr_literal("Harry")));
    }

    static void RunTests() {
        s_mem_arena mem_arena = CreateMemArena(g_mem_arena_size);
        ZF_DEFER({ mem_arena.Release(); });

        TestHashMap(mem_arena);
    }
}

int main(const int arg_cnt, const char *const *const args) {
    zf::RunTests();
}
