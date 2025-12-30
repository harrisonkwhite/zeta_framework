#include <zcl.h>

namespace zf {
    static t_b8 TestBits(s_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 TestSorting(s_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 TestList(s_arena *const arena) {
        // @todo
        return true;
    }

    static t_b8 TestHashMap(s_arena *const arena) {
        // @todo
        return true;
    }

    struct s_test {
        s_cstr_literal title;
        t_b8 (*func)(s_arena *const arena) = nullptr;
    };

    constexpr s_static_array<s_test, 4> g_tests = {{
        {.title = "Bits", .func = TestBits},
        {.title = "Sorting", .func = TestSorting},
        {.title = "List", .func = TestList},
        {.title = "Hash Map", .func = TestHashMap},
    }};

    static void RunTests() {
        s_arena arena = CreateArena();
        ZF_DEFER({ DestroyArena(&arena); });

        for (t_i32 i = 0; i < g_tests.g_len; i++) {
            Log(s_cstr_literal("Running test \"%\"..."), g_tests[i].title);
            g_tests[i].func(&arena);
        }
    }
}

int main(const int arg_cnt, const char *const *const args) {
    zf::RunTests();
}
