#include <zgl/zgl_game.h>

#include <zgl/zgl_platform.h>
#include <zgl/zgl_input.h>
#include <zgl/zgl_gfx_core.h>
#include <zgl/zgl_audio.h>

namespace zf {
    constexpr s_v2_i g_init_window_size = {1280, 720};
    constexpr t_f64 g_targ_ticks_per_sec = 60.0; // @todo: Make this customisable?

    void game_run(const t_game_init_func init_func, const t_game_tick_func tick_func, const t_game_render_func render_func, const t_game_deinit_func deinit_func) {
        ZF_ASSERT(init_func);
        ZF_ASSERT(tick_func);
        ZF_ASSERT(render_func);

        //
        // Initialisation
        //
        s_arena perm_arena = arena_create();
        ZF_DEFER({ arena_destroy(&perm_arena); });

        s_arena temp_arena = arena_create();
        ZF_DEFER({ arena_destroy(&temp_arena); });

        platform::startup(g_init_window_size);
        ZF_DEFER({ platform::shutdown(); });

        s_input_state input_state = {};

        gfx::s_rendering_basis *const rendering_basis = gfx::startup(&perm_arena);
        ZF_DEFER({ gfx::shutdown(rendering_basis); });

        s_rng *const rng = rng_create(0, &perm_arena); // @todo: Proper seed!

        init_func({
            .perm_arena = &perm_arena,
            .temp_arena = &temp_arena,
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

        while (!platform::window_get_should_close()) {
            platform::poll_os_events(&input_state);

            const t_f64 frame_time = platform::get_time();
            const t_f64 frame_time_delta = frame_time - frame_time_last;
            frame_dur_accum += frame_time_delta;
            frame_time_last = frame_time;

            const t_f64 targ_tick_interval = 1.0 / g_targ_ticks_per_sec;

            // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick.
            while (frame_dur_accum >= targ_tick_interval) {
                arena_rewind(&temp_arena);

                tick_func({
                    .perm_arena = &perm_arena,
                    .temp_arena = &temp_arena,
                    .input_state = &input_state,
                    .rng = rng,
                });

                input_state.events = {};

                frame_dur_accum -= targ_tick_interval;
            }

            arena_rewind(&temp_arena);

            gfx::s_rendering_context *const rendering_context = gfx::rendering_begin(rendering_basis, s_color_rgb8(109, 187, 255), &temp_arena); // @todo: Make the clear colour customisable?

            render_func({
                .perm_arena = &perm_arena,
                .temp_arena = &temp_arena,
                .rendering_context = rendering_context,
                .rng = rng,
            });

            gfx::rendering_end(rendering_context);
        }
    }
}
