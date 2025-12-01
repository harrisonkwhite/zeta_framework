#pragma once

#include <zc.h>
#include <zf/zf_rendering.h>
#include <zf/zf_window.h>

namespace zf {
    struct s_game_init_context {
        void* dev_mem;

        s_mem_arena* mem_arena;
        s_mem_arena* temp_mem_arena;

#if 0
        gfx::s_resource_arena* gfx_res_arena;
#endif
    };

    enum e_game_tick_result {
        ek_game_tick_result_normal, // Continue running the game as normal.
        ek_game_tick_result_exit,
        ek_game_tick_result_error
    };

    struct s_game_tick_context {
        void* dev_mem;

        s_mem_arena* mem_arena;
        s_mem_arena* temp_mem_arena;
    };

    struct s_game_render_context {
        void* dev_mem;

        s_mem_arena* mem_arena;
        s_mem_arena* temp_mem_arena;

#if 0
        const s_rendering_context* rendering_context;
#endif
    };

    struct s_game_info {
        t_size mem_arena_size;
        t_size temp_mem_arena_size;

        s_v2<t_s32> window_init_size;
        s_str_rdonly window_title;
        e_window_flags window_flags;

        t_size dev_mem_size;
        t_size dev_mem_alignment;

        t_s32 targ_ticks_per_sec;

        // Below are pointers to functions that the framework will call for you. The provided struct pointers expose parts of the framework state for you to work with.
        t_b8 (* init_func)(const s_game_init_context& zf_context); // Called once as one of the last steps of the game initialisation phase.
        e_game_tick_result (* tick_func)(const s_game_tick_context& zf_context); // Called once every tick (which can occur multiple times a frame).
        t_b8 (* render_func)(const s_game_render_context& zf_context); // Called after all ticks of the frame have been run.
        void (* clean_func)(void* const dev_mem); // Optional. Called when the game ends (including if it ends in error). This is not called if your initialisation function failed or hasn't yet been called.
    };

    inline void AssertGameInfoValidity(const s_game_info& info) {
        ZF_ASSERT(info.mem_arena_size > 0);
        ZF_ASSERT(info.temp_mem_arena_size > 0 && info.temp_mem_arena_size <= info.mem_arena_size);

        ZF_ASSERT(info.window_init_size.x > 0 && info.window_init_size.y > 0);

        ZF_ASSERT((info.dev_mem_size == 0 && info.dev_mem_alignment == 0)
            || (info.dev_mem_size > 0 && IsAlignmentValid(info.dev_mem_alignment)));

        ZF_ASSERT(info.targ_ticks_per_sec > 0);

        ZF_ASSERT(info.init_func);
        ZF_ASSERT(info.tick_func);
        ZF_ASSERT(info.render_func);
    }

    t_b8 RunGame(const s_game_info& info);
}
