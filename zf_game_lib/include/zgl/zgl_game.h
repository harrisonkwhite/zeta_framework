#pragma once

#include <zcl.h>

namespace zf {
    struct s_gfx_resource_arena;

    struct s_game_init_context {
        s_mem_arena &perm_mem_arena;
        s_mem_arena &temp_mem_arena;
    };

    struct s_game_tick_context {
        s_mem_arena &perm_mem_arena;
        s_mem_arena &temp_mem_arena;
    };

    struct s_batch_renderer;

    struct s_game_render_context {
        s_mem_arena &perm_mem_arena;
        s_mem_arena &temp_mem_arena;

        s_batch_renderer &batch_renderer;
    };

    using t_game_init_func = void (*)(const s_game_init_context &context);
    using t_game_tick_func = void (*)(const s_game_tick_context &context);
    using t_game_render_func = void (*)(const s_game_render_context &context);
    using t_game_cleanup_func = void (*)();

    void RunGame(const t_game_init_func init_func, const t_game_tick_func tick_func, const t_game_render_func render_func, const t_game_cleanup_func cleanup_func = nullptr);
}
