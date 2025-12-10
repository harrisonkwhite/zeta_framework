#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx.h>

namespace zf {
    struct s_platform_layer_info;
    struct s_audio_sys;

    struct s_game_init_context {
        s_mem_arena *mem_arena = nullptr;
        s_mem_arena *temp_mem_arena = nullptr;

        s_platform_layer_info *platform_layer_info = nullptr;

        // s_gfx_resource_arena *gfx_res_arena = nullptr;

        s_audio_sys *audio_sys = nullptr;
    };

    struct s_input_state;

    struct s_game_tick_context {
        s_mem_arena *mem_arena = nullptr;
        s_mem_arena *temp_mem_arena = nullptr;

        s_input_state *input_state = nullptr;

        s_platform_layer_info *platform_layer_info = nullptr;

        // s_gfx_resource_arena *gfx_res_arena = nullptr;

        s_audio_sys *audio_sys = nullptr;
    };

    struct s_game_render_context {
        s_mem_arena *mem_arena = nullptr;
        s_mem_arena *temp_mem_arena = nullptr;

        // s_rendering_context rendering_context = {};
    };

    using t_game_init_func = t_b8 (*)(const s_game_init_context &context);
    using t_game_tick_func = t_b8 (*)(const s_game_tick_context &context);
    using t_game_render_func = t_b8 (*)(const s_game_render_context &context);
    using t_game_cleanup_func = void (*)();

    t_b8 RunGame(const t_game_init_func init_func, const t_game_tick_func tick_func, const t_game_render_func render_func, const t_game_cleanup_func cleanup_func = nullptr);
}
