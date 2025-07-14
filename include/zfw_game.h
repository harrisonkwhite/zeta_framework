#ifndef ZFW_GAME_H
#define ZFW_GAME_H

#include <stdbool.h>
#include <stdint.h>
#include <GLFW/glfw3.h>
#include "zfw_math.h"
#include "zfw_rendering.h"
#include "zfw_audio.h"
#include "zfw_utils.h"

typedef char t_unicode_buf[32];

typedef uint64_t t_keys_down_bits;
typedef uint8_t t_mouse_buttons_down_bits;

typedef enum {
    ek_window_flags_resizable = 1 << 0,
    ek_window_flags_hide_cursor = 1 << 1
} e_window_flags;

typedef enum {
    ek_mouse_scroll_state_none,
    ek_mouse_scroll_state_down,
    ek_mouse_scroll_state_up
} e_mouse_scroll_state;

typedef enum {
    ek_key_code_space,
    ek_key_code_0,
    ek_key_code_1,
    ek_key_code_2,
    ek_key_code_3,
    ek_key_code_4,
    ek_key_code_5,
    ek_key_code_6,
    ek_key_code_7,
    ek_key_code_8,
    ek_key_code_9,
    ek_key_code_a,
    ek_key_code_b,
    ek_key_code_c,
    ek_key_code_d,
    ek_key_code_e,
    ek_key_code_f,
    ek_key_code_g,
    ek_key_code_h,
    ek_key_code_i,
    ek_key_code_j,
    ek_key_code_k,
    ek_key_code_l,
    ek_key_code_m,
    ek_key_code_n,
    ek_key_code_o,
    ek_key_code_p,
    ek_key_code_q,
    ek_key_code_r,
    ek_key_code_s,
    ek_key_code_t,
    ek_key_code_u,
    ek_key_code_v,
    ek_key_code_w,
    ek_key_code_x,
    ek_key_code_y,
    ek_key_code_z,
    ek_key_code_escape,
    ek_key_code_enter,
    ek_key_code_backspace,
    ek_key_code_tab,
    ek_key_code_right,
    ek_key_code_left,
    ek_key_code_down,
    ek_key_code_up,
    ek_key_code_f1,
    ek_key_code_f2,
    ek_key_code_f3,
    ek_key_code_f4,
    ek_key_code_f5,
    ek_key_code_f6,
    ek_key_code_f7,
    ek_key_code_f8,
    ek_key_code_f9,
    ek_key_code_f10,
    ek_key_code_f11,
    ek_key_code_f12,
    ek_key_code_left_shift,
    ek_key_code_left_control,
    ek_key_code_left_alt,
    ek_key_code_right_shift,
    ek_key_code_right_control,
    ek_key_code_right_alt,

    eks_key_code_cnt
} e_key_code;

static_assert(eks_key_code_cnt < BYTES_TO_BITS(sizeof(t_keys_down_bits)), "Too many key codes!");

typedef enum {
    ek_mouse_button_code_left,
    ek_mouse_button_code_right,
    ek_mouse_button_code_middle,

    eks_mouse_button_code_cnt
} e_mouse_button_code;

static_assert(eks_mouse_button_code_cnt < BYTES_TO_BITS(sizeof(t_mouse_buttons_down_bits)), "Too many mouse button codes!");

typedef struct {
    t_keys_down_bits keys_down;
    t_mouse_buttons_down_bits mouse_buttons_down;
    s_vec_2d mouse_pos;
    e_mouse_scroll_state mouse_scroll_state;
} s_input_state;

typedef struct {
    s_vec_2d_i pos;
    s_vec_2d_i size;
    bool fullscreen;
} s_window_state;

typedef struct {
    void* user_mem;
    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;
    s_window_state window_state;
    s_audio_sys* audio_sys;
} s_game_init_func_data;

typedef enum {
    ek_game_tick_func_result_default,
    ek_game_tick_func_result_exit,
    ek_game_tick_func_result_error
} e_game_tick_func_result;

typedef struct {
    void* user_mem;
    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;
    s_window_state window_state;
    const s_input_state* input_state;
    const s_input_state* input_state_last;
    const t_unicode_buf* unicode_buf;
    s_audio_sys* audio_sys;
} s_game_tick_func_data;

typedef struct {
    void* user_mem;
    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;
    s_rendering_context rendering_context;
    const s_input_state* input_state;
    const s_input_state* input_state_last;
} s_game_render_func_data;

typedef struct {
    int user_mem_size; // How much memory should be allocated in the permanent arena for your use? This might be the size of a specific struct, for example.
    int user_mem_alignment; // The alignment of the above memory.

    s_vec_2d_i window_init_size;
    const char* window_title;
    e_window_flags window_flags;

    // Below are pointers to functions that the framework will call for you. The provided struct pointers expose parts of the framework state for you to work with.
    bool (*init_func)(const s_game_init_func_data* const func_data); // Called as one of the last steps of the game initialisation phase.
    e_game_tick_func_result (*tick_func)(const s_game_tick_func_data* const func_data); // Called once every tick.
    bool (*render_func)(const s_game_render_func_data* const func_data); // Called after a tick.
    void (*clean_func)(void* const user_mem); // Called when the game ends (including if it ends in error). This is not called if the initialisation function failed or was not yet called.
} s_game_info;

bool RunGame(const s_game_info* info);

static inline bool IsKeyDown(const e_key_code kc, const s_input_state* const input_state) {
    return (input_state->keys_down & (1ULL << kc)) != 0;
}

static inline bool IsKeyPressed(const e_key_code kc, const s_input_state* const input_state, const s_input_state* const input_state_last) {
    return IsKeyDown(kc, input_state) && !IsKeyDown(kc, input_state_last);
}

static inline bool IsKeyReleased(const e_key_code kc, const s_input_state* const input_state, const s_input_state* const input_state_last) {
    return !IsKeyDown(kc, input_state) && IsKeyDown(kc, input_state_last);
}

static inline bool IsMouseButtonDown(const e_mouse_button_code mbc, const s_input_state* const input_state) {
    return (input_state->mouse_buttons_down & (1U << mbc)) != 0;
}

static inline bool IsMouseButtonPressed(const e_mouse_button_code mbc, const s_input_state* const input_state, const s_input_state* const input_state_last) {
    return IsMouseButtonDown(mbc, input_state) && !IsMouseButtonDown(mbc, input_state_last);
}

static inline bool IsMouseButtonReleased(const e_mouse_button_code mbc, const s_input_state* const input_state, const s_input_state* const input_state_last) {
    return !IsMouseButtonDown(mbc, input_state) && IsMouseButtonDown(mbc, input_state_last);
}

#endif
