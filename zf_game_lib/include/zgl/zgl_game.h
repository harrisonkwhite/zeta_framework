#pragma once

#include <zcl.h>

namespace zgl {
    struct t_input_state;
    struct t_platform;
    struct t_gfx;
    struct t_frame_context;
    struct t_audio_sys;

    namespace game {
        struct t_init_func_context {
            zcl::t_arena *perm_arena;
            zcl::t_arena *temp_arena;

            t_platform *platform;

            t_gfx *gfx;

            zgl::t_audio_sys *audio_sys;

            zcl::t_rng *rng;

            void *user_mem;
        };

        struct t_tick_func_context {
            zcl::t_arena *perm_arena;
            zcl::t_arena *temp_arena;

            const t_input_state *input_state;

            t_platform *platform;

            t_gfx *gfx;

            zgl::t_audio_sys *audio_sys;

            zcl::t_rng *rng;

            zcl::t_f64 fps;

            void *user_mem;
        };

        struct t_render_func_context {
            zcl::t_arena *perm_arena;
            zcl::t_arena *temp_arena;

            t_frame_context *frame_context;

            zcl::t_rng *rng;

            zcl::t_f64 fps;

            void *user_mem;
        };

        using t_init_func = void (*)(const t_init_func_context &context);
        using t_deinit_func = void (*)(void *const user_mem);
        using t_tick_func = void (*)(const t_tick_func_context &context);
        using t_render_func = void (*)(const t_render_func_context &context);

        struct t_config {
            t_init_func init_func;
            t_deinit_func deinit_func;
            t_tick_func tick_func;
            t_render_func render_func;

            zcl::t_i32 user_mem_size;
            zcl::t_i32 user_mem_alignment;
        };

        inline void ConfigAssertValid(const t_config &config) {
            ZCL_ASSERT(config.init_func);
            ZCL_ASSERT(config.deinit_func);
            ZCL_ASSERT(config.tick_func);
            ZCL_ASSERT(config.render_func);

            ZCL_ASSERT((config.user_mem_size == 0 && config.user_mem_alignment == 0) || (config.user_mem_size > 0 && zcl::AlignmentCheckValid(config.user_mem_alignment)));
        }

        void Run(const t_config &config);

        void SetTargetTPS(const zcl::t_f64 tps);
    }
}
