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
    };

    struct t_tick_func_context {
        mem::t_arena *perm_arena;
        mem::t_arena *temp_arena;

        const input::t_state *input_state;

        rendering::t_resource_group *perm_rendering_resource_group;

        rand::t_rng *rng;
    };

    struct t_render_func_context {
        mem::t_arena *perm_arena;
        mem::t_arena *temp_arena;

        rendering::t_context *rendering_context;

        rand::t_rng *rng;
    };

    using t_init_func = void (*)(const t_init_func_context &context);
    using t_deinit_func = void (*)();
    using t_tick_func = void (*)(const t_tick_func_context &context);
    using t_render_func = void (*)(const t_render_func_context &context);

    void run(const t_init_func init_func, const t_tick_func tick_func, const t_render_func render_func, const t_deinit_func deinit_func = nullptr);

    void set_target_tps(const t_f64 tps);
    void set_clear_color(const gfx::t_color_rgb24f col);
}
