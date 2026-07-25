#include <zcl.h>

static zcl::t_b8 TestBits(zcl::t_arena *const temp_arena) {
    // @todo
    return true;
}

static zcl::t_b8 TestSorting(zcl::t_arena *const temp_arena) {
    // @todo
    return true;
}

static zcl::t_b8 TestList(zcl::t_arena *const temp_arena) {
    // @todo
    return true;
}

static zcl::t_b8 TestHashMap(zcl::t_arena *const temp_arena) {
    // @todo
    return true;
}

struct t_test {
    zcl::t_str_rdonly title;
    zcl::t_b8 (*func)(zcl::t_arena *const temp_arena);
};

static const zcl::t_static_array<t_test, 4> g_tests = {{
    {.title = ZCL_STR_LITERAL("Bits"), .func = TestBits},
    {.title = ZCL_STR_LITERAL("Sorting"), .func = TestSorting},
    {.title = ZCL_STR_LITERAL("List"), .func = TestList},
    {.title = ZCL_STR_LITERAL("Hash Map"), .func = TestHashMap},
}};

static void RunTests() {
    zcl::t_arena *const arena = zcl::ArenaCreateBlockBased();
    ZCL_DEFER({ zcl::ArenaDestroy(arena); });

    for (zcl::t_i32 i = 0; i < g_tests.k_len; i++) {
        zcl::Log(ZCL_STR_LITERAL("Running test \"%\"..."), g_tests[i].title);
        g_tests[i].func(arena);
    }
}

int main(const int arg_cnt, const char *const *const args) {
    RunTests();
}
