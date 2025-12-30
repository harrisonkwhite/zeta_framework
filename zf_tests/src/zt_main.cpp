#include <zcl.h>

namespace zf {
    static t_b8 TestBits(c_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 TestSorting(c_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 TestList(c_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 TestHashMap(c_arena *const arena) {
        // @todo
        return true;
    }

    struct s_test {
        s_cstr_literal title;
        t_b8 (*func)(c_arena *const arena) = nullptr;
    };

    constexpr s_static_array<s_test, 4> g_tests = {{
        {.title = "Bits", .func = TestBits},
        {.title = "Sorting", .func = TestSorting},
        {.title = "List", .func = TestList},
        {.title = "Hash Map", .func = TestHashMap},
    }};

    static void RunTests() {
        c_arena arena;
        ZF_DEFER({ arena.Release(); });

        for (t_i32 i = 0; i < g_tests.g_len; i++) {
            Log(s_cstr_literal("Running test \"%\"..."), g_tests[i].title);
            g_tests[i].func(&arena);
        }
    }
}

int main(const int arg_cnt, const char *const *const args) {
    zf::RunTests();
}
