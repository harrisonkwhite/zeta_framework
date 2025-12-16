#include <zgl/zgl_game.h>

#include <zgl/zgl_gfx.h>
#include <zgl/zgl_platform.h>
#include <zgl/zgl_rendering.h>

namespace zf {
    constexpr s_v2_i g_init_window_size = {1280, 720};
    constexpr t_f64 g_targ_ticks_per_sec = 60.0; // @todo: Make this customisable?

    void RunGame(const t_game_init_func init_func, const t_game_tick_func tick_func, const t_game_render_func render_func, const t_game_cleanup_func cleanup_func) {
        ZF_ASSERT(init_func);
        ZF_ASSERT(tick_func);
        ZF_ASSERT(render_func);

#ifndef ZF_DEBUG
        // Redirect stderr to crash log file.
        freopen("error.log", "w", stderr);
#endif

        //
        // Initialisation
        //
        auto perm_mem_arena = CreateMemArena(Megabytes(80));
        ZF_DEFER({ perm_mem_arena.Release(); });

        auto temp_mem_arena = CreateMemArena(Megabytes(10));
        ZF_DEFER({ temp_mem_arena.Release(); });

        internal::InitPlatform(g_init_window_size);
        ZF_DEFER({ internal::ShutdownPlatform(); });

        InitGFX();
        ZF_DEFER({ ShutdownGFX(); });

        auto rendering_basis = CreateRenderingBasis(perm_mem_arena, temp_mem_arena);

        init_func({
            .perm_mem_arena = perm_mem_arena,
            .temp_mem_arena = temp_mem_arena,
            .perm_gfx_res_arena = rendering_basis.gfx_res_arena,
        });

        ZF_DEFER({
            if (cleanup_func) {
                cleanup_func();
            }
        });

        //
        // Main Loop
        //
        internal::ShowWindow();

        t_f64 frame_time_last = Time();
        t_f64 frame_dur_accum = 0.0;

        while (!internal::ShouldWindowClose()) {
            temp_mem_arena.Rewind(0);

            internal::PollOSEvents();

            const t_f64 frame_time = Time();
            const t_f64 frame_time_delta = frame_time - frame_time_last;
            frame_dur_accum += frame_time_delta;
            frame_time_last = frame_time;

            const t_f64 targ_tick_interval = 1.0 / g_targ_ticks_per_sec;

            // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick and update the display.
            if (frame_dur_accum >= targ_tick_interval) {
                do {
                    tick_func({
                        .perm_mem_arena = perm_mem_arena,
                        .temp_mem_arena = temp_mem_arena,
                        .perm_gfx_res_arena = rendering_basis.gfx_res_arena,
                    });

                    internal::ClearInputEvents();

                    frame_dur_accum -= targ_tick_interval;
                } while (frame_dur_accum >= targ_tick_interval);

                // internal::PlatformLock();

                s_rendering_state &rs = internal::BeginRendering(rendering_basis, temp_mem_arena);

                render_func({
                    .perm_mem_arena = perm_mem_arena,
                    .temp_mem_arena = temp_mem_arena,
                    .rendering_state = rs,
                });

                internal::EndRendering(rs, temp_mem_arena);

                // internal::PlatformUnlock();
            }
        }
    }
}
