#include "zfw_game.h"

#include <stdio.h>
#include <GLFW/glfw3.h>
#include "zfw_audio.h"
#include "zfw_graphics.h"
#include "zfw_mem.h"
#include "zfw_random.h"
#include "zfw_io.h"

#define PERM_MEM_ARENA_SIZE ZFW_MEGABYTES(80)
#define TEMP_MEM_ARENA_SIZE ZFW_MEGABYTES(40)

#define TARG_TICKS_PER_SEC 60
#define TARG_TICK_INTERVAL (1.0 / (TARG_TICKS_PER_SEC))

static const int g_glfw_keys[] = {
    [zfw_ek_key_code_space] = GLFW_KEY_SPACE,
    [zfw_ek_key_code_0] = GLFW_KEY_0,
    [zfw_ek_key_code_1] = GLFW_KEY_1,
    [zfw_ek_key_code_2] = GLFW_KEY_2,
    [zfw_ek_key_code_3] = GLFW_KEY_3,
    [zfw_ek_key_code_4] = GLFW_KEY_4,
    [zfw_ek_key_code_5] = GLFW_KEY_5,
    [zfw_ek_key_code_6] = GLFW_KEY_6,
    [zfw_ek_key_code_7] = GLFW_KEY_7,
    [zfw_ek_key_code_8] = GLFW_KEY_8,
    [zfw_ek_key_code_9] = GLFW_KEY_9,
    [zfw_ek_key_code_a] = GLFW_KEY_A,
    [zfw_ek_key_code_b] = GLFW_KEY_B,
    [zfw_ek_key_code_c] = GLFW_KEY_C,
    [zfw_ek_key_code_d] = GLFW_KEY_D,
    [zfw_ek_key_code_e] = GLFW_KEY_E,
    [zfw_ek_key_code_f] = GLFW_KEY_F,
    [zfw_ek_key_code_g] = GLFW_KEY_G,
    [zfw_ek_key_code_h] = GLFW_KEY_H,
    [zfw_ek_key_code_i] = GLFW_KEY_I,
    [zfw_ek_key_code_j] = GLFW_KEY_J,
    [zfw_ek_key_code_k] = GLFW_KEY_K,
    [zfw_ek_key_code_l] = GLFW_KEY_L,
    [zfw_ek_key_code_m] = GLFW_KEY_M,
    [zfw_ek_key_code_n] = GLFW_KEY_N,
    [zfw_ek_key_code_o] = GLFW_KEY_O,
    [zfw_ek_key_code_p] = GLFW_KEY_P,
    [zfw_ek_key_code_q] = GLFW_KEY_Q,
    [zfw_ek_key_code_r] = GLFW_KEY_R,
    [zfw_ek_key_code_s] = GLFW_KEY_S,
    [zfw_ek_key_code_t] = GLFW_KEY_T,
    [zfw_ek_key_code_u] = GLFW_KEY_U,
    [zfw_ek_key_code_v] = GLFW_KEY_V,
    [zfw_ek_key_code_w] = GLFW_KEY_W,
    [zfw_ek_key_code_x] = GLFW_KEY_X,
    [zfw_ek_key_code_y] = GLFW_KEY_Y,
    [zfw_ek_key_code_z] = GLFW_KEY_Z,
    [zfw_ek_key_code_escape] = GLFW_KEY_ESCAPE,
    [zfw_ek_key_code_enter] = GLFW_KEY_ENTER,
    [zfw_ek_key_code_backspace] = GLFW_KEY_BACKSPACE,
    [zfw_ek_key_code_tab] = GLFW_KEY_TAB,
    [zfw_ek_key_code_right] = GLFW_KEY_RIGHT,
    [zfw_ek_key_code_left] = GLFW_KEY_LEFT,
    [zfw_ek_key_code_down] = GLFW_KEY_DOWN,
    [zfw_ek_key_code_up] = GLFW_KEY_UP,
    [zfw_ek_key_code_f1] = GLFW_KEY_F1,
    [zfw_ek_key_code_f2] = GLFW_KEY_F2,
    [zfw_ek_key_code_f3] = GLFW_KEY_F3,
    [zfw_ek_key_code_f4] = GLFW_KEY_F4,
    [zfw_ek_key_code_f5] = GLFW_KEY_F5,
    [zfw_ek_key_code_f6] = GLFW_KEY_F6,
    [zfw_ek_key_code_f7] = GLFW_KEY_F7,
    [zfw_ek_key_code_f8] = GLFW_KEY_F8,
    [zfw_ek_key_code_f9] = GLFW_KEY_F9,
    [zfw_ek_key_code_f10] = GLFW_KEY_F10,
    [zfw_ek_key_code_f11] = GLFW_KEY_F11,
    [zfw_ek_key_code_f12] = GLFW_KEY_F12,
    [zfw_ek_key_code_left_shift] = GLFW_KEY_LEFT_SHIFT,
    [zfw_ek_key_code_left_control] = GLFW_KEY_LEFT_CONTROL,
    [zfw_ek_key_code_left_alt] = GLFW_KEY_LEFT_ALT,
    [zfw_ek_key_code_right_shift] = GLFW_KEY_RIGHT_SHIFT,
    [zfw_ek_key_code_right_control] = GLFW_KEY_RIGHT_CONTROL,
    [zfw_ek_key_code_right_alt] = GLFW_KEY_RIGHT_ALT
};

ZFW_CHECK_STATIC_ARRAY_LEN(g_glfw_keys, zfw_eks_key_code_cnt);

static const int g_glfw_mouse_buttons[] = {
    [zfw_ek_mouse_button_code_left] = GLFW_MOUSE_BUTTON_LEFT,
    [zfw_ek_mouse_button_code_right] = GLFW_MOUSE_BUTTON_RIGHT,
    [zfw_ek_mouse_button_code_middle] = GLFW_MOUSE_BUTTON_MIDDLE
};

ZFW_CHECK_STATIC_ARRAY_LEN(g_glfw_mouse_buttons, zfw_eks_mouse_button_code_cnt);

// Data to be accessed in GLFW callbacks.
typedef struct {
    zfw_e_mouse_scroll_state mouse_scroll_state; // When mouse scroll is detected, this can be updated via callback. It can be reset after the next tick (so there is a chance for the ZFW user to detect the scroll).
    zfw_t_unicode_buf unicode_buf; // This is filled up as characters are typed, and zeroed out after the next tick. This way, the ZFW user can see everything that has been typed since the last tick.
} s_glfw_user_data;

static void AssertGameInfoValidity(const zfw_s_game_info* const info) {
    assert(info->user_mem_size >= 0);
    assert(info->user_mem_size == 0 || ZFW_IsValidAlignment(info->user_mem_alignment));

    assert(info->window_title);
    assert(info->window_init_size.x > 0 && info->window_init_size.y > 0);

    assert(info->init_func);
    assert(info->tick_func);
    assert(info->render_func);
}

static zfw_s_window_state WindowState(GLFWwindow* const glfw_window) {
    zfw_s_window_state state = {
        .fullscreen = glfwGetWindowMonitor(glfw_window) != NULL
    };
    glfwGetWindowPos(glfw_window, &state.pos.x, &state.pos.y);
    glfwGetWindowSize(glfw_window, &state.size.x, &state.size.y);

    return state;
}

static void RefreshInputState(zfw_s_input_state* const state, GLFWwindow* const glfw_window, const zfw_e_mouse_scroll_state mouse_scroll_state) {
    ZFW_ZERO_OUT(*state);

    for (int i = 0; i < zfw_eks_key_code_cnt; i++) {
        if (glfwGetKey(glfw_window, g_glfw_keys[i])) {
            state->keys_down |= (zfw_t_keys_down_bits)1 << i;
        }
    }

    for (int i = 0; i < zfw_eks_mouse_button_code_cnt; i++) {
        if (glfwGetMouseButton(glfw_window, g_glfw_mouse_buttons[i])) {
            state->mouse_buttons_down |= (zfw_t_mouse_buttons_down_bits)1 << i;
        }
    }

    double mouse_x_dbl, mouse_y_dbl;
    glfwGetCursorPos(glfw_window, &mouse_x_dbl, &mouse_y_dbl);
    state->mouse_pos = (zfw_s_vec_2d){mouse_x_dbl, mouse_y_dbl};

    state->mouse_scroll_state = mouse_scroll_state;
}

static void GLFWScrollCallback(GLFWwindow* const window, const double offs_x, const double offs_y) {
    s_glfw_user_data* const user_data = glfwGetWindowUserPointer(window);

    if (offs_y > 0.0) {
        user_data->mouse_scroll_state = zfw_ek_mouse_scroll_state_up;
    } else if (offs_y < 0.0) {
        user_data->mouse_scroll_state = zfw_ek_mouse_scroll_state_down;
    } else {
        user_data->mouse_scroll_state = zfw_ek_mouse_scroll_state_none;
    }
}

static void GLFWCharCallback(GLFWwindow* const window, const unsigned int codepoint) {
    s_glfw_user_data* const user_data = glfwGetWindowUserPointer(window);

    for (int i = 0; i < sizeof(user_data->unicode_buf); i++) {
        if (!user_data->unicode_buf[i]) {
            user_data->unicode_buf[i] = (char)codepoint;
            return;
        }
    }

    ZFW_LogError("Unicode buffer is full!");
}

static GLFWwindow* CreateGLFWWindow(const zfw_s_vec_2d_i size, const char* const title, const zfw_e_window_flags flags, s_glfw_user_data* const user_data) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ZFW_GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ZFW_GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* const glfw_window = glfwCreateWindow(size.x, size.y, title, NULL, NULL);

    if (glfw_window) {
        glfwSetWindowAttrib(glfw_window, GLFW_RESIZABLE, flags & zfw_ek_window_flags_resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwSetInputMode(glfw_window, GLFW_CURSOR, flags & zfw_ek_window_flags_hide_cursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

        glfwMakeContextCurrent(glfw_window);

        glfwSwapInterval(1); // Enables VSync.

        glfwSetWindowUserPointer(glfw_window, user_data);

        glfwSetScrollCallback(glfw_window, GLFWScrollCallback);
        glfwSetCharCallback(glfw_window, GLFWCharCallback);
    } else {
        ZFW_LogError("Failed to create a GLFW window!");
    }

    return glfw_window;
}

static void ResizeGLViewportIfDifferent(const zfw_s_vec_2d_i size) {
    assert(size.x > 0 && size.y > 0);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    if (viewport[0] != 0 || viewport[1] != 0 || viewport[2] != size.x || viewport[3] != size.y) {
        glViewport(0, 0, size.x, size.y);
    }
}

bool ZFW_RunGame(const zfw_s_game_info* const info) {
    assert(info);
    AssertGameInfoValidity(info);

    bool error = false;

    //
    // Initialisation
    //
    ZFW_Log("Initialising...");

    ZFW_InitRNG();

    // Initialise memory arenas.
    zfw_s_mem_arena perm_mem_arena = {0}; // The memory in here exists for the lifetime of the program, it does not get reset.

    if (!ZFW_InitMemArena(&perm_mem_arena, PERM_MEM_ARENA_SIZE)) {
        ZFW_LogError("Failed to initialise the permanent memory arena!");
        error = true;
        goto clean_nothing;
    }

    zfw_s_mem_arena temp_mem_arena = {0}; // While the memory here also exists for the program lifetime, it gets reset after game initialisation and after every frame. Useful if you just need some temporary working space.

    if (!ZFW_InitMemArena(&temp_mem_arena, TEMP_MEM_ARENA_SIZE)) {
        ZFW_LogError("Failed to initialise the temporary memory arena!");
        error = true;
        goto clean_perm_mem_arena;
    }

    // Initialise GLFW.
    if (!glfwInit()) {
        ZFW_LogError("Failed to initialise GLFW!");
        error = true;
        goto clean_temp_mem_arena;
    }

    s_glfw_user_data glfw_user_data = {0}; // This is accessible from the GLFW callbacks.
    GLFWwindow* const glfw_window = CreateGLFWWindow(info->window_init_size, info->window_title, info->window_flags, &glfw_user_data);

    if (!glfw_window) {
        error = true;
        goto clean_glfw;
    }

    // Initialise OpenGL rendering.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        ZFW_LogError("Failed to load OpenGL function pointers!");
        error = true;
        goto clean_glfw_window;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    zfw_s_rendering_basis rendering_basis = {0};

    if (!ZFW_InitRenderingBasis(&rendering_basis, &temp_mem_arena)) {
        error = true;
        goto clean_glfw_window;
    }

    zfw_s_rendering_state* const rendering_state = ZFW_MEM_ARENA_PUSH_TYPE(&perm_mem_arena, zfw_s_rendering_state);

    if (!rendering_state) {
        error = true;
        goto clean_rendering_basis;
    }

    // Initialise audio system.
    zfw_s_audio_sys audio_sys = {0};

    if (!ZFW_InitAudioSys(&audio_sys)) {
        error = true;
        goto clean_rendering_basis;
    }

    // Initialise user memory.
    void* user_mem = NULL;

    if (info->user_mem_size > 0) {
        user_mem = ZFW_PushToMemArena(&perm_mem_arena, info->user_mem_size, info->user_mem_alignment);

        if (!user_mem) {
            ZFW_LogError("Failed to allocate user memory!");
            error = true;
            goto clean_audio_sys;
        }
    }

    // Run the user-defined game initialisation function.
    {
        const zfw_s_game_init_func_data func_data = {
            .user_mem = user_mem,
            .perm_mem_arena = &perm_mem_arena,
            .temp_mem_arena = &temp_mem_arena,
            .window_state = WindowState(glfw_window),
            .audio_sys = &audio_sys
        };

        if (!info->init_func(&func_data)) {
            ZFW_LogError("Provided game initialisation function failed!");
            error = true;
            goto clean_audio_sys;
        }
    }

    //
    // Main Loop
    //
    glfwShowWindow(glfw_window);

    zfw_s_input_state input_state = {0};

    double frame_time_last = glfwGetTime();
    double frame_dur_accum = 0.0;

    bool running = true;

    ZFW_Log("Entering the main loop...");

    while (!glfwWindowShouldClose(glfw_window) && running) {
        glfwPollEvents();

        ZFW_RewindMemArena(&temp_mem_arena, 0);

        const zfw_s_window_state window_state = WindowState(glfw_window);

        ResizeGLViewportIfDifferent(window_state.size);

        const double frame_time = glfwGetTime();
        frame_dur_accum += frame_time - frame_time_last; // Update accumulator with delta time.
        frame_time_last = frame_time;

        // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick and update the display.
        if (frame_dur_accum >= TARG_TICK_INTERVAL) {
            const zfw_s_input_state input_state_last = input_state;
            RefreshInputState(&input_state, glfw_window, glfw_user_data.mouse_scroll_state);

            ZFW_UpdateAudioSys(&audio_sys);

            // Run ticks.
            do {
                // Execute the user-defined tick function.
                const zfw_s_game_tick_func_data func_data = {
                    .user_mem = user_mem,
                    .perm_mem_arena = &perm_mem_arena,
                    .temp_mem_arena = &temp_mem_arena,
                    .window_state = window_state,
                    .input_state = &input_state,
                    .input_state_last = &input_state_last,
                    .unicode_buf = &glfw_user_data.unicode_buf,
                    .audio_sys = &audio_sys
                };

                const zfw_e_game_tick_func_result res = info->tick_func(&func_data);

                ZFW_ZERO_OUT(glfw_user_data.unicode_buf);
                glfw_user_data.mouse_scroll_state = zfw_ek_mouse_scroll_state_none;

                if (res == ek_game_tick_func_result_exit) {
                    running = false;
                }

                if (res == ek_game_tick_func_result_error) {
                    running = false;
                    error = true;
                }

                frame_dur_accum -= TARG_TICK_INTERVAL;
            } while (frame_dur_accum >= TARG_TICK_INTERVAL);

            // Update the display.
            ZFW_ZERO_OUT(*rendering_state);
            ZFW_InitRenderingState(rendering_state);

            {
                // Execute the user-defined render function.
                const zfw_s_game_render_func_data func_data = {
                    .user_mem = user_mem,
                    .perm_mem_arena = &perm_mem_arena,
                    .temp_mem_arena = &temp_mem_arena,
                    .mouse_pos = input_state.mouse_pos,
                    .rendering_context = {
                        .basis = &rendering_basis,
                        .state = rendering_state,
                        .window_size = window_state.size
                    }
                };

                if (!info->render_func(&func_data)) {
                    running = false;
                    error = true;
                }

                assert(rendering_state->batch.num_slots_used == 0 && "User-defined rendering function completed, but not everything has been flushed!");
            }

            glfwSwapBuffers(glfw_window);
        }
    }

    if (info->clean_func) {
        info->clean_func(user_mem);
    }

clean_audio_sys:
    ZFW_CleanAudioSys(&audio_sys);

clean_rendering_basis:
    ZFW_CleanRenderingBasis(&rendering_basis);

clean_glfw_window:
    glfwDestroyWindow(glfw_window);

clean_glfw:
    glfwTerminate();

clean_temp_mem_arena:
    ZFW_CleanMemArena(&temp_mem_arena);

clean_perm_mem_arena:
    ZFW_CleanMemArena(&perm_mem_arena);

clean_nothing:

    return !error;
}
