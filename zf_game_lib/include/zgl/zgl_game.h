#pragma once

#include <zcl.h>
#include <zgl/zgl_input.h>
#include <zgl/zgl_rendering.h>

namespace zf::game {
    struct t_init_func_context {
        mem::t_arena *perm_arena;
        mem::t_arena *temp_arena;

        rendering::t_resource_group *perm_rendering_resource_group;

        rand::t_rng *rng;

        void *user_mem;
    };

    struct t_tick_func_context {
        mem::t_arena *perm_arena;
        mem::t_arena *temp_arena;

        const input::t_state *input_state;

        rendering::t_resource_group *perm_rendering_resource_group;

        rand::t_rng *rng;

        void *user_mem;
    };

    struct t_render_func_context {
        mem::t_arena *perm_arena;
        mem::t_arena *temp_arena;

        rendering::t_frame_context *frame_context;

        rand::t_rng *rng;

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

        t_i32 user_mem_size;
        t_i32 user_mem_alignment;
    };

    inline void config_assert_valid(const t_config &config) {
        ZF_ASSERT(config.init_func);
        ZF_ASSERT(config.deinit_func);
        ZF_ASSERT(config.tick_func);
        ZF_ASSERT(config.render_func);

        ZF_ASSERT((config.user_mem_size == 0 && config.user_mem_alignment == 0) || (config.user_mem_size > 0 && mem::alignment_check_valid(config.user_mem_alignment)));
    }

    void run(const t_config &config);

    void set_target_tps(const t_f64 tps);
    void set_clear_color(const gfx::t_color_rgba32f col);
}
