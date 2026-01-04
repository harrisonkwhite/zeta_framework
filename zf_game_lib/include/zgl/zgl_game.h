#pragma once

#include <zcl.h>
#include <zgl/zgl_input.h>
#include <zgl/zgl_gfx_core.h>

namespace zf {
    struct s_game_init_context {
        s_arena *perm_arena;
        s_arena *temp_arena;

        zf_rendering_resource_group *perm_gfx_resource_group;

        s_rng *rng;
    };

    struct s_game_tick_context {
        s_arena *perm_arena;
        s_arena *temp_arena;

        const zf_input_state *input_state;

        zf_rendering_resource_group *perm_gfx_resource_group;

        s_rng *rng;
    };

    struct s_game_render_context {
        s_arena *perm_arena;
        s_arena *temp_arena;

        s_rendering_context *rendering_context;

        s_rng *rng;
    };

    using t_game_init_func = void (*)(const s_game_init_context &context);
    using t_game_deinit_func = void (*)();
    using t_game_tick_func = void (*)(const s_game_tick_context &context);
    using t_game_render_func = void (*)(const s_game_render_context &context);

    void RunGame(const t_game_init_func init_func, const t_game_tick_func tick_func, const t_game_render_func render_func, const t_game_deinit_func deinit_func = nullptr);
}
