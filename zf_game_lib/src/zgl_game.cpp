#include <zgl/zgl_game.h>

#include <zgl/zgl_input.h>
#include <zgl/zgl_platform.h>
#include <zgl/zgl_audio.h>

namespace zgl {
    constexpr zcl::t_v2_i k_window_size_init = {1280, 720};

    void GameRun(const t_game_config &config) {
        GameConfigAssertValid(config);

        //
        // Initialization
        //
        zcl::t_arena perm_arena = zcl::ArenaCreateBlockBased();
        ZCL_DEFER({ zcl::ArenaDestroy(&perm_arena); });

        zcl::t_arena temp_arena = zcl::ArenaCreateBlockBased();
        ZCL_DEFER({ zcl::ArenaDestroy(&temp_arena); });

        t_input_state *const input_state = detail::InputCreateState(&perm_arena);

        t_platform *const platform = detail::PlatformStartup(k_window_size_init, input_state, &perm_arena);
        ZCL_DEFER({ detail::PlatformShutdown(platform); });

        t_frame_basis *frame_basis;
        t_gfx *const gfx = detail::GFXStartup(platform, &perm_arena, &temp_arena, &frame_basis);
        ZCL_DEFER({ detail::GFXShutdown(gfx, frame_basis); });

        t_audio_sys *const audio_sys = detail::AudioStartup(&perm_arena);
        ZCL_DEFER({ detail::AudioShutdown(audio_sys); });

        zcl::t_rng *const rng = zcl::RNGCreate(zcl::RandGenSeed(), &perm_arena);

        zcl::ArenaRewind(&temp_arena);

        void *const user_mem = config.user_mem_size > 0 ? zcl::ArenaPush(&perm_arena, config.user_mem_size, config.user_mem_alignment) : nullptr;

        config.init_func({
            .perm_arena = &perm_arena,
            .temp_arena = &temp_arena,
            .platform = platform,
            .gfx = gfx,
            .audio_sys = audio_sys,
            .rng = rng,
            .user_mem = user_mem,
        });

        ZCL_DEFER({
            config.deinit_func({
                .perm_arena = &perm_arena,
                .temp_arena = &temp_arena,
                .platform = platform,
                .gfx = gfx,
                .audio_sys = audio_sys,
                .rng = rng,
                .user_mem = user_mem,
            });
        });

        //
        // Main Loop
        //
        zcl::t_b8 frame_first = true;
        zcl::t_f64 frame_time_last = 0.0;

        zcl::t_f64 tick_interval_accum = 0.0;

        zcl::t_f64 fps = 0.0;
        zcl::t_f64 fps_time_accum = 0.0;
        zcl::t_i32 fps_frame_cnt_accum = 0;
        constexpr zcl::t_f64 k_fps_refresh_time = 1.0;

        while (!WindowCheckCloseRequested(platform)) {
            detail::PollOSEvents(platform);

            const zcl::t_f64 frame_time = GetTime(platform);

            const zcl::t_f64 tick_interval_target = 1.0 / config.tps_target;
            const zcl::t_f64 tick_interval_limit = tick_interval_target * 8.0;

            if (!frame_first) {
                const zcl::t_f64 frame_time_delta_raw = frame_time - frame_time_last;
                constexpr zcl::t_f64 k_frame_time_delta_limit = 1.0;
                const zcl::t_f64 frame_time_delta_capped = zcl::CalcMin(frame_time_delta_raw, k_frame_time_delta_limit);

                tick_interval_accum += frame_time_delta_capped;

                if (tick_interval_accum > tick_interval_limit) {
                    tick_interval_accum = tick_interval_limit;
                }

                fps_time_accum += frame_time_delta_capped;
                fps_frame_cnt_accum++;

                if (fps_time_accum >= k_fps_refresh_time) {
                    fps = fps_frame_cnt_accum / fps_time_accum;
                    fps_time_accum -= k_fps_refresh_time;
                    fps_frame_cnt_accum = 0;
                }

                while (tick_interval_accum >= tick_interval_target) {
                    zcl::ArenaRewind(&temp_arena);

                    detail::SoundsProcessFinished(audio_sys);

                    config.tick_func({
                        .perm_arena = &perm_arena,
                        .temp_arena = &temp_arena,
                        .input_state = input_state,
                        .platform = platform,
                        .gfx = gfx,
                        .audio_sys = audio_sys,
                        .rng = rng,
                        .fps = fps,
                        .user_mem = user_mem,
                    });

                    detail::InputClearEvents(input_state);

                    tick_interval_accum -= tick_interval_target;
                }
            }

            zcl::ArenaRewind(&temp_arena);

            const t_frame_context frame_context = detail::FrameBegin(gfx, frame_basis, platform, &temp_arena);

            config.render_func({
                .perm_arena = &perm_arena,
                .temp_arena = &temp_arena,
                .platform = platform,
                .frame_context = frame_context,
                .rng = rng,
                .fps = fps,
                .user_mem = user_mem,
            });

            detail::FrameEnd(frame_context);

            if (frame_first) {
                detail::WindowShow(platform);
                frame_first = false;
            }

            frame_time_last = frame_time;
        }
    }
}
