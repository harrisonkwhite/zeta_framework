#include <zgl/zgl_game.h>

#include <zgl/zgl_audio.h>
#include <zgl/zgl_gfx.h>
#include <zgl/zgl_input.h>
#include <zgl/zgl_platform.h>

namespace zf {
    constexpr t_f64 g_targ_ticks_per_sec = 60.0; // @todo: Make this customisable?

    t_b8 RunGame(const s_game_info &info) {
        ZF_ASSERT((info.dev_mem_size == 0 && info.dev_mem_alignment == 0) || (info.dev_mem_size > 0 && IsAlignmentValid(info.dev_mem_alignment)));
        ZF_ASSERT(info.init_func);
        ZF_ASSERT(info.tick_func);
        ZF_ASSERT(info.render_func);

#ifndef ZF_DEBUG
        // Redirect stderr to crash log file.
        freopen("error.log", "w", stderr);
#endif

        const t_b8 success = [&info]() {
            //
            // Initialisation
            //
            s_mem_arena mem_arena;

            if (!mem_arena.Init(Megabytes(80))) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ mem_arena.Release(); });

            s_mem_arena temp_mem_arena;

            if (!temp_mem_arena.InitAsChild(Megabytes(10), &mem_arena)) {
                ZF_REPORT_ERROR();
                return false;
            }

            s_input_state input_state = {};

            s_platform_layer_info *platform_layer_info;

            if (!internal::InitPlatformLayer(&mem_arena, &input_state, &platform_layer_info)) {
                ZF_REPORT_ERROR();
                return false;
            }

            s_audio_sys *audio_sys;

            if (!internal::CreateAudioSys(&mem_arena, &audio_sys)) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ internal::DestroyAudioSys(audio_sys); });

            // Initialise developer memory.
            void *dev_mem = nullptr;

            if (info.dev_mem_size > 0) {
                dev_mem = mem_arena.PushRaw(info.dev_mem_size, info.dev_mem_alignment);

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
                    .temp_mem_arena = &temp_mem_arena,
                    .platform_layer_info = platform_layer_info,
                    .audio_sys = audio_sys,
                };

                if (!info.init_func(context)) {
                    ZF_REPORT_ERROR();
                    return false;
                }
            }

            ZF_DEFER({
                if (info.cleanup_func) {
                    info.cleanup_func(dev_mem);
                }
            });

            //
            // Main Loop
            //
            internal::ShowWindow(platform_layer_info);

            t_f64 frame_time_last = Time();
            t_f64 frame_dur_accum = 0.0;

            while (!internal::ShouldWindowClose(platform_layer_info)) {
                temp_mem_arena.Clear();

                internal::PollOSEvents();

                const t_f64 frame_time = Time();
                const t_f64 frame_time_delta = frame_time - frame_time_last;
                frame_dur_accum += frame_time_delta;
                frame_time_last = frame_time;

                const t_f64 targ_tick_interval = 1.0 / g_targ_ticks_per_sec;

                // Once enough time has passed (i.e. the time accumulator has reached the tick
                // interval), run at least a single tick and update the display.
                if (frame_dur_accum >= targ_tick_interval) {
                    // Run possibly multiple ticks.
                    do {
                        internal::ProcFinishedSounds(audio_sys);

                        const s_game_tick_context context = {
                            .dev_mem = dev_mem,
                            .mem_arena = &mem_arena,
                            .temp_mem_arena = &temp_mem_arena,
                            .input_state = &input_state,
                            .platform_layer_info = platform_layer_info,
                            .audio_sys = audio_sys,
                        };

                        if (!info.tick_func(context)) {
                            ZF_REPORT_ERROR();
                            return false;
                        }

                        input_state.events = {};

                        frame_dur_accum -= targ_tick_interval;
                    } while (frame_dur_accum >= targ_tick_interval);

#if 0
                    // Perform a single render.
                    s_rendering_context rendering_context;

                    if (!internal::BeginFrame(&rendering_context, rendering_basis, WindowFramebufferSizeCache(platform_layer_info), &mem_arena)) {
                        ZF_REPORT_ERROR();
                        return false;
                    }

                    {
                        const s_game_render_context context = {
                            .dev_mem = dev_mem,
                            .mem_arena = &mem_arena,
                            .temp_mem_arena = &temp_mem_arena,
                            .rendering_context = rendering_context,
                        };

                        if (!info.render_func(context)) {
                            ZF_REPORT_ERROR();
                            return false;
                        }
                    }

                    internal::CompleteFrame(rendering_context);
#endif

                    internal::SwapWindowBuffers(platform_layer_info);
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
