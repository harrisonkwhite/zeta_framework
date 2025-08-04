#ifndef ZFW_GAME_H
#define ZFW_GAME_H

#include <stdbool.h>
#include <stdint.h>
#include <cu.h>
#include "zfw_input.h"
#include "zfw_rendering.h"
#include "zfw_math.h"
#include "zfw_audio.h"

typedef enum {
    zfw_ek_window_flags_resizable = 1 << 0,
    zfw_ek_window_flags_hide_cursor = 1 << 1
} zfw_e_window_flags;

typedef struct {
    zfw_s_vec_2d_int pos;
    zfw_s_vec_2d_int size;
    bool fullscreen;
} zfw_s_window_state;

typedef struct {
    void* dev_mem;

    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;

    zfw_s_window_state window_state;

    zfw_s_gl_resource_arena* const gl_res_arena;
    const zfw_s_rendering_basis* const rendering_basis;

    zfw_s_audio_sys* audio_sys;
} zfw_s_game_init_context;

typedef enum {
    zfw_ek_game_tick_result_normal, // Continue running the game as normal.
    zfw_ek_game_tick_result_exit,
    zfw_ek_game_tick_result_error
} zfw_e_game_tick_result;

typedef struct {
    void* dev_mem;

    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;

    zfw_s_window_state window_state;

    zfw_s_input_context input_context;

    zfw_s_gl_resource_arena* const gl_res_arena;
    const zfw_s_rendering_basis* const rendering_basis;

    zfw_s_audio_sys* audio_sys;
} zfw_s_game_tick_context;

typedef struct {
    void* dev_mem;

    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;

    zfw_s_vec_2d mouse_pos;

    zfw_s_rendering_context rendering_context;
} zfw_s_game_render_context;

typedef struct {
    zfw_s_vec_2d_int window_init_size;
    const char* window_title;
    zfw_e_window_flags window_flags;

    size_t dev_mem_size; // How much memory should be allocated in the permanent arena for your use? This might be the size of a specific struct, for example.
    size_t dev_mem_alignment; // The alignment of the above memory.

    // Below are pointers to functions that the framework will call for you. The provided struct pointers expose parts of the framework state for you to work with.
    bool (*init_func)(const zfw_s_game_init_context* const func_data); // Called as one of the last steps of the game initialisation phase.
    zfw_e_game_tick_result (*tick_func)(const zfw_s_game_tick_context* const func_data); // Called once every tick (which can occur multiple times a frame).
    bool (*render_func)(const zfw_s_game_render_context* const func_data); // Called after all ticks have been run.
    void (*clean_func)(void* const dev_mem); // Called when the game ends (including if it ends in error). This is not called if your initialisation function failed or hasn't yet been called.
} zfw_s_game_info;

static inline void ZFW_AssertGameInfoValidity(const zfw_s_game_info* const info) {
    assert(info);

    assert(info->window_title);
    assert(info->window_init_size.x > 0 && info->window_init_size.y > 0);

    assert((info->dev_mem_size == 0 && info->dev_mem_alignment == 0) || (info->dev_mem_size > 0 && IsAlignmentValid(info->dev_mem_alignment)));

    assert(info->init_func);
    assert(info->tick_func);
    assert(info->render_func);
}

bool ZFW_RunGame(const zfw_s_game_info* const info);

#endif
