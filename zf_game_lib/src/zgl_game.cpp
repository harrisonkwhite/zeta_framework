#include <zgl/zgl_game.h>

#include <zgl/zgl_platform.h>

namespace zf::game {
    static const t_f64 g_init_target_tps = 60.0;
    static const math::t_v2_i g_init_window_size = {1280, 720};
    static const gfx::t_color_rgb24f g_init_clear_color = gfx::color_convert_to_rgb24f(gfx::color_create_rgb8(109, 187, 255));

    static struct {
        t_b8 running;
        t_f64 targ_tps;
        gfx::t_color_rgb24f clear_color;
    } g_module_state;

    void run(const t_init_func init_func, const t_tick_func tick_func, const t_render_func render_func, const t_deinit_func deinit_func) {
        ZF_REQUIRE(!g_module_state.running);

        g_module_state = {
            .running = true,
            .targ_tps = g_init_target_tps,
            .clear_color = g_init_clear_color,
        };

        ZF_DEFER({ g_module_state = {}; });

        //
        // Initialisation
        //
        mem::t_arena perm_arena = mem::arena_create();
        ZF_DEFER({ mem::arena_destroy(&perm_arena); });

        mem::t_arena temp_arena = mem::arena_create();
        ZF_DEFER({ mem::arena_destroy(&temp_arena); });

        platform::module_startup(g_init_window_size);
        ZF_DEFER({ platform::module_shutdown(); });

        input::t_state *const input_state = input::create_state(&perm_arena);

        rendering::t_resource_group *perm_rendering_resource_group;
        rendering::t_basis *const rendering_basis = rendering::module_startup(&perm_arena, &perm_rendering_resource_group);
        ZF_DEFER({ rendering::module_shutdown(rendering_basis); });

        rand::t_rng *const rng = rand::rng_create(0, &perm_arena); // @todo: Proper seed!

        init_func({
            .perm_arena = &perm_arena,
            .temp_arena = &temp_arena,
            .perm_rendering_resource_group = perm_rendering_resource_group,
            .rng = rng,
        });

        ZF_DEFER({
            if (deinit_func) {
                deinit_func();
            }
        });

        //
        // Main Loop
        //
        platform::window_show();

        t_f64 frame_time_last = platform::get_time();
        t_f64 frame_dur_accum = 0.0;

        while (!platform::window_should_close()) {
            platform::poll_os_events(input_state);

            const t_f64 frame_time = platform::get_time();
            const t_f64 frame_time_delta = frame_time - frame_time_last;
            frame_dur_accum += frame_time_delta;
            frame_time_last = frame_time;

            const t_f64 targ_tick_interval = 1.0 / g_module_state.targ_tps;

            // Once enough time has passed (i.e. the time accumulator has reached the target tick interval), run at least a single tick.
            while (frame_dur_accum >= targ_tick_interval) {
                mem::arena_rewind(&temp_arena);

                tick_func({
                    .perm_arena = &perm_arena,
                    .temp_arena = &temp_arena,
                    .input_state = input_state,
                    .perm_rendering_resource_group = perm_rendering_resource_group,
                    .rng = rng,
                });

                input::clear_events(input_state);

                frame_dur_accum -= targ_tick_interval;
            }

            mem::arena_rewind(&temp_arena);

            rendering::t_context *const rendering_context = rendering::frame_begin(rendering_basis, g_module_state.clear_color, &temp_arena);

            render_func({
                .perm_arena = &perm_arena,
                .temp_arena = &temp_arena,
                .rendering_context = rendering_context,
                .rng = rng,
            });

            rendering::frame_end(rendering_context);
        }
    }

    void set_target_tps(const t_f64 tps) {
        ZF_ASSERT(g_module_state.running);
        ZF_ASSERT(tps > 0.0);

        g_module_state.targ_tps = tps;
    }

    void set_clear_color(const gfx::t_color_rgb24f col) {
        ZF_ASSERT(g_module_state.running);
        ZF_ASSERT(gfx::color_is_valid(col));

        g_module_state.clear_color = col;
    }
}
