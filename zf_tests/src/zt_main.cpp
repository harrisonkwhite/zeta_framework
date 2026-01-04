#include <zcl.h>

namespace zf {
    static B8 TestBits(s_arena *const arena) {
        // @todo
        return true;
    }

    static B8 TestSorting(s_arena *const arena) {
        // @todo
        return true;
    }

    static B8 TestList(s_arena *const arena) {
        // @todo
        return true;
    }

    static B8 TestHashMap(s_arena *const arena) {
        // @todo
        return true;
    }

    struct s_test {
        const char *title_cstr;
        B8 (*func)(s_arena *const arena);
    };

    static const s_static_array<s_test, 4> g_tests = {{
        {.title_cstr = "Bits", .func = TestBits},
        {.title_cstr = "Sorting", .func = TestSorting},
        {.title_cstr = "List", .func = TestList},
        {.title_cstr = "Hash Map", .func = TestHashMap},
    }};

    static void RunTests() {
        s_arena arena = CreateArena();
        ZF_DEFER({ ArenaDestroy(&arena); });

        for (t_i32 i = 0; i < g_tests.g_len; i++) {
            Log(ZF_STR_LITERAL("Running test \"%\"..."), strs::convert_cstr(g_tests[i].title_cstr));
            g_tests[i].func(&arena);
        }
    }
}

int main(const int arg_cnt, const char *const *const args) {
    zf::RunTests();
}
