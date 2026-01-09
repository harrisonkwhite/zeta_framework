#pragma once

#include <zcl.h>
#include <zgl/zgl_input.h>
#include <zgl/zgl_gfx.h>

namespace zgl::game {
    struct t_init_func_context {
        zf::mem::t_arena *perm_arena;
        zf::mem::t_arena *temp_arena;

        gfx::t_resource_group *perm_gfx_resource_group;

        zf::rand::t_rng *rng;

        void *user_mem;
    };

    struct t_tick_func_context {
        zf::mem::t_arena *perm_arena;
        zf::mem::t_arena *temp_arena;

        const input::t_state *input_state;

        gfx::t_resource_group *perm_gfx_resource_group;

        zf::rand::t_rng *rng;

        void *user_mem;
    };

    struct t_render_func_context {
        zf::mem::t_arena *perm_arena;
        zf::mem::t_arena *temp_arena;

        gfx::t_frame_context *frame_context;

        zf::rand::t_rng *rng;

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

        zf::t_i32 user_mem_size;
        zf::t_i32 user_mem_alignment;
    };

    inline void config_assert_valid(const t_config &config) {
        ZF_ASSERT(config.init_func);
        ZF_ASSERT(config.deinit_func);
        ZF_ASSERT(config.tick_func);
        ZF_ASSERT(config.render_func);

        ZF_ASSERT((config.user_mem_size == 0 && config.user_mem_alignment == 0) || (config.user_mem_size > 0 && zf::mem::alignment_check_valid(config.user_mem_alignment)));
    }

    void run(const t_config &config);

    void set_target_tps(const zf::t_f64 tps);
}
