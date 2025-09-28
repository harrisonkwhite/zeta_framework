#include "zf_game.h"

#include <cstdio>
#include "zf_rng.h"

namespace zf {
    constexpr size_t g_perm_mem_arena_size = Megabytes(80);
    constexpr size_t g_temp_mem_arena_size = Megabytes(40);
    constexpr t_s32 g_gl_resource_arena_res_limit = 1024;

    enum class ec_game_run_stage {
        nothing_initted,
        perm_mem_arena_initted,
        temp_mem_arena_initted,
        window_initted,
        renderer_initted,
        dev_init_func_ran_and_succeeded
    };

    struct s_game {
        ec_game_run_stage run_stage; // Used to determine what needs to be cleaned up.

        c_mem_arena perm_mem_arena; // The memory in here exists for the lifetime of the program, it does not get reset.
        c_mem_arena temp_mem_arena; // While the memory here also exists for the program lifetime, it gets reset after game initialisation and after every frame. Useful if you just need some temporary working space.

        void* dev_mem; // Memory optionally reserved by the developer for their own use, accessible in their defined functions through the provided ZF context.
    };

    static bool ExecGameInitAndMainLoop(s_game& game, const s_game_info& info) {
        //
        // Initialisation
        //
        InitRNG();

        // Initialise memory arenas.
        if (!game.perm_mem_arena.Init(g_perm_mem_arena_size)) {
            ZF_LOG_ERROR("Failed to initialise the permanent memory arena!");
            return false;
        }

        game.run_stage = ec_game_run_stage::perm_mem_arena_initted;

        if (!game.temp_mem_arena.Init(g_temp_mem_arena_size)) {
            ZF_LOG_ERROR("Failed to initialise the temporary memory arena!");
            return false;
        }

        game.run_stage = ec_game_run_stage::temp_mem_arena_initted;

        // Initialise the window.
        if (!c_window::Init(info.window_init_size, info.window_title, info.window_flags)) {
            ZF_LOG_ERROR("Failed to initialise the window!");
            return false;
        }

        game.run_stage = ec_game_run_stage::window_initted;

        // Initialise the renderer.
        if (!c_renderer::Init(game.temp_mem_arena)) {
            ZF_LOG_ERROR("Failed to initialise the renderer!");
            return false;
        }

        game.run_stage = ec_game_run_stage::renderer_initted;

        // Initialise developer memory.
        if (info.dev_mem_size > 0) {
            game.dev_mem = game.perm_mem_arena.PushRaw(info.dev_mem_size, info.dev_mem_alignment);

            if (!game.dev_mem) {
                ZF_LOG_ERROR("Failed to reserve developer memory!");
                return false;
            }
        }

        // Run the developer's initialisation function.
        {
            const s_game_init_context context = {
                .dev_mem = game.dev_mem,
                .perm_mem_arena = game.perm_mem_arena,
                .temp_mem_arena = game.temp_mem_arena
            };

            if (!info.init_func(context)) {
                ZF_LOG_ERROR("Developer game initialisation function failed!");
                return false;
            }
        }

        game.run_stage = ec_game_run_stage::dev_init_func_ran_and_succeeded;

        // Now that everything is set up, we can show the window.
        c_window::Show();

        //
        // Main Loop
        //
        double frame_time_last = c_window::GetTime();
        double frame_dur_accum = 0.0;

        while (!c_window::ShouldClose()) {
            game.temp_mem_arena.Rewind(0);

            const double frame_time = c_window::GetTime();
            const double frame_time_delta = frame_time - frame_time_last;
            frame_dur_accum += frame_time_delta;
            frame_time_last = frame_time;

            const double targ_tick_interval = 1.0 / info.targ_ticks_per_sec;

            // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick and update the display.
            if (frame_dur_accum >= targ_tick_interval) {
                do {
                    const s_game_tick_context context = {
                        .dev_mem = game.dev_mem,
                        .perm_mem_arena = game.perm_mem_arena,
                        .temp_mem_arena = game.temp_mem_arena
                    };

                    const e_game_tick_result res = info.tick_func(context);

                    c_window::ClearInputEvents();

                    if (res == ek_game_tick_result_exit) {
                        ZF_LOG("Exit request detected from developer game tick function...");
                        c_window::SetWindowShouldClose(true);
                    }

                    if (res == ek_game_tick_result_error) {
                        ZF_LOG_ERROR("Developer game tick function failed!");
                        return false;
                    }

                    frame_dur_accum -= targ_tick_interval;
                } while (frame_dur_accum >= targ_tick_interval);

                c_renderer::Draw({32.0f, 32.0f}, {64.0f, 64.0f}, origins::g_origin_top_left, 0.0f, colors::g_blue);
                c_renderer::Draw({64.0f, 64.0f}, {64.0f, 64.0f}, origins::g_origin_top_left, 0.0f, colors::g_brown);

                c_renderer::CompleteFrame();
            }

            c_window::PollEvents();
        }

        return true;
    }

    bool RunGame(const s_game_info& info) {
        AssertGameInfoValidity(info);

        s_game game;

        const bool result = ExecGameInitAndMainLoop(game, info);

        // Clean up.
        for (auto i = static_cast<t_s32>(game.run_stage); i >= 0; i--) {
            switch (static_cast<ec_game_run_stage>(i)) {
                case ec_game_run_stage::nothing_initted:
                    break;

                case ec_game_run_stage::perm_mem_arena_initted:
                    game.perm_mem_arena.Clean();
                    break;

                case ec_game_run_stage::temp_mem_arena_initted:
                    game.temp_mem_arena.Clean();
                    break;

                case ec_game_run_stage::window_initted:
                    c_window::Clean();
                    break;

                case ec_game_run_stage::renderer_initted:
                    c_renderer::Shutdown();
                    break;

                case ec_game_run_stage::dev_init_func_ran_and_succeeded:
                    if (info.clean_func) {
                        info.clean_func(game.dev_mem);
                    }

                    break;
            }
        }

        return result;
    }
}
