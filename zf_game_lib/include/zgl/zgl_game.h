#pragma once

#include <zcl.h>
#include <zgl/zgl_input.h>
#include <zgl/zgl_gfx.h>

namespace zgl::game {
    // ============================================================
    // @section: Types and Constants

    struct t_init_func_context {
        zcl::mem::t_arena *perm_arena;
        zcl::mem::t_arena *temp_arena;

        gfx::t_resource_group *perm_gfx_resource_group;

        zcl::rand::t_rng *rng;

        void *user_mem;
    };

    struct t_tick_func_context {
        zcl::mem::t_arena *perm_arena;
        zcl::mem::t_arena *temp_arena;

        const input::t_state *input_state;

        gfx::t_resource_group *perm_gfx_resource_group;

        zcl::rand::t_rng *rng;

        zcl::t_f64 fps;

        void *user_mem;
    };

    struct t_render_func_context {
        zcl::mem::t_arena *perm_arena;
        zcl::mem::t_arena *temp_arena;

        gfx::t_frame_context *frame_context;

        zcl::rand::t_rng *rng;

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

    // ============================================================


    // ============================================================
    // @section: Functions

    void run(const t_config &config);

    inline void config_assert_valid(const t_config &config) {
        ZF_ASSERT(config.init_func);
        ZF_ASSERT(config.deinit_func);
        ZF_ASSERT(config.tick_func);
        ZF_ASSERT(config.render_func);

        ZF_ASSERT((config.user_mem_size == 0 && config.user_mem_alignment == 0) || (config.user_mem_size > 0 && zcl::mem::alignment_check_valid(config.user_mem_alignment)));
    }

    void tps_set_target(const zcl::t_f64 tps);

    // ============================================================
}
