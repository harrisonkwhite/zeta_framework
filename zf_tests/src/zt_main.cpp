#include <zcl.h>

static zcl::t_b8 test_bits(zcl::t_arena *const arena) {
    // @todo
    return true;
}

static zcl::t_b8 test_sorting(zcl::t_arena *const arena) {
    // @todo
    return true;
}

static zcl::t_b8 test_list(zcl::t_arena *const arena) {
    // @todo
    return true;
}

static zcl::t_b8 test_hash_map(zcl::t_arena *const arena) {
    // @todo
    return true;
}

struct t_test {
    const char *title_cstr;
    zcl::t_b8 (*func)(zcl::t_arena *const arena);
};

constexpr zcl::t_static_array<t_test, 4> k_tests = {{
    {.title_cstr = "Bits", .func = test_bits},
    {.title_cstr = "Sorting", .func = test_sorting},
    {.title_cstr = "List", .func = test_list},
    {.title_cstr = "Hash Map", .func = test_hash_map},
}};

static void run_tests() {
    zcl::t_arena arena = zcl::arena_create_blockbased();
    ZCL_DEFER({ zcl::arena_destroy(&arena); });

    for (zcl::t_i32 i = 0; i < k_tests.k_len; i++) {
        zcl::Log(ZCL_STR_LITERAL("Running test \"%\"..."), zcl::cstr_to_str(k_tests[i].title_cstr));
        k_tests[i].func(&arena);
    }
}

int main(const int arg_cnt, const char *const *const args) {
    run_tests();
}
