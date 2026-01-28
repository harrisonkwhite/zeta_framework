#pragma once

#include <zcl.h>
#include <zgl/zgl_input.h>
#include <zgl/zgl_platform_public.h>
#include <zgl/zgl_gfx_public.h>
#include <zgl/zgl_rendering.h>
#include <zgl/zgl_audio_public.h>

namespace zgl {
    struct t_input_state; // Forward declaration from Input.

    struct t_game_init_func_context {
        zcl::t_arena *perm_arena;
        zcl::t_arena *temp_arena;

        t_platform_ticket_mut platform_ticket;

        t_gfx_ticket_mut gfx_ticket;

        t_audio_ticket_mut audio_ticket;

        zcl::t_rng *rng;

        void *user_mem;
    };

    struct t_game_deinit_func_context {
        zcl::t_arena *perm_arena;
        zcl::t_arena *temp_arena;

        t_platform_ticket_mut platform_ticket;

        t_gfx_ticket_mut gfx_ticket;

        t_audio_ticket_mut audio_ticket;

        zcl::t_rng *rng;

        void *user_mem;
    };

    struct t_game_tick_func_context {
        zcl::t_arena *perm_arena;
        zcl::t_arena *temp_arena;

        const t_input_state *input_state;

        t_platform_ticket_mut platform_ticket;

        t_gfx_ticket_mut gfx_ticket;

        t_audio_ticket_mut audio_ticket;

        zcl::t_rng *rng;

        zcl::t_f64 fps;

        void *user_mem;
    };

    struct t_game_render_func_context {
        zcl::t_arena *perm_arena;
        zcl::t_arena *temp_arena;

        const t_input_state *input_state;

        t_platform_ticket_rdonly platform_ticket;

        t_rendering_context rendering_context;

        zcl::t_rng *rng;

        zcl::t_f64 fps;

        void *user_mem;
    };

    struct t_game_window_focus_func_context {
        zcl::t_arena *perm_arena;
        zcl::t_arena *temp_arena;

        const t_input_state *input_state;

        t_platform_ticket_mut platform_ticket;

        t_gfx_ticket_mut gfx_ticket;

        t_audio_ticket_mut audio_ticket;

        zcl::t_rng *rng;

        void *user_mem;
    };

    struct t_game_backbuffer_resize_func_context {
        zcl::t_arena *perm_arena;
        zcl::t_arena *temp_arena;

        const t_input_state *input_state;

        t_platform_ticket_mut platform_ticket;

        t_gfx_ticket_mut gfx_ticket;

        t_audio_ticket_mut audio_ticket;

        zcl::t_rng *rng;

        void *user_mem;
    };

    using t_game_init_func = void (*)(const t_game_init_func_context &context);
    using t_game_deinit_func = void (*)(const t_game_deinit_func_context &context);
    using t_game_tick_func = void (*)(const t_game_tick_func_context &context);
    using t_game_render_func = void (*)(const t_game_render_func_context &context);
    using t_game_window_focus_func = void (*)(const t_game_window_focus_func_context &context);
    using t_game_backbuffer_resize_func = void (*)(const t_game_backbuffer_resize_func_context &context);

    struct t_game_config {
        t_game_init_func init_func;
        t_game_deinit_func deinit_func;
        t_game_tick_func tick_func;
        t_game_render_func render_func;
        t_game_window_focus_func window_focus_func;
        t_game_backbuffer_resize_func backbuffer_resize_func;

        zcl::t_i32 user_mem_size;
        zcl::t_i32 user_mem_alignment;

        zcl::t_f64 tps_target;

        zcl::t_i32 frame_vertex_limit;
    };

    inline t_game_config GameConfigCreate(const t_game_init_func init_func, const t_game_deinit_func deinit_func, const t_game_tick_func tick_func, const t_game_render_func render_func) {
        return {
            .init_func = init_func,
            .deinit_func = deinit_func,
            .tick_func = tick_func,
            .render_func = render_func,
            .window_focus_func = nullptr,
            .backbuffer_resize_func = nullptr,

            .user_mem_size = 0,
            .user_mem_alignment = 0,

            .tps_target = 60.0,

            .frame_vertex_limit = 8192,
        };
    }

    inline void GameConfigAssertValid(const t_game_config &config) {
        ZCL_ASSERT(config.init_func);
        ZCL_ASSERT(config.deinit_func);
        ZCL_ASSERT(config.tick_func);
        ZCL_ASSERT(config.render_func);

        ZCL_ASSERT((config.user_mem_size == 0 && config.user_mem_alignment == 0)
            || (config.user_mem_size > 0 && zcl::AlignmentCheckValid(config.user_mem_alignment)));

        ZCL_ASSERT(config.tps_target > 0.0);

        ZCL_ASSERT(config.frame_vertex_limit > 0);
    }

    void GameRun(const t_game_config &config);
}
