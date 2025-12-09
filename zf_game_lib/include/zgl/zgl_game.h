#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx.h>

namespace zf {
    struct s_platform_layer_info;
    struct s_audio_sys;

    struct s_game_init_context {
        void *dev_mem;

        s_mem_arena *mem_arena;
        s_mem_arena *temp_mem_arena;

        s_platform_layer_info *platform_layer_info;

        // s_gfx_resource_arena *gfx_res_arena;

        s_audio_sys *audio_sys;
    };

    struct s_input_state;

    struct s_game_tick_context {
        void *dev_mem;

        s_mem_arena *mem_arena;
        s_mem_arena *temp_mem_arena;

        s_input_state *input_state;

        s_platform_layer_info *platform_layer_info;

        // s_gfx_resource_arena *gfx_res_arena;

        s_audio_sys *audio_sys;
    };

    struct s_game_render_context {
        void *dev_mem;

        s_mem_arena *mem_arena;
        s_mem_arena *temp_mem_arena;

        // s_rendering_context rendering_context;
    };

    t_b8 RunGame();
}
