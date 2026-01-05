#pragma once

#include <zcl.h>

namespace zf {
    struct t_rendering_resource_group;

    struct t_game_init_context {
        t_arena *perm_arena;
        t_arena *temp_arena;

        t_rendering_resource_group *perm_rendering_resource_group;

        rand::RNG *rng;
    };

    struct t_input_state;

    struct t_game_tick_context {
        t_arena *perm_arena;
        t_arena *temp_arena;

        const t_input_state *input_state;

        t_rendering_resource_group *perm_rendering_resource_group;

        rand::RNG *rng;
    };

    struct t_rendering_context;

    struct t_game_render_context {
        t_arena *perm_arena;
        t_arena *temp_arena;

        t_rendering_context *rendering_context;

        rand::RNG *rng;
    };

    using t_game_init_func = void (*)(const t_game_init_context &context);
    using t_game_deinit_func = void (*)();
    using t_game_tick_func = void (*)(const t_game_tick_context &context);
    using t_game_render_func = void (*)(const t_game_render_context &context);

    void game_run(const t_game_init_func init_func, const t_game_tick_func tick_func, const t_game_render_func render_func, const t_game_deinit_func deinit_func = nullptr);
}
