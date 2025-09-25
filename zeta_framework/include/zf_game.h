#pragma once

#include <cstddef>
#include <cassert>
#include <zc.h>
#include "zf_input.h"
#include "zf_rendering.h"

namespace zf {
    enum e_window_flags {
        ek_window_flags_resizable = 1 << 0,
        ek_window_flags_hide_cursor = 1 << 1
    };

    struct s_window_state {
        s_v2_s32 pos;
        s_v2_s32 size;
        bool fullscreen;
    };

    struct s_game_init_context {
        void* dev_mem;

        c_mem_arena& perm_mem_arena;
        c_mem_arena& temp_mem_arena;

        s_window_state window_state;

        s_gl_resource_arena& gl_res_arena;

        s_rendering_basis& rendering_basis;
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

        s_window_state window_state;

        s_input_context input_context;

        s_gl_resource_arena& gl_res_arena;

        s_rendering_basis& rendering_basis;
    };

    struct s_game_render_context {
        void* dev_mem;

        c_mem_arena& perm_mem_arena;
        c_mem_arena& temp_mem_arena;

        s_v2 mouse_pos;

        s_rendering_context rendering_context;
    };

    struct s_game_info {
        s_v2_s32 window_init_size;
        c_string_view window_title;
        e_window_flags window_flags;

        size_t dev_mem_size; // How much memory should be allocated in the permanent arena for your use? This might be the size of a specific struct, for example.
        size_t dev_mem_alignment; // The alignment of the above memory.

        t_s32 targ_ticks_per_sec;

        // Below are pointers to functions that the framework will call for you. The provided struct pointers expose parts of the framework state for you to work with.
        bool (*init_func)(const s_game_init_context& zfw_context); // Called as one of the last steps of the game initialisation phase.
        e_game_tick_result (*tick_func)(const s_game_tick_context& zfw_context); // Called once every tick (which can occur multiple times a frame).
        bool (*render_func)(const s_game_render_context& zfw_context); // Called after all ticks have been run.
        void (*clean_func)(void* const dev_mem); // Called when the game ends (including if it ends in error). This is not called if your initialisation function failed or hasn't yet been called.
    };

    inline void AssertGameInfoValidity(const s_game_info& info) {
        assert(info.window_init_size.x > 0 && info.window_init_size.y > 0);
        //assert(IsStrTerminated(info.window_title));

        assert((info.dev_mem_size == 0 && info.dev_mem_alignment == 0)
            || (info.dev_mem_size > 0 && IsAlignmentValid(info.dev_mem_alignment)));

        assert(info.targ_ticks_per_sec > 0);

        assert(info.init_func);
        assert(info.tick_func);
        assert(info.render_func);
    }

    [[nodiscard]] bool RunGame(const s_game_info& info);
}
