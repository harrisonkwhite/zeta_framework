#include <zcl.h>

static void TestSorting(zcl::t_rng *const rng, zcl::t_arena *const temp_arena) {
    enum t_sort_type {
        ek_sort_type_bubble,
        ek_sort_type_insertion,
        ek_sort_type_selection,
        ek_sort_type_merge,
        ek_sort_type_quick,

        ekm_sort_type_cnt,
    };

    const auto nums_generator = [rng, temp_arena](const zcl::t_i32 len) -> zcl::t_array_mut<zcl::t_i32> {
        ZCL_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const auto result = zcl::ArenaPushArray<zcl::t_i32>(temp_arena, len);

        for (zcl::t_i32 i = 0; i < result.len; i++) {
            result[i] = zcl::RandGenI32(rng);
        }

        return result;
    };

    for (zcl::t_i32 i = 0; i < ekm_sort_type_cnt; i++) {
        for (zcl::t_i32 len = 0; len < 128; len++) {
            const auto nums = nums_generator(len);

            switch (static_cast<t_sort_type>(i)) {
                case ek_sort_type_bubble: {
                    zcl::RunBubbleSort(nums);
                    break;
                }

                case ek_sort_type_insertion: {
                    zcl::RunInsertionSort(nums);
                    break;
                }

                case ek_sort_type_selection: {
                    zcl::RunSelectionSort(nums);
                    break;
                }

                case ek_sort_type_merge: {
                    zcl::RunMergeSort(nums, temp_arena);
                    break;
                }

                case ek_sort_type_quick: {
                    zcl::RunQuickSort(nums);
                    break;
                }

                case ekm_sort_type_cnt: {
                    ZCL_UNREACHABLE();
                }
            }

            ZCL_REQUIRE(zcl::CheckSorted(nums));

            zcl::ArenaRewind(temp_arena);
        }
    }
}

static void TestList(zcl::t_rng *const rng, zcl::t_arena *const temp_arena) {
}

static void TestHashMap(zcl::t_rng *const rng, zcl::t_arena *const temp_arena) {
}

struct t_test {
    zcl::t_str_rdonly title;
    void (*func)(zcl::t_rng *const rng, zcl::t_arena *const temp_arena);
};

static const zcl::t_static_array<t_test, 3> g_tests = {{
    {.title = ZCL_STR_LITERAL("Sorting"), .func = TestSorting},
    {.title = ZCL_STR_LITERAL("List"), .func = TestList},
    {.title = ZCL_STR_LITERAL("Hash Map"), .func = TestHashMap},
}};

static void RunTests() {
    zcl::t_arena *const arena = zcl::ArenaCreateBlockBased();
    ZCL_DEFER({ zcl::ArenaDestroy(arena); });

    zcl::t_rng *const rng = zcl::RNGCreate(zcl::RandGenSeed(), arena);

    for (zcl::t_i32 i = 0; i < g_tests.k_len; i++) {
        zcl::Log(ZCL_STR_LITERAL("Running test \"%\"..."), g_tests[i].title);
        g_tests[i].func(rng, arena);
        zcl::ArenaRewind(arena);
    }

    zcl::Log(ZCL_STR_LITERAL("All tests completed!"));
}

int main(const int arg_cnt, const char *const *const args) {
    RunTests();
}
