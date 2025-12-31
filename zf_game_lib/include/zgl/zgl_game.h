#pragma once

#include <zcl.h>

namespace zf {
    struct s_game_init_context {
        s_arena *perm_arena;
        s_arena *temp_arena;
    };

    struct s_input_state;

    struct s_game_tick_context {
        s_arena *perm_arena;
        s_arena *temp_arena;

        const s_input_state *input_state;
    };

    struct s_rendering_context;

    struct s_game_render_context {
        s_arena *perm_arena;
        s_arena *temp_arena;

        s_rendering_context *rendering_context;
    };

    using t_game_init_func = void (*)(const s_game_init_context &context);
    using t_game_deinit_func = void (*)();
    using t_game_tick_func = void (*)(const s_game_tick_context &context);
    using t_game_render_func = void (*)(const s_game_render_context &context);

    void RunGame(const t_game_init_func init_func, const t_game_tick_func tick_func, const t_game_render_func render_func, const t_game_deinit_func deinit_func = nullptr);
}
