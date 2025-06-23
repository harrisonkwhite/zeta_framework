#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include "zfw_game.h"
#include "zfw_rendering.h"
#include "zfw_utils.h"
#include "zfw_random.h"

#define PERM_MEM_ARENA_SIZE ((1 << 20) * 80)
#define TEMP_MEM_ARENA_SIZE ((1 << 20) * 40)

#define TARG_TICKS_PER_SEC 60
#define TARG_TICK_INTERVAL (1.0 / TARG_TICKS_PER_SEC)

#define GL_VERSION_MAJOR 4
#define GL_VERSION_MINOR 3

static const int g_glfw_keys[eks_key_code_cnt] = {
    [ek_key_code_space] = GLFW_KEY_SPACE,
    [ek_key_code_0] = GLFW_KEY_0,
    [ek_key_code_1] = GLFW_KEY_1,
    [ek_key_code_2] = GLFW_KEY_2,
    [ek_key_code_3] = GLFW_KEY_3,
    [ek_key_code_4] = GLFW_KEY_4,
    [ek_key_code_5] = GLFW_KEY_5,
    [ek_key_code_6] = GLFW_KEY_6,
    [ek_key_code_7] = GLFW_KEY_7,
    [ek_key_code_8] = GLFW_KEY_8,
    [ek_key_code_9] = GLFW_KEY_9,
    [ek_key_code_a] = GLFW_KEY_A,
    [ek_key_code_b] = GLFW_KEY_B,
    [ek_key_code_c] = GLFW_KEY_C,
    [ek_key_code_d] = GLFW_KEY_D,
    [ek_key_code_e] = GLFW_KEY_E,
    [ek_key_code_f] = GLFW_KEY_F,
    [ek_key_code_g] = GLFW_KEY_G,
    [ek_key_code_h] = GLFW_KEY_H,
    [ek_key_code_i] = GLFW_KEY_I,
    [ek_key_code_j] = GLFW_KEY_J,
    [ek_key_code_k] = GLFW_KEY_K,
    [ek_key_code_l] = GLFW_KEY_L,
    [ek_key_code_m] = GLFW_KEY_M,
    [ek_key_code_n] = GLFW_KEY_N,
    [ek_key_code_o] = GLFW_KEY_O,
    [ek_key_code_p] = GLFW_KEY_P,
    [ek_key_code_q] = GLFW_KEY_Q,
    [ek_key_code_r] = GLFW_KEY_R,
    [ek_key_code_s] = GLFW_KEY_S,
    [ek_key_code_t] = GLFW_KEY_T,
    [ek_key_code_u] = GLFW_KEY_U,
    [ek_key_code_v] = GLFW_KEY_V,
    [ek_key_code_w] = GLFW_KEY_W,
    [ek_key_code_x] = GLFW_KEY_X,
    [ek_key_code_y] = GLFW_KEY_Y,
    [ek_key_code_z] = GLFW_KEY_Z,
    [ek_key_code_escape] = GLFW_KEY_ESCAPE,
    [ek_key_code_enter] = GLFW_KEY_ENTER,
    [ek_key_code_tab] = GLFW_KEY_TAB,
    [ek_key_code_right] = GLFW_KEY_RIGHT,
    [ek_key_code_left] = GLFW_KEY_LEFT,
    [ek_key_code_down] = GLFW_KEY_DOWN,
    [ek_key_code_up] = GLFW_KEY_UP,
    [ek_key_code_f1] = GLFW_KEY_F1,
    [ek_key_code_f2] = GLFW_KEY_F2,
    [ek_key_code_f3] = GLFW_KEY_F3,
    [ek_key_code_f4] = GLFW_KEY_F4,
    [ek_key_code_f5] = GLFW_KEY_F5,
    [ek_key_code_f6] = GLFW_KEY_F6,
    [ek_key_code_f7] = GLFW_KEY_F7,
    [ek_key_code_f8] = GLFW_KEY_F8,
    [ek_key_code_f9] = GLFW_KEY_F9,
    [ek_key_code_f10] = GLFW_KEY_F10,
    [ek_key_code_f11] = GLFW_KEY_F11,
    [ek_key_code_f12] = GLFW_KEY_F12,
    [ek_key_code_left_shift] = GLFW_KEY_LEFT_SHIFT,
    [ek_key_code_left_control] = GLFW_KEY_LEFT_CONTROL,
    [ek_key_code_left_alt] = GLFW_KEY_LEFT_ALT,
    [ek_key_code_right_shift] = GLFW_KEY_RIGHT_SHIFT,
    [ek_key_code_right_control] = GLFW_KEY_RIGHT_CONTROL,
    [ek_key_code_right_alt] = GLFW_KEY_RIGHT_ALT
};

static const int g_glfw_mouse_buttons[eks_mouse_button_code_cnt] = {
    [ek_mouse_button_code_left] = GLFW_MOUSE_BUTTON_LEFT,
    [ek_mouse_button_code_right] = GLFW_MOUSE_BUTTON_RIGHT,
    [ek_mouse_button_code_middle] = GLFW_MOUSE_BUTTON_MIDDLE
};

typedef struct {
    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;
    GLFWwindow* glfw_window;
    s_pers_render_data* pers_render_data;
} s_game_cleanup_info;

static void AssertGameInfoValidity(const s_game_info* const info) {
    assert(info->user_mem_size >= 0);
    assert(info->user_mem_alignment >= 0);

    assert(info->window_title);
    assert(info->window_init_size.x > 0 && info->window_init_size.y > 0);

    assert(info->init_func);
    assert(info->tick_func);
    assert(info->render_func);
}

static void CleanGame(const s_game_cleanup_info* const cleanup_info) {
    if (cleanup_info->glfw_window) {
        glfwDestroyWindow(cleanup_info->glfw_window);
    }

    glfwTerminate();

    CleanMemArena(cleanup_info->temp_mem_arena);
    CleanMemArena(cleanup_info->perm_mem_arena);
}

static s_window_state GetWindowState(GLFWwindow* const glfw_window) {
    assert(glfw_window);

    s_window_state state = {
        .fullscreen = glfwGetWindowMonitor(glfw_window) != NULL
    };
    glfwGetWindowPos(glfw_window, &state.pos.x, &state.pos.y);
    glfwGetWindowSize(glfw_window, &state.size.x, &state.size.y);

    return state;
}

static void RefreshInputState(s_input_state* const state, GLFWwindow* const glfw_window, const e_mouse_scroll_state mouse_scroll_state) {
    assert(state);
    assert(glfw_window);

    ZeroOut(state, sizeof(*state));

    for (int i = 0; i < eks_key_code_cnt; i++) {
        if (glfwGetKey(glfw_window, g_glfw_keys[i])) {
            state->keys_down |= (t_keys_down_bits)1 << i;
        }
    }

    for (int i = 0; i < eks_mouse_button_code_cnt; i++) {
        if (glfwGetMouseButton(glfw_window, g_glfw_mouse_buttons[i])) {
            state->mouse_buttons_down |= (t_mouse_buttons_down_bits)1 << i;
        }
    }

    double mouse_x_dbl, mouse_y_dbl;
    glfwGetCursorPos(glfw_window, &mouse_x_dbl, &mouse_y_dbl);
    state->mouse_pos = (s_vec_2d){mouse_x_dbl, mouse_y_dbl};

    state->mouse_scroll = mouse_scroll_state;
}

static void GLFWScrollCallback(GLFWwindow* const window, const double offs_x, const double offs_y) {
    e_mouse_scroll_state* const scroll_state = glfwGetWindowUserPointer(window);

    if (offs_y > 0.0) {
        *scroll_state = ek_mouse_scroll_state_up;
    } else if (offs_y < 0.0) {
        *scroll_state = ek_mouse_scroll_state_down;
    } else {
        *scroll_state = ek_mouse_scroll_state_none;
    }
}

bool RunGame(const s_game_info* const info) {
    assert(info);
    AssertGameInfoValidity(info);

    s_game_cleanup_info cleanup_info = {0};

    //
    // Initialisation
    //
    printf("Initialising...\n");

    InitRNG();

    s_mem_arena perm_mem_arena = {0};

    if (!InitMemArena(&perm_mem_arena, PERM_MEM_ARENA_SIZE)) {
        fprintf(stderr, "Failed to initialise the permanent memory arena!\n");
        CleanGame(&cleanup_info);
        return false;
    }

    cleanup_info.perm_mem_arena = &perm_mem_arena;

    s_mem_arena temp_mem_arena = {0};

    if (!InitMemArena(&temp_mem_arena, TEMP_MEM_ARENA_SIZE)) {
        fprintf(stderr, "Failed to initialise the temporary memory arena!\n");
        CleanGame(&cleanup_info);
        return false;
    }

    cleanup_info.temp_mem_arena = &temp_mem_arena;

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialise GLFW!\n");
        CleanGame(&cleanup_info);
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* const glfw_window = glfwCreateWindow(
        info->window_init_size.x,
        info->window_init_size.y,
        info->window_title,
        NULL,
        NULL
    );

    if (!glfw_window) {
        fprintf(stderr, "Failed to create a GLFW window!\n");
        CleanGame(&cleanup_info);
        return false;
    }

    cleanup_info.glfw_window = glfw_window;

    glfwMakeContextCurrent(glfw_window);

    glfwSwapInterval(1); // Enables VSync.

    glfwSetWindowAttrib(glfw_window, GLFW_RESIZABLE, info->window_flags & ek_window_flag_resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwSetInputMode(glfw_window, GLFW_CURSOR, info->window_flags & ek_window_flag_hide_cursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

    s_input_state input_state = {0};
    e_mouse_scroll_state mouse_scroll_state = ek_mouse_scroll_state_none;
    glfwSetWindowUserPointer(glfw_window, &mouse_scroll_state);
    glfwSetScrollCallback(glfw_window, GLFWScrollCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to load OpenGL function pointers!\n");
        return false;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    s_pers_render_data pers_render_data = {0};
    InitPersRenderData(&pers_render_data, info->window_init_size);

    s_rendering_state* const rendering_state = MEM_ARENA_PUSH_TYPE(&perm_mem_arena, s_rendering_state);

    if (!rendering_state) {
        return false;
    }

    void* const user_mem = PushToMemArena(&perm_mem_arena, info->user_mem_size, info->user_mem_alignment);

    if (!user_mem) {
        fprintf(stderr, "Failed to allocate user memory!\n");
        return false;
    }

    {
        const s_game_init_func_data func_data = {
            .user_mem = user_mem,
            .perm_mem_arena = &perm_mem_arena,
            .temp_mem_arena = &temp_mem_arena,
            .window_state = GetWindowState(glfw_window)
        };

        if (!info->init_func(&func_data)) {
            fprintf(stderr, "Provided game initialisation function failed!\n");
            CleanGame(&cleanup_info);
            return false;
        }
    }

    glfwShowWindow(glfw_window);

    //
    // Main Loop
    //
    double frame_time_last = glfwGetTime();
    double frame_dur_accum = 0.0;

    printf("Entering the main loop...\n");

    while (!glfwWindowShouldClose(glfw_window)) {
        const s_window_state window_state_at_frame_begin = GetWindowState(glfw_window);

        const double frame_time = glfwGetTime();
        frame_dur_accum += frame_time - frame_time_last;
        frame_time_last = frame_time;

        if (frame_dur_accum >= TARG_TICK_INTERVAL) {
            const s_input_state input_state_last = input_state;
            RefreshInputState(&input_state, glfw_window, mouse_scroll_state);

            {
                const s_game_tick_func_data func_data = {
                    .user_mem = user_mem,
                    .perm_mem_arena = &perm_mem_arena,
                    .temp_mem_arena = &temp_mem_arena,
                    .window_state = window_state_at_frame_begin,
                    .input_state = &input_state,
                    .input_state_last = &input_state_last
                };

                if (!info->tick_func(&func_data)) {
                    CleanGame(&cleanup_info);
                    return false;
                }
            }

            BeginRendering(rendering_state);

            {
                const s_game_render_func_data func_data = {
                    .user_mem = user_mem,
                    .perm_mem_arena = &perm_mem_arena,
                    .temp_mem_arena = &temp_mem_arena,
                    .rendering_context = {
                        .pers = &pers_render_data,
                        .state = rendering_state,
                        .display_size = window_state_at_frame_begin.size
                    },
                    .input_state = &input_state,
                    .input_state_last = &input_state_last
                };

                if (!info->render_func(&func_data)) {
                    CleanGame(&cleanup_info);
                    return false;
                }

                ResetMemArena(&temp_mem_arena);
            }

            assert(rendering_state->batch_slots_used_cnt == 0); // Make sure that we flushed.

            glfwSwapBuffers(glfw_window);

            frame_dur_accum = 0;
        }

        glfwPollEvents();

        // Handle any window state changes.
        const s_window_state window_state_after_poll_events = GetWindowState(glfw_window);

        if (!Vec2DIsEqual(window_state_after_poll_events.size, VEC_2D_I_ZERO)
            && !Vec2DIsEqual(window_state_after_poll_events.size, window_state_at_frame_begin.size)) {
            glViewport(0, 0, window_state_after_poll_events.size.x, window_state_after_poll_events.size.y);

            if (!ResizeRenderSurfaces(&pers_render_data.surfs, window_state_after_poll_events.size)) {
                fprintf(stderr, "Failed to resize render surfaces!\n");
                return false;
            }
        }
    }

    CleanGame(&cleanup_info);

    return true;
}

bool IsKeyDown(const e_key_code kc, const s_input_state* input_state) {
    return (input_state->keys_down & (1ULL << kc)) != 0;
}

bool IsKeyPressed(e_key_code key_code, const s_input_state* st, const s_input_state* last) {
    return IsKeyDown(key_code, st) && !IsKeyDown(key_code, last);
}

bool IsKeyReleased(e_key_code key_code, const s_input_state* st, const s_input_state* last) {
    return !IsKeyDown(key_code, st) && IsKeyDown(key_code, last);
}

bool IsMouseButtonDown(e_mouse_button_code mbc, const s_input_state* st) {
    return (st->mouse_buttons_down & (1U << mbc)) != 0;
}

bool IsMouseButtonPressed(e_mouse_button_code mbc, const s_input_state* st, const s_input_state* last) {
    return IsMouseButtonDown(mbc, st) && !IsMouseButtonDown(mbc, last);
}

bool IsMouseButtonReleased(e_mouse_button_code mbc, const s_input_state* st, const s_input_state* last) {
    return !IsMouseButtonDown(mbc, st) && IsMouseButtonDown(mbc, last);
}
