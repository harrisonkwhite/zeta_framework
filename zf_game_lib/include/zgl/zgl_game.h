#pragma once

#include <zcl.h>
#include <zgl/zgl_platform_public.h>
#include <zgl/zgl_gfx.h>

namespace zgl {
    struct t_input_state; // Forward declaration.
    struct t_audio_sys;   // Forward declaration.

    struct t_game_init_func_context {
        zcl::t_arena *perm_arena;
        zcl::t_arena *temp_arena;

        t_platform_ticket_mut platform_ticket;

        t_gfx *gfx;

        t_audio_sys *audio_sys;

        zcl::t_rng *rng;

        void *user_mem;
    };

    struct t_game_deinit_func_context {
        zcl::t_arena *perm_arena;
        zcl::t_arena *temp_arena;

        t_platform_ticket_mut platform_ticket;

        t_gfx *gfx;

        t_audio_sys *audio_sys;

        zcl::t_rng *rng;

        void *user_mem;
    };

    struct t_game_tick_func_context {
        zcl::t_arena *perm_arena;
        zcl::t_arena *temp_arena;

        const t_input_state *input_state;

        t_platform_ticket_mut platform_ticket;

        t_gfx *gfx;

        t_audio_sys *audio_sys;

        zcl::t_rng *rng;

        zcl::t_f64 fps;

        void *user_mem;
    };

    struct t_game_render_func_context {
        zcl::t_arena *perm_arena;
        zcl::t_arena *temp_arena;

        t_platform_ticket_rdonly platform_ticket;

        t_frame_context frame_context;

        zcl::t_rng *rng;

        zcl::t_f64 fps;

        void *user_mem;
    };

    using t_game_init_func = void (*)(const t_game_init_func_context &context);
    using t_game_deinit_func = void (*)(const t_game_deinit_func_context &context);
    using t_game_tick_func = void (*)(const t_game_tick_func_context &context);
    using t_game_render_func = void (*)(const t_game_render_func_context &context);

    // @todo: Would be useful to have a framebuffer resize callback, since texture target resizes are done in ticks but ticks don't run before every frame. You might have frames before the next tick where the thing hasn't resized yet.
    // @todo: Other callbacks could be useful too - maybe window focus?

    struct t_game_config {
        t_game_init_func init_func;
        t_game_deinit_func deinit_func;
        t_game_tick_func tick_func;
        t_game_render_func render_func;

        zcl::t_i32 user_mem_size;
        zcl::t_i32 user_mem_alignment;

        zcl::t_f64 tps_target;
    };

    inline void GameConfigAssertValid(const t_game_config &config) {
        ZCL_ASSERT(config.init_func);
        ZCL_ASSERT(config.deinit_func);
        ZCL_ASSERT(config.tick_func);
        ZCL_ASSERT(config.render_func);

        ZCL_ASSERT((config.user_mem_size == 0 && config.user_mem_alignment == 0)
            || (config.user_mem_size > 0 && zcl::AlignmentCheckValid(config.user_mem_alignment)));

        ZCL_ASSERT(config.tps_target > 0.0);
    }

    void GameRun(const t_game_config &config);
}
