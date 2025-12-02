#include <zf/zf_game.h>

#include <zf/zf_renderer.h>
#include <zf/zf_audio_sys.h>

namespace zf {
    t_b8 RunGame(const s_game_info& info) {
        AssertGameInfoValidity(info);

        const t_b8 success = [&info]() {
            //
            // Initialisation
            //
#ifndef ZF_DEBUG
            // Redirect stderr to crash log file.
            freopen("error.log", "w", stderr);
#endif

            // Set up memory arenas.
            s_mem_arena mem_arena;

            if (!AllocMemArena(info.mem_arena_size, mem_arena)) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ FreeMemArena(mem_arena); });

            s_mem_arena temp_mem_arena;

            if (!MakeSubMemArena(mem_arena, info.temp_mem_arena_size, temp_mem_arena)) {
                ZF_REPORT_ERROR();
                return false;
            }

            // Initialise the window.
            if (!InitWindow(info.window_init_size, info.window_init_title, info.window_init_flags, temp_mem_arena)) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ ShutdownWindow(); });

            // Initialise the renderer.
            if (!renderer::Init(mem_arena, temp_mem_arena)) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ renderer::Shutdown(); });

            // Initialise audio system.
            if (!audio_sys::Init()) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ audio_sys::Shutdown(); });

            // Initialise developer memory.
            void* dev_mem = nullptr;

            if (info.dev_mem_size > 0) {
                dev_mem = PushToMemArena(mem_arena, info.dev_mem_size, info.dev_mem_alignment);

                if (!dev_mem) {
                    ZF_REPORT_ERROR();
                    return false;
                }
            }

            // Run the developer's initialisation function.
            {
                const s_game_init_context context = {
                    .dev_mem = dev_mem,
                    .mem_arena = &mem_arena,
                    .temp_mem_arena = &temp_mem_arena
                };

                if (!info.init_func(context)) {
                    ZF_REPORT_ERROR();
                    return false;
                }
            }

            ZF_DEFER({
                if (info.clean_func) {
                    info.clean_func(dev_mem);
                }
            });

            // Now that everything is set up, we can show the window.
            ShowWindow();

            //
            // Main Loop
            //
            t_f64 frame_time_last = Time();
            t_f64 frame_dur_accum = 0.0;

            while (!ShouldWindowClose()) {
                RewindMemArena(temp_mem_arena, 0);

                const t_f64 frame_time = Time();
                const t_f64 frame_time_delta = frame_time - frame_time_last;
                frame_dur_accum += frame_time_delta;
                frame_time_last = frame_time;

                const t_f64 targ_tick_interval = 1.0 / info.targ_ticks_per_sec;

                // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick and update the display.
                if (frame_dur_accum >= targ_tick_interval) {
                    audio_sys::ProcFinishedSounds();

                    // Run possibly multiple ticks.
                    do {
                        const s_game_tick_context context = {
                            .dev_mem = dev_mem,
                            .mem_arena = &mem_arena,
                            .temp_mem_arena = &temp_mem_arena
                        };

                        const e_game_tick_result res = info.tick_func(context);

                        ClearInputEvents();

                        if (res == ek_game_tick_result_exit) {
                            Log("Exit request detected from developer game tick function...");
                            SetWindowShouldClose();
                        }

                        if (res == ek_game_tick_result_error) {
                            ZF_REPORT_ERROR();
                            return false;
                        }

                        frame_dur_accum -= targ_tick_interval;
                    } while (frame_dur_accum >= targ_tick_interval);

                    // Perform a single render.
                    renderer::BeginRenderingPhase();

                    {
                        const s_game_render_context context = {
                            .dev_mem = dev_mem,
                            .mem_arena = &mem_arena,
                            .temp_mem_arena = &temp_mem_arena
                        };

                        if (!info.render_func(context)) {
                            ZF_REPORT_ERROR();
                            renderer::EndRenderingPhase();
                            return false;
                        }
                    }

                    renderer::EndRenderingPhase();

                    SwapWindowBuffers();
                }

                PollOSEvents();
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
