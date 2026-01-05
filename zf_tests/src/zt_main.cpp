#include <zcl.h>

namespace zf {
    static t_b8 TestBits(t_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 TestSorting(t_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 TestList(t_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 TestHashMap(t_arena *const arena) {
        // @todo
        return true;
    }

    struct s_test {
        const char *title_cstr;
        t_b8 (*func)(t_arena *const arena);
    };

    static const t_static_array<s_test, 4> g_tests = {{
        {.title_cstr = "Bits", .func = TestBits},
        {.title_cstr = "Sorting", .func = TestSorting},
        {.title_cstr = "List", .func = TestList},
        {.title_cstr = "Hash Map", .func = TestHashMap},
    }};

    static void RunTests() {
        t_arena arena = f_mem_create_arena();
        ZF_DEFER({ f_mem_destroy_arena(&arena); });

        for (t_i32 i = 0; i < g_tests.g_len; i++) {
            f_io_log(ZF_STR_LITERAL("Running test \"%\"..."), f_strs_convert_cstr(g_tests[i].title_cstr));
            g_tests[i].func(&arena);
        }
    }
}

int main(const int arg_cnt, const char *const *const args) {
    zf::RunTests();
}
