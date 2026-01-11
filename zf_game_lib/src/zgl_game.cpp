#include <zgl/zgl_game.h>

#include <zgl/zgl_platform.h>
#include <zgl/zgl_audio.h>

namespace zgl::game {
    constexpr zcl::t_f64 k_init_tps_target = 60.0;
    constexpr zcl::math::t_v2_i k_init_window_size = {1280, 720};

    static struct {
        zcl::t_b8 running;
        zcl::t_f64 tps_targ;
    } g_module_state;

    void run(const t_config &config) {
        ZF_REQUIRE(!g_module_state.running);
        config_assert_valid(config);

        g_module_state = {
            .running = true,
            .tps_targ = k_init_tps_target,
        };

        ZF_DEFER({ g_module_state = {}; });

        //
        // Initialisation
        //
        zcl::mem::t_arena perm_arena = zcl::mem::arena_create_blockbased();
        ZF_DEFER({ zcl::mem::arena_destroy(&perm_arena); });

        zcl::mem::t_arena temp_arena = zcl::mem::arena_create_blockbased();
        ZF_DEFER({ zcl::mem::arena_destroy(&temp_arena); });

        platform::module_startup(k_init_window_size);
        ZF_DEFER({ platform::module_shutdown(); });

        input::t_state *const input_state = input::create_state(&perm_arena);

        gfx::t_resource_group *perm_gfx_resource_group;
        gfx::t_frame_basis *const frame_basis = gfx::module_startup(&perm_arena, &temp_arena, &perm_gfx_resource_group);
        ZF_DEFER({ gfx::module_shutdown(frame_basis); });

        audio::module_startup();
        ZF_DEFER({ audio::module_shutdown(); });

        zcl::rand::t_rng *const rng = zcl::rand::rng_create(0, &perm_arena); // @todo: Proper seed!

        void *const user_mem = config.user_mem_size > 0 ? zcl::mem::arena_push(&perm_arena, config.user_mem_size, config.user_mem_alignment) : nullptr;

        config.init_func({
            .perm_arena = &perm_arena,
            .temp_arena = &temp_arena,
            .perm_gfx_resource_group = perm_gfx_resource_group,
            .rng = rng,
            .user_mem = user_mem,
        });

        ZF_DEFER({
            config.deinit_func(user_mem);
        });

        //
        // Main Loop
        //

        // @todo: Need to properly handle overflow cases!

        zcl::t_i32 frame_cnt = 0;
        zcl::t_f64 frame_time_last = 0.0;
        zcl::t_f64 frame_dur_accum = 0.0;

        zcl::t_f64 fps = 0.0;
        zcl::t_i32 fps_frame_cnt = 0;
        zcl::t_f64 fps_time_accum = 0.0;

        while (!platform::window_check_close_requested()) {
            platform::poll_os_events(input_state);

            const zcl::t_f64 frame_time = platform::get_time();

            if (frame_cnt > 0) {
                const zcl::t_f64 frame_time_delta = frame_time - frame_time_last;
                frame_dur_accum += frame_time_delta;

                fps_time_accum += frame_time_delta;
                fps = fps_frame_cnt / fps_time_accum;

                if (fps_time_accum >= 1.0) {
                    fps_frame_cnt = 0;
                    fps_time_accum = 0.0;
                }

                const zcl::t_f64 targ_tick_interval = 1.0 / g_module_state.tps_targ;

                while (frame_dur_accum >= targ_tick_interval) {
                    zcl::mem::arena_rewind(&temp_arena);

                    audio::proc_finished_sounds();

                    config.tick_func({
                        .perm_arena = &perm_arena,
                        .temp_arena = &temp_arena,
                        .input_state = input_state,
                        .perm_gfx_resource_group = perm_gfx_resource_group,
                        .rng = rng,
                        .fps = fps,
                        .user_mem = user_mem,
                    });

                    input::clear_events(input_state);

                    frame_dur_accum -= targ_tick_interval;
                }
            }

            zcl::mem::arena_rewind(&temp_arena);

            gfx::t_frame_context *const frame_context = gfx::frame_begin(frame_basis, &temp_arena);

            config.render_func({
                .perm_arena = &perm_arena,
                .temp_arena = &temp_arena,
                .frame_context = frame_context,
                .rng = rng,
                .fps = fps,
                .user_mem = user_mem,
            });

            gfx::frame_end(frame_context);

            if (frame_cnt == 0) {
                platform::window_show();
            }

            frame_cnt++;
            frame_time_last = frame_time;

            fps_frame_cnt++;
        }
    }

    void tps_set_target(const zcl::t_f64 tps) {
        ZF_ASSERT(g_module_state.running);
        ZF_ASSERT(tps > 0.0);

        g_module_state.tps_targ = tps;
    }
}
