#include <zcl.h>

static zcl::t_b8 TestBits(zcl::t_arena *const arena) {
    // @todo
    return true;
}

static zcl::t_b8 TestSorting(zcl::t_arena *const arena) {
    // @todo
    return true;
}

static zcl::t_b8 TestList(zcl::t_arena *const arena) {
    // @todo
    return true;
}

static zcl::t_b8 TestHashMap(zcl::t_arena *const arena) {
    // @todo
    return true;
}

struct t_test {
    const char *title_c_str;
    zcl::t_b8 (*func)(zcl::t_arena *const arena);
};

constexpr zcl::t_static_array<t_test, 4> k_tests = {{
    {.title_c_str = "Bits", .func = TestBits},
    {.title_c_str = "Sorting", .func = TestSorting},
    {.title_c_str = "List", .func = TestList},
    {.title_c_str = "Hash Map", .func = TestHashMap},
}};

static void RunTests() {
    zcl::t_arena arena = zcl::ArenaCreateBlockBased();
    ZCL_DEFER({ zcl::ArenaDestroy(&arena); });

    for (zcl::t_i32 i = 0; i < k_tests.k_len; i++) {
        zcl::Log(ZCL_STR_LITERAL("Running test \"%\"..."), zcl::CStrToStr(k_tests[i].title_c_str));
        k_tests[i].func(&arena);
    }
}

int main(const int arg_cnt, const char *const *const args) {
    RunTests();
}
