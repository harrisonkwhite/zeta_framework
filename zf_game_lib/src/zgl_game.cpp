#include <zgl/zgl_game.h>

#include <zgl/zgl_platform.h>
#include <zgl/zgl_input.h>
#include <zgl/zgl_gfx_core.h>
#include <zgl/zgl_audio.h>

namespace zf {
    constexpr s_v2_i g_init_window_size = {1280, 720};
    constexpr t_f64 g_targ_ticks_per_sec = 60.0; // @todo: Make this customisable?

    void RunGame(const t_game_init_func init_func, const t_game_tick_func tick_func, const t_game_render_func render_func, const t_game_deinit_func deinit_func) {
        ZF_ASSERT(init_func);
        ZF_ASSERT(tick_func);
        ZF_ASSERT(render_func);

        //
        // Initialisation
        //
        s_mem_arena perm_mem_arena = {};
        ZF_DEFER({ perm_mem_arena.Release(); });

        s_mem_arena temp_mem_arena = {};
        ZF_DEFER({ temp_mem_arena.Release(); });

        StartupPlatformModule(g_init_window_size);
        ZF_DEFER({ ShutdownPlatformModule(); });

        s_input_state input_state = {};

        s_rendering_basis *const rendering_basis = StartupGFXModule(&perm_mem_arena);
        ZF_DEFER({ ShutdownGFXModule(rendering_basis); });

        StartupAudioModule();
        ZF_DEFER({ ShutdownAudioModule(); });

        init_func({
            .perm_mem_arena = &perm_mem_arena,
            .temp_mem_arena = &temp_mem_arena,
        });

        ZF_DEFER({
            if (deinit_func) {
                deinit_func();
            }
        });

        //
        // Main Loop
        //
        ShowWindow();

        t_f64 frame_time_last = Time();
        t_f64 frame_dur_accum = 0.0;

        while (!ShouldWindowClose()) {
            temp_mem_arena.Rewind();

            PollOSEvents(&input_state);

            const t_f64 frame_time = Time();
            const t_f64 frame_time_delta = frame_time - frame_time_last;
            frame_dur_accum += frame_time_delta;
            frame_time_last = frame_time;

            const t_f64 targ_tick_interval = 1.0 / g_targ_ticks_per_sec;

            // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick and update the display.
            if (frame_dur_accum >= targ_tick_interval) {
                ProcFinishedSounds();

                do {
                    tick_func({
                        .perm_mem_arena = &perm_mem_arena,
                        .temp_mem_arena = &temp_mem_arena,
                        .input_state = &input_state,
                    });

                    input_state.events = {};

                    frame_dur_accum -= targ_tick_interval;
                } while (frame_dur_accum >= targ_tick_interval);

                s_ptr<s_rendering_context> rendering_context = BeginRendering(rendering_basis, s_color_rgb8(109, 187, 255), &temp_mem_arena); // @todo: Make the clear colour customisable?

                render_func({
                    .perm_mem_arena = &perm_mem_arena,
                    .temp_mem_arena = &temp_mem_arena,
                    .rendering_context = rendering_context,
                });

                EndRendering(rendering_context);
            }
        }
    }
}
