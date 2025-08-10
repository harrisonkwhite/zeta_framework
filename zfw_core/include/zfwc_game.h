#ifndef ZFWC_GAME_H
#define ZFWC_GAME_H

#include <stdbool.h>
#include <stdint.h>
#include <cu.h>
#include "zfwc_input.h"
#include "zfwc_graphics.h"

typedef enum {
    ek_window_flags_resizable = 1 << 0,
    ek_window_flags_hide_cursor = 1 << 1
} e_window_flags;

typedef struct {
    s_v2_s32 pos;
    s_v2_s32 size;
    bool fullscreen;
} s_window_state;

typedef struct {
    void* dev_mem;

    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;

    s_window_state window_state;

    s_gl_resource_arena* gl_res_arena;

    s_rendering_basis* rendering_basis;
} s_game_init_context;

typedef enum {
    ek_game_tick_result_normal, // Continue running the game as normal.
    ek_game_tick_result_exit,
    ek_game_tick_result_error
} e_game_tick_result;

typedef struct {
    void* dev_mem;

    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;

    s_window_state window_state;

    s_input_context input_context;

    s_gl_resource_arena* gl_res_arena;

    s_rendering_basis* rendering_basis;
} s_game_tick_context;

typedef struct {
    void* dev_mem;

    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;

    s_v2 mouse_pos;

    s_rendering_context rendering_context;
} s_game_render_context;

typedef struct {
    s_v2_s32 window_init_size;
    s_char_array_view window_title;
    e_window_flags window_flags;

    size_t dev_mem_size; // How much memory should be allocated in the permanent arena for your use? This might be the size of a specific struct, for example.
    size_t dev_mem_alignment; // The alignment of the above memory.

    // Below are pointers to functions that the framework will call for you. The provided struct pointers expose parts of the framework state for you to work with.
    bool (*init_func)(const s_game_init_context* const func_data); // Called as one of the last steps of the game initialisation phase.
    e_game_tick_result (*tick_func)(const s_game_tick_context* const func_data); // Called once every tick (which can occur multiple times a frame).
    bool (*render_func)(const s_game_render_context* const func_data); // Called after all ticks have been run.
    void (*clean_func)(void* const dev_mem); // Called when the game ends (including if it ends in error). This is not called if your initialisation function failed or hasn't yet been called.
} s_game_info;

static inline void AssertGameInfoValidity(const s_game_info* const info) {
    assert(info);

    assert(IsStrTerminated(info->window_title));
    assert(info->window_init_size.x > 0 && info->window_init_size.y > 0);

    assert((info->dev_mem_size == 0 && info->dev_mem_alignment == 0) || (info->dev_mem_size > 0 && IsAlignmentValid(info->dev_mem_alignment)));

    assert(info->init_func);
    assert(info->tick_func);
    assert(info->render_func);
}

bool RunGame(const s_game_info* const info);

#endif
