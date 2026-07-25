#include <zcl.h>

static void TestSorting(zcl::t_rng *const rng, zcl::t_arena *const temp_arena) {
    zcl::t_static_array<zcl::t_i32, 32> nums = {};

    for (zcl::t_i32 i = 0; i < nums.k_len; i++) {
        nums[i] = zcl::RandGenI32(rng);
    }

    zcl::RunBubbleSort(zcl::ArrayToNonstatic(&nums));

    ZCL_REQUIRE(zcl::CheckSorted(zcl::ArrayToNonstatic(&nums)));
}

static void TestList(zcl::t_rng *const rng, zcl::t_arena *const temp_arena) {
}

static void TestHashMap(zcl::t_rng *const rng, zcl::t_arena *const temp_arena) {
}

struct t_test {
    zcl::t_str_rdonly title;
    void (*func)(zcl::t_rng *const rng, zcl::t_arena *const temp_arena);
};

static const zcl::t_static_array<t_test, 3> g_tests = {{
    {.title = ZCL_STR_LITERAL("Sorting"), .func = TestSorting},
    {.title = ZCL_STR_LITERAL("List"), .func = TestList},
    {.title = ZCL_STR_LITERAL("Hash Map"), .func = TestHashMap},
}};

static void RunTests() {
    zcl::t_arena *const arena = zcl::ArenaCreateBlockBased();
    ZCL_DEFER({ zcl::ArenaDestroy(arena); });

    zcl::t_rng *const rng = zcl::RNGCreate(zcl::RandGenSeed(), arena);

    for (zcl::t_i32 i = 0; i < g_tests.k_len; i++) {
        zcl::Log(ZCL_STR_LITERAL("Running test \"%\"..."), g_tests[i].title);
        g_tests[i].func(rng, arena);
        zcl::ArenaRewind(arena);
    }

    zcl::Log(ZCL_STR_LITERAL("All tests completed!"));
}

int main(const int arg_cnt, const char *const *const args) {
    RunTests();
}
