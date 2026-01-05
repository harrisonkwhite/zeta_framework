#include <zgl/zgl_game.h>

#include <zgl/zgl_platform.h>
#include <zgl/zgl_input.h>
#include <zgl/zgl_rendering.h>

namespace zf {
    static const t_v2_i g_init_window_size = {1280, 720};
    static const t_f64 g_targ_ticks_per_sec = 60.0; // @todo: Make this customisable?

    // @todo: Need a better and more consistent set of verbs. "Is" is too vague - how much calculation is being performed, should I cache the result?

    void game_run(const t_game_init_func init_func, const t_game_tick_func tick_func, const t_game_render_func render_func, const t_game_deinit_func deinit_func) {
        ZF_ASSERT(init_func);
        ZF_ASSERT(tick_func);
        ZF_ASSERT(render_func);

        //
        // Initialisation
        //
        t_arena perm_arena = f_mem_arena_create();
        ZF_DEFER({ f_mem_arena_destroy(&perm_arena); });

        t_arena temp_arena = f_mem_arena_create();
        ZF_DEFER({ f_mem_arena_destroy(&temp_arena); });

        platform::f_startup(g_init_window_size);
        ZF_DEFER({ platform::f_shutdown(); });

        input::t_state *const input_state = input::f_create_state(&perm_arena);

        t_rendering_resource_group *perm_rendering_resource_group;
        t_rendering_basis *const rendering_basis = f_rendering_startup_module(&perm_arena, &perm_rendering_resource_group);
        ZF_DEFER({ f_rendering_shutdown_module(rendering_basis); });

        t_rng *const rng = f_rand_create_rng(0, &perm_arena); // @todo: Proper seed!

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
        platform::f_show_window();

        t_f64 frame_time_last = platform::f_get_time();
        t_f64 frame_dur_accum = 0.0;

        while (!platform::f_should_window_close()) {
            platform::f_poll_os_events(input_state);

            const t_f64 frame_time = platform::f_get_time();
            const t_f64 frame_time_delta = frame_time - frame_time_last;
            frame_dur_accum += frame_time_delta;
            frame_time_last = frame_time;

            const t_f64 targ_tick_interval = 1.0 / g_targ_ticks_per_sec;

            // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick.
            while (frame_dur_accum >= targ_tick_interval) {
                f_mem_arena_rewind(&temp_arena);

                tick_func({
                    .perm_arena = &perm_arena,
                    .temp_arena = &temp_arena,
                    .input_state = input_state,
                    .perm_rendering_resource_group = perm_rendering_resource_group,
                    .rng = rng,
                });

                input::f_clear_events(input_state);

                frame_dur_accum -= targ_tick_interval;
            }

            f_mem_arena_rewind(&temp_arena);

            t_rendering_context *const rendering_context = f_rendering_begin_frame(rendering_basis, {109, 187, 255}, &temp_arena); // @todo: Make the clear colour customisable?

            render_func({
                .perm_arena = &perm_arena,
                .temp_arena = &temp_arena,
                .rendering_context = rendering_context,
                .rng = rng,
            });

            f_rendering_end_frame(rendering_context);
        }
    }
}
