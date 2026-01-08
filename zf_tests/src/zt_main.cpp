#include <zcl.h>

namespace zf {
    static t_b8 test_bits(mem::t_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 test_sorting(mem::t_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 test_list(mem::t_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 test_hash_map(mem::t_arena *const arena) {
        // @todo
        return true;
    }

    struct t_test {
        const char *title_cstr;
        t_b8 (*func)(mem::t_arena *const arena);
    };

    constexpr t_static_array<t_test, 4> k_tests = {{
        {.title_cstr = "Bits", .func = test_bits},
        {.title_cstr = "Sorting", .func = test_sorting},
        {.title_cstr = "List", .func = test_list},
        {.title_cstr = "Hash Map", .func = test_hash_map},
    }};

    static void run_tests() {
        mem::t_arena arena = mem::arena_create_blockbased();
        ZF_DEFER({ mem::arena_destroy(&arena); });

        for (t_i32 i = 0; i < k_tests.k_len; i++) {
            io::log(ZF_STR_LITERAL("Running test \"%\"..."), strs::cstr_to_str(k_tests[i].title_cstr));
            k_tests[i].func(&arena);
        }
    }
}

int main(const int arg_cnt, const char *const *const args) {
    zf::run_tests();
}
