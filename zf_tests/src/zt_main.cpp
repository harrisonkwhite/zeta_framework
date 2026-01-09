#include <zcl.h>

static zcl::t_b8 test_bits(zcl::mem::t_arena *const arena) {
    // @todo
    return true;
}

static zcl::t_b8 test_sorting(zcl::mem::t_arena *const arena) {
    // @todo
    return true;
}

static zcl::t_b8 test_list(zcl::mem::t_arena *const arena) {
    // @todo
    return true;
}

static zcl::t_b8 test_hash_map(zcl::mem::t_arena *const arena) {
    // @todo
    return true;
}

struct t_test {
    const char *title_cstr;
    zcl::t_b8 (*func)(zcl::mem::t_arena *const arena);
};

constexpr zcl::t_static_array<t_test, 4> k_tests = {{
    {.title_cstr = "Bits", .func = test_bits},
    {.title_cstr = "Sorting", .func = test_sorting},
    {.title_cstr = "List", .func = test_list},
    {.title_cstr = "Hash Map", .func = test_hash_map},
}};

static void run_tests() {
    zcl::mem::t_arena arena = zcl::mem::arena_create_blockbased();
    ZF_DEFER({ zcl::mem::arena_destroy(&arena); });

    for (zcl::t_i32 i = 0; i < k_tests.k_len; i++) {
        zcl::io::log(ZF_STR_LITERAL("Running test \"%\"..."), zcl::strs::cstr_to_str(k_tests[i].title_cstr));
        k_tests[i].func(&arena);
    }
}

int main(const int arg_cnt, const char *const *const args) {
    run_tests();
}
