#include <zgl/zgl_game.h>

#include <zgl/zgl_audio.h>
#include <zgl/zgl_gfx.h>
#include <zgl/zgl_platform.h>

namespace zf {
    constexpr s_v2_i g_init_window_size = {1280, 720};
    constexpr t_f64 g_targ_ticks_per_sec = 60.0; // @todo: Make this customisable?

    t_b8 RunGame(const t_game_init_func init_func, const t_game_tick_func tick_func, const t_game_render_func render_func, const t_game_cleanup_func cleanup_func) {
        ZF_ASSERT(init_func);
        ZF_ASSERT(tick_func);
        ZF_ASSERT(render_func);

#ifndef ZF_DEBUG
        // Redirect stderr to crash log file.
        freopen("error.log", "w", stderr);
#endif

        const t_b8 success = [init_func, tick_func, render_func, cleanup_func]() {
            //
            // Initialisation
            //
            s_mem_arena mem_arena = {};

            if (!mem_arena.Init(Megabytes(80))) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ mem_arena.Release(); });

            s_mem_arena temp_mem_arena = {};

            if (!temp_mem_arena.InitAsChild(Megabytes(10), mem_arena)) {
                ZF_REPORT_ERROR();
                return false;
            }

            if (!platform::internal::Init(g_init_window_size)) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ platform::internal::Shutdown(); });

            if (!gfx::Init()) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ gfx::Shutdown(); });

            s_ptr<s_audio_sys> audio_sys = nullptr;

            if (!CreateAudioSys(mem_arena, audio_sys)) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ DestroyAudioSys(*audio_sys); });

            // Run the developer's initialisation function.
            {
                const s_game_init_context context = {
                    .mem_arena = mem_arena,
                    .temp_mem_arena = temp_mem_arena,
                    .audio_sys = *audio_sys,
                };

                if (!init_func(context)) {
                    ZF_REPORT_ERROR();
                    return false;
                }
            }

            ZF_DEFER({
                if (cleanup_func) {
                    cleanup_func();
                }
            });

            //
            // Main Loop
            //
            platform::internal::ShowWindow();

            t_f64 frame_time_last = platform::Time();
            t_f64 frame_dur_accum = 0.0;

            while (!platform::internal::ShouldWindowClose()) {
                temp_mem_arena.Rewind(0);

                platform::internal::PollOSEvents();

                const t_f64 frame_time = platform::Time();
                const t_f64 frame_time_delta = frame_time - frame_time_last;
                frame_dur_accum += frame_time_delta;
                frame_time_last = frame_time;

                const t_f64 targ_tick_interval = 1.0 / g_targ_ticks_per_sec;

                // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick and update the display.
                if (frame_dur_accum >= targ_tick_interval) {
                    // Run possibly multiple ticks.
                    do {
                        ProcFinishedSounds(*audio_sys);

                        const s_game_tick_context context = {
                            .mem_arena = mem_arena,
                            .temp_mem_arena = temp_mem_arena,
                            .audio_sys = *audio_sys,
                        };

                        if (!tick_func(context)) {
                            ZF_REPORT_ERROR();
                            return false;
                        }

                        platform::internal::ClearInputEvents();

                        frame_dur_accum -= targ_tick_interval;
                    } while (frame_dur_accum >= targ_tick_interval);

                    // Perform a single render.
#if 0
                    internal::BeginFrame(WindowFramebufferSizeCache(*platform_layer_info));

                    internal::EndFrame();
#endif
#if 0
                    s_rendering_context rendering_context = {};

                    if (!internal::BeginFrame(*rendering_basis, WindowFramebufferSizeCache(*platform_layer_info), temp_mem_arena, rendering_context)) {
                        ZF_REPORT_ERROR();
                        return false;
                    }

                    {
                        const s_game_render_context context = {
                            .mem_arena = mem_arena,
                            .temp_mem_arena = temp_mem_arena,
                            .rendering_context = rendering_context,
                        };

                        if (!render_func(context)) {
                            ZF_REPORT_ERROR();
                            return false;
                        }
                    }

                    internal::CompleteFrame(rendering_context);
#endif
                }
            }

            return true;
        }();

#ifndef ZF_DEBUG
        if (!success) {
            ShowErrorBox("Error", "A fatal error occurred! Please check \"error.log\" for details.");
        }
#endif

        return success;
    }
}
