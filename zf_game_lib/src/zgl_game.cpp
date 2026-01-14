#include <zgl/zgl_game.h>

#include <zgl/zgl_input.h>
#include <zgl/zgl_platform.h>
#include <zgl/zgl_gfx.h>
#include <zgl/zgl_audio.h>

namespace zgl::game {
    constexpr zcl::t_f64 k_init_tps_target = 60.0;
    constexpr zcl::t_v2_i k_init_window_size = {1280, 720};

    static struct {
        zcl::t_b8 running;
        zcl::t_f64 tps_targ;
    } g_module_state;

    void Run(const t_config &config) {
        ZCL_REQUIRE(!g_module_state.running);
        ConfigAssertValid(config);

        g_module_state = {
            .running = true,
            .tps_targ = k_init_tps_target,
        };

        ZCL_DEFER({ g_module_state = {}; });

        //
        // Initialization
        //
        zcl::t_arena perm_arena = zcl::ArenaCreateBlockBased();
        ZCL_DEFER({ zcl::ArenaDestroy(&perm_arena); });

        zcl::t_arena temp_arena = zcl::ArenaCreateBlockBased();
        ZCL_DEFER({ zcl::ArenaDestroy(&temp_arena); });

        platform::ModuleStartup(k_init_window_size);
        ZCL_DEFER({ platform::ModuleShutdown(); });

        input::t_state *const input_state = input::CreateState(&perm_arena);

        gfx::t_resource_group *perm_gfx_resource_group;
        gfx::t_frame_basis *const frame_basis = gfx::ModuleStartup(&perm_arena, &temp_arena, &perm_gfx_resource_group);
        ZCL_DEFER({ gfx::ModuleShutdown(frame_basis); });

        audio::ModuleStartup();
        ZCL_DEFER({ audio::ModuleShutdown(); });

        zcl::t_rng *const rng = zcl::RNGCreate(zcl::RandGenSeed(), &perm_arena);

        zcl::ArenaRewind(&temp_arena);

        void *const user_mem = config.user_mem_size > 0 ? zcl::ArenaPush(&perm_arena, config.user_mem_size, config.user_mem_alignment) : nullptr;

        config.init_func({
            .perm_arena = &perm_arena,
            .temp_arena = &temp_arena,
            .perm_gfx_resource_group = perm_gfx_resource_group,
            .rng = rng,
            .user_mem = user_mem,
        });

        ZCL_DEFER({
            config.deinit_func(user_mem);
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

        while (!platform::WindowCheckCloseRequested()) {
            platform::PollEvents(input_state);

            const zcl::t_f64 frame_time = platform::GetTime();

            const zcl::t_f64 tick_interval_targ = 1.0 / g_module_state.tps_targ;
            const zcl::t_f64 tick_interval_limit = tick_interval_targ * 8.0;

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

                while (tick_interval_accum >= tick_interval_targ) {
                    zcl::ArenaRewind(&temp_arena);

                    audio::SoundsProcFinished();

                    config.tick_func({
                        .perm_arena = &perm_arena,
                        .temp_arena = &temp_arena,
                        .input_state = input_state,
                        .perm_gfx_resource_group = perm_gfx_resource_group,
                        .rng = rng,
                        .fps = fps,
                        .user_mem = user_mem,
                    });

                    input::ClearEvents(input_state);

                    tick_interval_accum -= tick_interval_targ;
                }
            }

            zcl::ArenaRewind(&temp_arena);

            gfx::t_frame_context *const frame_context = gfx::FrameBegin(frame_basis, &temp_arena);

            config.render_func({
                .perm_arena = &perm_arena,
                .temp_arena = &temp_arena,
                .frame_context = frame_context,
                .rng = rng,
                .fps = fps,
                .user_mem = user_mem,
            });

            gfx::FrameEnd(frame_context);

            if (frame_first) {
                platform::WindowShow();
                frame_first = false;
            }

            frame_time_last = frame_time;
        }
    }

    void SetTargetTPS(const zcl::t_f64 tps) {
        ZCL_ASSERT(g_module_state.running);
        ZCL_ASSERT(tps > 0.0);

        g_module_state.tps_targ = tps;
    }
}
