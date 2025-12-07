#include <zgl/zgl_audio.h>
#include <zgl/zgl_game.h>
#include <zgl/zgl_gfx.h>
#include <zgl/zgl_input.h>
#include <zgl/zgl_platform.h>

namespace zf
{
    t_b8 RunGame(const s_game_info& info)
    {
        ZF_ASSERT(info.mem_arena_size > 0);
        ZF_ASSERT(info.temp_mem_arena_size > 0 && info.temp_mem_arena_size <= info.mem_arena_size);

        ZF_ASSERT((info.dev_mem_size == 0 && info.dev_mem_alignment == 0) ||
                  (info.dev_mem_size > 0 && IsAlignmentValid(info.dev_mem_alignment)));

        ZF_ASSERT(info.targ_ticks_per_sec > 0);

        ZF_ASSERT(info.init_func);
        ZF_ASSERT(info.tick_func);
        ZF_ASSERT(info.render_func);

#ifndef ZF_DEBUG
        // Redirect stderr to crash log file.
        freopen("error.log", "w", stderr);
#endif

        const t_b8 success = [&info]()
        {
            //
            // Initialisation
            //
            s_mem_arena mem_arena;

            if (!InitMemArena(&mem_arena, info.mem_arena_size))
            {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ ReleaseMemArena(&mem_arena); });

            s_mem_arena temp_mem_arena;

            if (!InitChildMemArena(&temp_mem_arena, info.temp_mem_arena_size, &mem_arena))
            {
                ZF_REPORT_ERROR();
                return false;
            }

            s_input_state input_state = {};

            const auto platform_layer_info = P_InitPlatformLayer(&mem_arena, &input_state);

            if (!platform_layer_info)
            {
                ZF_REPORT_ERROR();
                return false;
            }

            s_rendering_basis* rendering_basis;

            if (!InitGFX(&rendering_basis, &mem_arena, &temp_mem_arena))
            {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ ShutdownGFX(rendering_basis); });

            s_audio_sys* audio_sys;

            if (!P_InitAudioSys(mem_arena, audio_sys))
            {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ P_ShutdownAudioSys(*audio_sys); });

            // Initialise developer memory.
            void* dev_mem = nullptr;

            if (info.dev_mem_size > 0)
            {
                dev_mem = PushToMemArena(&mem_arena, info.dev_mem_size, info.dev_mem_alignment);

                if (!dev_mem)
                {
                    ZF_REPORT_ERROR();
                    return false;
                }
            }

            // Run the developer's initialisation function.
            {
                const s_game_init_context context = {.dev_mem = dev_mem,
                    .mem_arena = &mem_arena,
                    .temp_mem_arena = &temp_mem_arena,
                    .platform_layer_info = platform_layer_info,
                    .audio_sys = audio_sys};

                if (!info.init_func(context))
                {
                    ZF_REPORT_ERROR();
                    return false;
                }
            }

            ZF_DEFER({
                if (info.clean_func)
                {
                    info.clean_func(dev_mem);
                }
            });

            //
            // Main Loop
            //
            P_ShowWindow(platform_layer_info);

            t_f64 frame_time_last = Time();
            t_f64 frame_dur_accum = 0.0;

            while (!P_ShouldWindowClose(platform_layer_info))
            {
                RewindMemArena(&temp_mem_arena, 0);

                P_PollOSEvents();

                const t_f64 frame_time = Time();
                const t_f64 frame_time_delta = frame_time - frame_time_last;
                frame_dur_accum += frame_time_delta;
                frame_time_last = frame_time;

                const t_f64 targ_tick_interval = 1.0 / info.targ_ticks_per_sec;

                // Once enough time has passed (i.e. the time accumulator has reached the tick
                // interval), run at least a single tick and update the display.
                if (frame_dur_accum >= targ_tick_interval)
                {
                    P_ProcFinishedSounds(*audio_sys);

                    // Run possibly multiple ticks.
                    do
                    {
                        const s_game_tick_context context = {.dev_mem = dev_mem,
                            .mem_arena = &mem_arena,
                            .temp_mem_arena = &temp_mem_arena,
                            .input_state = &input_state,
                            .platform_layer_info = platform_layer_info,
                            .audio_sys = audio_sys};

                        if (!info.tick_func(context))
                        {
                            ZF_REPORT_ERROR();
                            return false;
                        }

                        ZeroOut(&input_state.events);

                        frame_dur_accum -= targ_tick_interval;
                    } while (frame_dur_accum >= targ_tick_interval);

                    // Perform a single render.
                    s_rendering_context rendering_context;

                    if (!BeginFrame(&rendering_context, rendering_basis,
                            WindowFramebufferSizeCache(platform_layer_info), &mem_arena))
                    {
                        ZF_REPORT_ERROR();
                        return false;
                    }

                    {
                        const s_game_render_context context = {.dev_mem = dev_mem,
                            .mem_arena = &mem_arena,
                            .temp_mem_arena = &temp_mem_arena,
                            .rendering_context = rendering_context};

                        if (!info.render_func(context))
                        {
                            ZF_REPORT_ERROR();
                            return false;
                        }
                    }

                    CompleteFrame(rendering_context);

                    P_SwapWindowBuffers(platform_layer_info);
                }
            }

            return true;
        }();

#ifndef ZF_DEBUG
        if (!success)
        {
            ShowErrorBox(
                "Error", "A fatal error occurred! Please check \"error.log\" for details.");
        }
#endif

        return success;
    }
}
