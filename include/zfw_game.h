#ifndef ZFW_GAME_H
#define ZFW_GAME_H

#include <stdbool.h>
#include <stdint.h>
#include <GLFW/glfw3.h>
#include "zfw_math.h"
#include "zfw_rendering.h"
#include "zfw_audio.h"
#include "zfw_utils.h"

typedef char zfw_t_unicode_buf[32];

typedef uint64_t zfw_t_keys_down_bits;
typedef uint8_t zfw_t_mouse_buttons_down_bits;

typedef enum {
    zfw_ek_window_flags_resizable = 1 << 0,
    zfw_ek_window_flags_hide_cursor = 1 << 1
} zfw_e_window_flags;

typedef enum {
    zfw_ek_mouse_scroll_state_none,
    zfw_ek_mouse_scroll_state_down,
    zfw_ek_mouse_scroll_state_up
} zfw_e_mouse_scroll_state;

typedef enum {
    zfw_ek_key_code_space,
    zfw_ek_key_code_0,
    zfw_ek_key_code_1,
    zfw_ek_key_code_2,
    zfw_ek_key_code_3,
    zfw_ek_key_code_4,
    zfw_ek_key_code_5,
    zfw_ek_key_code_6,
    zfw_ek_key_code_7,
    zfw_ek_key_code_8,
    zfw_ek_key_code_9,
    zfw_ek_key_code_a,
    zfw_ek_key_code_b,
    zfw_ek_key_code_c,
    zfw_ek_key_code_d,
    zfw_ek_key_code_e,
    zfw_ek_key_code_f,
    zfw_ek_key_code_g,
    zfw_ek_key_code_h,
    zfw_ek_key_code_i,
    zfw_ek_key_code_j,
    zfw_ek_key_code_k,
    zfw_ek_key_code_l,
    zfw_ek_key_code_m,
    zfw_ek_key_code_n,
    zfw_ek_key_code_o,
    zfw_ek_key_code_p,
    zfw_ek_key_code_q,
    zfw_ek_key_code_r,
    zfw_ek_key_code_s,
    zfw_ek_key_code_t,
    zfw_ek_key_code_u,
    zfw_ek_key_code_v,
    zfw_ek_key_code_w,
    zfw_ek_key_code_x,
    zfw_ek_key_code_y,
    zfw_ek_key_code_z,
    zfw_ek_key_code_escape,
    zfw_ek_key_code_enter,
    zfw_ek_key_code_backspace,
    zfw_ek_key_code_tab,
    zfw_ek_key_code_right,
    zfw_ek_key_code_left,
    zfw_ek_key_code_down,
    zfw_ek_key_code_up,
    zfw_ek_key_code_f1,
    zfw_ek_key_code_f2,
    zfw_ek_key_code_f3,
    zfw_ek_key_code_f4,
    zfw_ek_key_code_f5,
    zfw_ek_key_code_f6,
    zfw_ek_key_code_f7,
    zfw_ek_key_code_f8,
    zfw_ek_key_code_f9,
    zfw_ek_key_code_f10,
    zfw_ek_key_code_f11,
    zfw_ek_key_code_f12,
    zfw_ek_key_code_left_shift,
    zfw_ek_key_code_left_control,
    zfw_ek_key_code_left_alt,
    zfw_ek_key_code_right_shift,
    zfw_ek_key_code_right_control,
    zfw_ek_key_code_right_alt,

    zfw_eks_key_code_cnt
} zfw_e_key_code;

static_assert(zfw_eks_key_code_cnt < ZFW_SIZE_IN_BITS(zfw_t_keys_down_bits), "Too many key codes!");

typedef enum {
    zfw_ek_mouse_button_code_left,
    zfw_ek_mouse_button_code_right,
    zfw_ek_mouse_button_code_middle,

    zfw_eks_mouse_button_code_cnt
} zfw_e_mouse_button_code;

static_assert(zfw_eks_mouse_button_code_cnt < ZFW_SIZE_IN_BITS(zfw_t_mouse_buttons_down_bits), "Too many mouse button codes!");

typedef struct {
    zfw_t_keys_down_bits keys_down;
    zfw_t_mouse_buttons_down_bits mouse_buttons_down;
    zfw_s_vec_2d mouse_pos;
    zfw_e_mouse_scroll_state mouse_scroll_state;
} zfw_s_input_state;

typedef struct {
    zfw_s_vec_2d_i pos;
    zfw_s_vec_2d_i size;
    bool fullscreen;
} zfw_s_window_state;

typedef struct {
    void* user_mem;
    zfw_s_mem_arena* perm_mem_arena;
    zfw_s_mem_arena* temp_mem_arena;
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
    zfw_s_mem_arena* perm_mem_arena;
    zfw_s_mem_arena* temp_mem_arena;
    zfw_s_window_state window_state;
    const zfw_s_input_state* input_state;
    const zfw_s_input_state* input_state_last;
    const zfw_t_unicode_buf* unicode_buf;
    zfw_s_audio_sys* audio_sys;
} zfw_s_game_tick_func_data;

typedef struct {
    void* user_mem;
    zfw_s_mem_arena* perm_mem_arena;
    zfw_s_mem_arena* temp_mem_arena;
    zfw_s_vec_2d mouse_pos;
    zfw_s_rendering_context rendering_context;
} zfw_s_game_render_func_data;

typedef struct {
    size_t user_mem_size; // How much memory should be allocated in the permanent arena for your use? This might be the size of a specific struct, for example.
    size_t user_mem_alignment; // The alignment of the above memory.

    zfw_s_vec_2d_i window_init_size;
    const char* window_title;
    zfw_e_window_flags window_flags;

    // Below are pointers to functions that the framework will call for you. The provided struct pointers expose parts of the framework state for you to work with.
    bool (*init_func)(const zfw_s_game_init_func_data* const func_data); // Called as one of the last steps of the game initialisation phase.
    zfw_e_game_tick_func_result (*tick_func)(const zfw_s_game_tick_func_data* const func_data); // Called once every tick.
    bool (*render_func)(const zfw_s_game_render_func_data* const func_data); // Called after a tick.
    void (*clean_func)(void* const user_mem); // Called when the game ends (including if it ends in error). This is not called if the initialisation function failed or was not yet called.
} zfw_s_game_info;

bool ZFWRunGame(const zfw_s_game_info* const info);

static inline bool ZFWIsKeyDown(const zfw_e_key_code kc, const zfw_s_input_state* const input_state) {
    return (input_state->keys_down & ((zfw_t_keys_down_bits)1 << kc)) != 0;
}

static inline bool ZFWIsKeyPressed(const zfw_e_key_code kc, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last) {
    return ZFWIsKeyDown(kc, input_state) && !ZFWIsKeyDown(kc, input_state_last);
}

static inline bool ZFWIsKeyReleased(const zfw_e_key_code kc, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last) {
    return !ZFWIsKeyDown(kc, input_state) && ZFWIsKeyDown(kc, input_state_last);
}

static inline bool ZFWIsMouseButtonDown(const zfw_e_mouse_button_code mbc, const zfw_s_input_state* const input_state) {
    return (input_state->mouse_buttons_down & ((zfw_t_mouse_buttons_down_bits)1 << mbc)) != 0;
}

static inline bool ZFWIsMouseButtonPressed(const zfw_e_mouse_button_code mbc, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last) {
    return ZFWIsMouseButtonDown(mbc, input_state) && !ZFWIsMouseButtonDown(mbc, input_state_last);
}

static inline bool ZFWIsMouseButtonReleased(const zfw_e_mouse_button_code mbc, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last) {
    return !ZFWIsMouseButtonDown(mbc, input_state) && ZFWIsMouseButtonDown(mbc, input_state_last);
}

#endif
