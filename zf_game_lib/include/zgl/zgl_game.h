#pragma once

#include <zcl.h>
#include <zgl/zgl_input.h>
#include <zgl/zgl_rendering.h>

namespace zf {
    struct t_game_init_context {
        mem::t_arena *perm_arena;
        mem::t_arena *temp_arena;

        rendering::t_resource_group *perm_rendering_resource_group;

        rand::t_rng *rng;
    };

    struct t_game_tick_context {
        mem::t_arena *perm_arena;
        mem::t_arena *temp_arena;

        const input::t_state *input_state;

        rendering::t_resource_group *perm_rendering_resource_group;

        rand::t_rng *rng;
    };

    struct t_game_render_context {
        mem::t_arena *perm_arena;
        mem::t_arena *temp_arena;

        rendering::t_context *rendering_context;

        rand::t_rng *rng;
    };

    using t_game_init_func = void (*)(const t_game_init_context &context);
    using t_game_deinit_func = void (*)();
    using t_game_tick_func = void (*)(const t_game_tick_context &context);
    using t_game_render_func = void (*)(const t_game_render_context &context);

    void game_run(const t_game_init_func init_func, const t_game_tick_func tick_func, const t_game_render_func render_func, const t_game_deinit_func deinit_func = nullptr);
}
