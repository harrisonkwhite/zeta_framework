#include <zf/zf_game.h>

#include <zf/zf_rng.h>
#include <zf/zf_debug.h>

namespace zf {
    constexpr t_size g_perm_mem_arena_size = Megabytes(80);
    constexpr t_size g_temp_mem_arena_size = Megabytes(40);
    constexpr t_s32 g_gl_resource_arena_res_limit = 1024;

    enum class ec_game_run_stage {
        nothing_initted,
        perm_mem_arena_initted,
        temp_mem_arena_initted,
        window_initted,
        gfx_res_arena_initted,
        dev_init_func_ran_and_succeeded
    };

    struct s_game {
        ec_game_run_stage run_stage = ec_game_run_stage::nothing_initted; // Used to determine what needs to be cleaned up.

        s_mem_arena perm_mem_arena; // The memory in here exists for the lifetime of the program, it does not get reset.
        s_mem_arena temp_mem_arena; // While the memory here also exists for the program lifetime, it gets reset after game initialisation and after every frame. Useful if you just need some temporary working space.

        c_gfx_resource_arena gfx_res_arena; // For GFX resources existing for the lifetime of the game.

        void* dev_mem = nullptr; // Memory optionally reserved by the developer for their own use, accessible in their defined functions through the provided ZF context.
    };

    static t_b8 ExecGameInitAndMainLoop(s_game& game, const s_game_info& info) {
        //
        // Initialisation
        //
        ConfigErrorOutput();

        InitRNG();

        // Initialise memory arenas.
        if (!MakeMemArena(g_perm_mem_arena_size, game.perm_mem_arena)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        game.run_stage = ec_game_run_stage::perm_mem_arena_initted;

        if (!MakeMemArena(g_temp_mem_arena_size, game.temp_mem_arena)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        game.run_stage = ec_game_run_stage::temp_mem_arena_initted;

        // Initialise the window.
        if (!InitWindow(info.window_init_size, info.window_title, info.window_flags)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        game.run_stage = ec_game_run_stage::window_initted;

        // Initialise the permanent GFX resource arena.
        if (!game.gfx_res_arena.Init(game.perm_mem_arena, 1024)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        game.run_stage = ec_game_run_stage::gfx_res_arena_initted;

        // Initialise the rendering basis.
        c_renderer renderer;

        if (!renderer.Init(game.gfx_res_arena, game.perm_mem_arena, game.temp_mem_arena)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        // Initialise developer memory.
        if (info.dev_mem_size > 0) {
            game.dev_mem = PushToMemArena(game.perm_mem_arena, info.dev_mem_size, info.dev_mem_alignment);

            if (!game.dev_mem) {
                ZF_REPORT_FAILURE();
                return false;
            }
        }

        // Run the developer's initialisation function.
        {
            const s_game_init_context context = {
                .dev_mem = game.dev_mem,
                .perm_mem_arena = game.perm_mem_arena,
                .temp_mem_arena = game.temp_mem_arena,
                .gfx_res_arena = game.gfx_res_arena
            };

            if (!info.init_func(context)) {
                ZF_REPORT_FAILURE();
                return false;
            }
        }

        game.run_stage = ec_game_run_stage::dev_init_func_ran_and_succeeded;

        // Now that everything is set up, we can show the window.
        ShowWindow();

        //
        // Main Loop
        //
        t_f64 frame_time_last = GetTime();
        t_f64 frame_dur_accum = 0.0;

        while (!ShouldWindowClose()) {
            RewindMemArena(game.temp_mem_arena, 0);

            const t_f64 frame_time = GetTime();
            const t_f64 frame_time_delta = frame_time - frame_time_last;
            frame_dur_accum += frame_time_delta;
            frame_time_last = frame_time;

            const t_f64 targ_tick_interval = 1.0 / info.targ_ticks_per_sec;

            // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick and update the display.
            if (frame_dur_accum >= targ_tick_interval) {
                do {
                    const s_game_tick_context context = {
                        .dev_mem = game.dev_mem,
                        .perm_mem_arena = game.perm_mem_arena,
                        .temp_mem_arena = game.temp_mem_arena
                    };

                    const e_game_tick_result res = info.tick_func(context);

                    ClearInputEvents();

                    if (res == ek_game_tick_result_exit) {
                        ZF_LOG("Exit request detected from developer game tick function...");
                        SetWindowShouldClose(true);
                    }

                    if (res == ek_game_tick_result_error) {
                        ZF_REPORT_FAILURE();
                        return false;
                    }

                    frame_dur_accum -= targ_tick_interval;
                } while (frame_dur_accum >= targ_tick_interval);

                renderer.Begin();

                {
                    const s_game_render_context context = {
                        .dev_mem = game.dev_mem,
                        .perm_mem_arena = game.perm_mem_arena,
                        .temp_mem_arena = game.temp_mem_arena,
                        .renderer = renderer
                    };

                    if (!info.render_func(context)) {
                        ZF_REPORT_FAILURE();
                        return false;
                    }
                }

                renderer.End();

                SwapBuffers();
            }

            PollEvents();
        }

        return true;
    }

    t_b8 RunGame(const s_game_info& info) {
        AssertGameInfoValidity(info);

        s_game game;

        const t_b8 success = ExecGameInitAndMainLoop(game, info);

        // Clean up.
        for (auto i = static_cast<t_s32>(game.run_stage); i >= 0; i--) {
            switch (static_cast<ec_game_run_stage>(i)) {
                case ec_game_run_stage::nothing_initted:
                    break;

                case ec_game_run_stage::perm_mem_arena_initted:
                    ReleaseMemArena(game.perm_mem_arena);
                    break;

                case ec_game_run_stage::temp_mem_arena_initted:
                    ReleaseMemArena(game.temp_mem_arena);
                    break;

                case ec_game_run_stage::window_initted:
                    ReleaseWindow();
                    break;

                case ec_game_run_stage::gfx_res_arena_initted:
                    game.gfx_res_arena.Release();
                    break;

                case ec_game_run_stage::dev_init_func_ran_and_succeeded:
                    if (info.clean_func) {
                        info.clean_func(game.dev_mem);
                    }

                    break;
            }
        }

#ifndef ZF_DEBUG
        if (!success) {
            ShowErrorBox("Error", "A fatal error occurred! Please check \"error.log\" for details.");
        }
#endif

        return success;
    }
}
