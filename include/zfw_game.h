#ifndef ZFW_GAME_H
#define ZFW_GAME_H

#include <stdbool.h>
#include <stdint.h>
#include <cu.h>
#include "zfw_input.h"
#include "zfw_graphics.h"
#include "zfw_math.h"
#include "zfw_audio.h"

typedef enum {
    zfw_ek_window_flags_resizable = 1 << 0,
    zfw_ek_window_flags_hide_cursor = 1 << 1
} zfw_e_window_flags;

typedef struct {
    zfw_s_vec_2d_s32 pos;
    zfw_s_vec_2d_s32 size;
    bool fullscreen;
} zfw_s_window_state;

typedef struct {
    void* user_mem;
    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;
    zfw_s_window_state window_state;
    zfw_s_audio_sys* audio_sys;
} zfw_s_game_init_func_data;

typedef enum {
    ek_game_tick_func_result_default, // Continue running the game as normal.
    ek_game_tick_func_result_exit,
    ek_game_tick_func_result_error
} zfw_e_game_tick_func_result;

typedef struct {
    void* user_mem;
    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;
    zfw_s_window_state window_state;
    const zfw_s_input_state* input_state;
    const zfw_s_input_state* input_state_last;
    const zfw_t_unicode_buf* unicode_buf;
    zfw_s_audio_sys* audio_sys;
} zfw_s_game_tick_func_data;

typedef struct {
    void* user_mem;
    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;
    zfw_s_vec_2d mouse_pos;
    zfw_s_rendering_context rendering_context;
} zfw_s_game_render_func_data;

typedef struct {
    size_t user_mem_size; // How much memory should be allocated in the permanent arena for your use? This might be the size of a specific struct, for example.
    size_t user_mem_alignment; // The alignment of the above memory.

    zfw_s_vec_2d_s32 window_init_size;
    const char* window_title;
    zfw_e_window_flags window_flags;

    // Below are pointers to functions that the framework will call for you. The provided struct pointers expose parts of the framework state for you to work with.
    bool (*init_func)(const zfw_s_game_init_func_data* const func_data); // Called as one of the last steps of the game initialisation phase.
    zfw_e_game_tick_func_result (*tick_func)(const zfw_s_game_tick_func_data* const func_data); // Called once every tick.
    bool (*render_func)(const zfw_s_game_render_func_data* const func_data); // Called after a tick.
    void (*clean_func)(void* const user_mem); // Called when the game ends (including if it ends in error). This is not called if the initialisation function failed or was not yet called.

    int surf_cnt; // How many surfaces to generate and auto-refresh.
} zfw_s_game_info;

bool ZFW_RunGame(const zfw_s_game_info* const info);

#endif
