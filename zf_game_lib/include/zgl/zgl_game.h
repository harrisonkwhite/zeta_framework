#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx.h>

namespace zf
{
    struct s_platform_layer_info;
    struct s_audio_sys;

    struct s_game_init_context
    {
        void* dev_mem;

        s_mem_arena* mem_arena;
        s_mem_arena* temp_mem_arena;

        s_platform_layer_info* platform_layer_info;

        s_gfx_resource_arena* gfx_res_arena;

        s_audio_sys* audio_sys;
    };

    struct s_input_state;

    struct s_game_tick_context
    {
        void* dev_mem;

        s_mem_arena* mem_arena;
        s_mem_arena* temp_mem_arena;

        s_input_state* input_state;

        s_platform_layer_info* platform_layer_info;

        s_gfx_resource_arena* gfx_res_arena;

        s_audio_sys* audio_sys;
    };

    struct s_game_render_context
    {
        void* dev_mem;

        s_mem_arena* mem_arena;
        s_mem_arena* temp_mem_arena;

        s_rendering_context rendering_context;
    };

    // @todo: Shouldn't need this. Embed into RunGame() args.
    struct s_game_info
    {
        // @todo: Move out.
        t_size mem_arena_size;
        t_size temp_mem_arena_size;

        t_size dev_mem_size;
        t_size dev_mem_alignment;

        t_s32 targ_ticks_per_sec; // @todo: Move out and default this to 60 fps. Allow runtime
                                  // adjustment to anything in the range [1, 60].

        // Called once as one of the last steps of the game initialisation phase.
        t_b8 (*init_func)(const s_game_init_context& context);

        // Called once every tick (which can occur multiple times a frame).
        t_b8 (*tick_func)(const s_game_tick_context& context);

        // Called after all ticks of the frame have been run.
        t_b8 (*render_func)(const s_game_render_context& context);

        // Optional. Called when the game ends (including if it ends in error). This is not called
        // if your initialisation function failed or hasn't yet been called.
        void (*clean_func)(void* const dev_mem);
    };

    t_b8 RunGame(const s_game_info& info);
}
