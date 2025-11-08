#pragma once

#include <cstddef>
#include <zc/debug.h>
#include <zc/math.h>
#include <zc/mem/mem.h>
#include <zc/mem/strs.h>
#include <zf/rendering.h>
#include <zf/window.h>

namespace zf {
    struct s_game_init_context {
        void* dev_mem;

        c_mem_arena& perm_mem_arena;
        c_mem_arena& temp_mem_arena;
    };

    enum e_game_tick_result {
        ek_game_tick_result_normal, // Continue running the game as normal.
        ek_game_tick_result_exit,
        ek_game_tick_result_error
    };

    struct s_game_tick_context {
        void* dev_mem;

        c_mem_arena& perm_mem_arena;
        c_mem_arena& temp_mem_arena;
    };

    struct s_game_render_context {
        void* dev_mem;

        c_mem_arena& perm_mem_arena;
        c_mem_arena& temp_mem_arena;

        c_renderer& renderer;
    };

    struct s_game_info {
        s_v2<int> window_init_size;
        s_str_view window_title;
        e_window_flags window_flags;

        size_t dev_mem_size; // How much memory should be allocated in the permanent arena for your use? This might be the size of a specific struct, for example.
        size_t dev_mem_alignment; // The alignment of the above memory.

        int targ_ticks_per_sec;

        // Below are pointers to functions that the framework will call for you. The provided struct pointers expose parts of the framework state for you to work with.
        bool (* init_func)(const s_game_init_context& zf_context); // Called as one of the last steps of the game initialisation phase.
        e_game_tick_result (* tick_func)(const s_game_tick_context& zf_context); // Called once every tick (which can occur multiple times a frame).
        bool (* render_func)(const s_game_render_context& zf_context); // Called after all ticks have been run.
        void (* clean_func)(void* const dev_mem); // Called when the game ends (including if it ends in error). This is not called if your initialisation function failed or hasn't yet been called.
    };

    inline void AssertGameInfoValidity(const s_game_info& info) {
        ZF_ASSERT(info.window_init_size.x > 0 && info.window_init_size.y > 0);
        ZF_ASSERT(info.window_title.IsTerminated());

        ZF_ASSERT((info.dev_mem_size == 0 && info.dev_mem_alignment == 0)
            || (info.dev_mem_size > 0 && IsAlignmentValid(info.dev_mem_alignment)));

        ZF_ASSERT(info.targ_ticks_per_sec > 0);

        ZF_ASSERT(info.init_func);
        ZF_ASSERT(info.tick_func);
        ZF_ASSERT(info.render_func);
    }

    // @idea: Give ZF core a distinct namespace. If something cannot run outside the call stack of this RunGame function, it should be in this ZF engine library.
    [[nodiscard]] bool RunGame(const s_game_info& info);
}
