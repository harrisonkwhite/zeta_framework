#include <zgl/zgl_game.h>

namespace zgl {
    constexpr zcl::t_v2_i k_window_size_init = {1280, 720};

    // @todo: Maybe switch to F32 for consistency.

    void GameRun(const t_game_config &config) {
        GameConfigAssertValid(config);

        //
        // Initialization
        //
        zcl::t_arena perm_arena = zcl::ArenaCreateBlockBased();
        ZCL_DEFER({ zcl::ArenaDestroy(&perm_arena); });

        zcl::t_arena temp_arena = zcl::ArenaCreateBlockBased();
        ZCL_DEFER({ zcl::ArenaDestroy(&temp_arena); });

        t_input_state *const input_state = internal::InputCreateState(&perm_arena);

        const t_platform_ticket_mut platform_ticket = internal::PlatformStartup(k_window_size_init, input_state, &perm_arena);
        ZCL_DEFER({ internal::PlatformShutdown(platform_ticket); });

        t_gfx *const gfx = internal::GFXStartup(platform_ticket, &perm_arena, &temp_arena);
        ZCL_DEFER({ internal::GFXShutdown(gfx); });

        t_rendering_basis *const rendering_basis = RenderingBasisCreate(gfx);
        ZCL_DEFER({ RenderingBasisDestroy(rendering_basis); });

        const t_audio_ticket_mut audio_ticket = internal::AudioStartup(&perm_arena);
        ZCL_DEFER({ internal::AudioShutdown(audio_ticket); });

        zcl::t_rng *const rng = zcl::RNGCreate(zcl::RandGenSeed(), &perm_arena);

        zcl::ArenaRewind(&temp_arena);

        void *const user_mem = config.user_mem_size > 0 ? zcl::ArenaPush(&perm_arena, config.user_mem_size, config.user_mem_alignment) : nullptr;

        config.init_func({
            .perm_arena = &perm_arena,
            .temp_arena = &temp_arena,
            .platform_ticket = platform_ticket,
            .gfx = gfx,
            .audio_ticket = audio_ticket,
            .rng = rng,
            .user_mem = user_mem,
        });

        ZCL_DEFER({
            config.deinit_func({
                .perm_arena = &perm_arena,
                .temp_arena = &temp_arena,
                .platform_ticket = platform_ticket,
                .gfx = gfx,
                .audio_ticket = audio_ticket,
                .rng = rng,
                .user_mem = user_mem,
            });
        });

        //
        // Main Loop
        //
        zcl::t_b8 frame_first = true;
        zcl::t_f64 frame_time_last = 0.0;

        zcl::t_f64 fps = 0.0;
        zcl::t_f64 fps_time_accum = 0.0;
        zcl::t_i32 fps_frame_cnt_accum = 0;
        constexpr zcl::t_f64 k_fps_refresh_time = 1.0;

        zcl::t_f64 tick_interval_accum = 0.0;

        while (!WindowCheckCloseRequested(platform_ticket)) {
            internal::PollOSEvents(platform_ticket, input_state);

            const zcl::t_b8 window_focused = WindowCheckFocused(platform_ticket);

            internal::AudioSetFrozen(audio_ticket, !window_focused);

            const zcl::t_f64 frame_time = GetTime(platform_ticket);

            const zcl::t_f64 tick_interval_target = 1.0 / config.tps_target;
            const zcl::t_f64 tick_interval_limit = tick_interval_target * 8.0;

            if (!frame_first) {
                const zcl::t_f64 frame_time_delta_raw = frame_time - frame_time_last;
                constexpr zcl::t_f64 k_frame_time_delta_limit = 1.0;
                const zcl::t_f64 frame_time_delta_capped = zcl::CalcMin(frame_time_delta_raw, k_frame_time_delta_limit);

                fps_time_accum += frame_time_delta_capped;
                fps_frame_cnt_accum++;

                if (fps_time_accum >= k_fps_refresh_time) {
                    fps = fps_frame_cnt_accum / fps_time_accum;
                    fps_time_accum -= k_fps_refresh_time;
                    fps_frame_cnt_accum = 0;
                }

                // Run ticks only if the window is focused.
                if (window_focused) {
                    tick_interval_accum += frame_time_delta_capped;

                    if (tick_interval_accum > tick_interval_limit) {
                        tick_interval_accum = tick_interval_limit;
                    }

                    while (tick_interval_accum >= tick_interval_target) {
                        zcl::ArenaRewind(&temp_arena);

                        internal::SoundsProcessFinished(audio_ticket);

                        config.tick_func({
                            .perm_arena = &perm_arena,
                            .temp_arena = &temp_arena,
                            .input_state = input_state,
                            .platform_ticket = platform_ticket,
                            .gfx = gfx,
                            .audio_ticket = audio_ticket,
                            .rng = rng,
                            .fps = fps,
                            .user_mem = user_mem,
                        });

                        internal::InputClearEvents(input_state);

                        tick_interval_accum -= tick_interval_target;
                    }
                }
            }

            zcl::ArenaRewind(&temp_arena);

#if 0
            FrameBegin(gfx);

            config.render_func({
                .perm_arena = &perm_arena,
                .temp_arena = &temp_arena,
                .platform_ticket = platform_ticket,
                .gfx = gfx,
                .rng = rng,
                .fps = fps,
                .user_mem = user_mem,
            });

            FrameEnd(gfx);
#endif

            if (frame_first) {
                internal::WindowShow(platform_ticket);
                frame_first = false;
            }

            frame_time_last = frame_time;
        }
    }
}
