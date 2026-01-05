#include <zcl.h>

namespace zf {
    static t_b8 f_test_bits(mem::t_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 f_test_sorting(mem::t_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 f_test_list(mem::t_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 f_test_hash_map(mem::t_arena *const arena) {
        // @todo
        return true;
    }

    struct t_test {
        const char *title_cstr;
        t_b8 (*func)(mem::t_arena *const arena);
    };

    static const t_static_array<t_test, 4> g_tests = {{
        {.title_cstr = "Bits", .func = f_test_bits},
        {.title_cstr = "Sorting", .func = f_test_sorting},
        {.title_cstr = "List", .func = f_test_list},
        {.title_cstr = "Hash Map", .func = f_test_hash_map},
    }};

    static void run_tests() {
        mem::t_arena arena = mem::arena_create();
        ZF_DEFER({ mem::arena_destroy(&arena); });

        for (t_i32 i = 0; i < g_tests.g_len; i++) {
            io::f_log(ZF_STR_LITERAL("Running test \"%\"..."), strs::f_convert_cstr(g_tests[i].title_cstr));
            g_tests[i].func(&arena);
        }
    }
}

int main(const int arg_cnt, const char *const *const args) {
    zf::run_tests();
}
