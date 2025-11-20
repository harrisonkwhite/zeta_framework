#include <zf/zf_game.h>

#include <zf/zf_rng.h>
#include <zf/zf_audio.h>
#include <zf/zf_debug.h>

namespace zf {
    constexpr t_s32 g_gfx_resource_arena_cap = 1024;

    struct s_game {
        s_mem_arena perm_mem_arena; // The memory in here exists for the lifetime of the program, it does not get reset.
        s_mem_arena temp_mem_arena; // While the memory here also exists for the program lifetime, it gets reset after game initialisation and after every frame. Useful if you just need some temporary working space.

        gfx::s_resource_arena gfx_res_arena; // For GFX resources existing for the lifetime of the game.

        void* dev_mem; // Memory optionally reserved by the developer for their own use, accessible in their defined functions through the provided ZF context.
    };

    using t_cleanup_op = void (*)(s_game& game, const s_game_info& info);
    using t_cleanup_op_stack = s_static_stack<t_cleanup_op, 6>;

    static t_b8 ExecGameInitAndMainLoop(s_game& game, const s_game_info& info, t_cleanup_op_stack& cleanup_ops) {
        //
        // Initialisation
        //
        ConfigErrorOutput();

        InitRNG();

        // Initialise memory arenas.
        if (!MakeMemArena(game.perm_mem_arena)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        StackPush(cleanup_ops, static_cast<t_cleanup_op>([](s_game& game, const s_game_info& info) {
            ReleaseMemArena(game.perm_mem_arena);
        }));

        if (!MakeMemArena(game.temp_mem_arena)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        StackPush(cleanup_ops, static_cast<t_cleanup_op>([](s_game& game, const s_game_info& info) {
            ReleaseMemArena(game.temp_mem_arena);
        }));

        // Initialise the window.
        if (!InitWindow(info.window_init_size, info.window_title, info.window_flags)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        StackPush(cleanup_ops, static_cast<t_cleanup_op>([](s_game& game, const s_game_info& info) {
            ReleaseWindow();
        }));

        // Initialise the permanent GFX resource arena.
        if (!gfx::MakeResourceArena(game.perm_mem_arena, g_gfx_resource_arena_cap, game.gfx_res_arena)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        StackPush(cleanup_ops, static_cast<t_cleanup_op>([](s_game& game, const s_game_info& info) {
            gfx::ReleaseResourceArena(game.gfx_res_arena);
        }));

        // Initialise the rendering basis.
        s_rendering_basis rendering_basis;

        if (!MakeRenderingBasis(game.gfx_res_arena, game.temp_mem_arena, rendering_basis)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        // Initialise audio system.
        if (!audio::InitSys()) {
            ZF_REPORT_FAILURE();
            return false;
        }

        StackPush(cleanup_ops, static_cast<t_cleanup_op>([](s_game& game, const s_game_info& info) {
            audio::ShutdownSys();
        }));

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

        StackPush(cleanup_ops, static_cast<t_cleanup_op>([](s_game& game, const s_game_info& info) {
            if (info.clean_func) {
                info.clean_func(game.dev_mem);
            }
        }));

        // Now that everything is set up, we can show the window.
        ShowWindow();

        //
        // Main Loop
        //
        t_f64 frame_time_last = GetTime();
        t_f64 frame_dur_accum = 0.0;

        while (!ShouldWindowClose()) {
            ClearMemArena(game.temp_mem_arena);

            const t_f64 frame_time = GetTime();
            const t_f64 frame_time_delta = frame_time - frame_time_last;
            frame_dur_accum += frame_time_delta;
            frame_time_last = frame_time;

            const t_f64 targ_tick_interval = 1.0 / info.targ_ticks_per_sec;

            // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick and update the display.
            if (frame_dur_accum >= targ_tick_interval) {
                //audio::ProcFinishedSounds();

                // Run possibly multiple ticks.
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

                // Perform a single render.
                s_rendering_state* const rendering_state = PrepareRenderingPhase(game.temp_mem_arena);

                if (!rendering_state) {
                    ZF_REPORT_FAILURE();
                    return false;
                }

                const s_rendering_context rendering_context = {
                    .basis = rendering_basis,
                    .state = *rendering_state
                };

                {
                    const s_game_render_context context = {
                        .dev_mem = game.dev_mem,
                        .perm_mem_arena = game.perm_mem_arena,
                        .temp_mem_arena = game.temp_mem_arena,
                        .rendering_context = rendering_context
                    };

                    if (!info.render_func(context)) {
                        ZF_REPORT_FAILURE();
                        return false;
                    }
                }

                CompleteRenderingPhase(rendering_context);

                SwapBuffers();
            }

            PollEvents();
        }

        return true;
    }

    t_b8 RunGame(const s_game_info& info) {
        AssertGameInfoValidity(info);

        s_game game = {};

        t_cleanup_op_stack cleanup_ops = {};

        const t_b8 success = ExecGameInitAndMainLoop(game, info, cleanup_ops);

        while (!IsStackEmpty(cleanup_ops)) {
            const t_cleanup_op op = StackPop(cleanup_ops);
            op(game, info);
        }

#ifndef ZF_DEBUG
        if (!success) {
            ShowErrorBox("Error", "A fatal error occurred! Please check \"error.log\" for details.");
        }
#endif

        return success;
    }
}
