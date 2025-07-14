#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include "zfw_game.h"
#include "zfw_random.h"
#include "zfw_utils.h"

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
    [ek_key_code_backspace] = GLFW_KEY_BACKSPACE,
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
    e_mouse_scroll_state mouse_scroll_state; // When mouse scroll is detected, this can be updated via callback. It can be reset after the next tick (so there is a chance for the ZFW user to detect the scroll).
    t_unicode_buf unicode_buf; // This is filled up as characters are typed, and zeroed out after the next tick. This way, the ZFW user can see everything that has been typed since the last tick.
} s_glfw_user_data;

typedef struct {
    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;
    GLFWwindow* glfw_window;
    s_pers_render_data* pers_render_data;
    s_audio_sys* audio_sys;
} s_game_cleanup_info;

static void AssertGameInfoValidity(const s_game_info* const info) {
    assert(info->user_mem_size >= 0);
    assert(info->user_mem_size == 0 || IsValidAlignment(info->user_mem_alignment));

    assert(info->window_title);
    assert(info->window_init_size.x > 0 && info->window_init_size.y > 0);

    assert(info->init_func);
    assert(info->tick_func);
    assert(info->render_func);
}

static void CleanGame(const s_game_cleanup_info* const cleanup_info) {
    if (cleanup_info->audio_sys) {
        CleanAudioSys(cleanup_info->audio_sys);
    }

    if (cleanup_info->pers_render_data) {
        CleanPersRenderData(cleanup_info->pers_render_data);
    }

    if (cleanup_info->glfw_window) {
        glfwDestroyWindow(cleanup_info->glfw_window);
    }

    glfwTerminate();

    CleanMemArena(cleanup_info->temp_mem_arena);
    CleanMemArena(cleanup_info->perm_mem_arena);
}

static s_window_state WindowState(GLFWwindow* const glfw_window) {
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

    ZERO_OUT(*state);

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

    state->mouse_scroll_state = mouse_scroll_state;
}

static void GLFWScrollCallback(GLFWwindow* const window, const double offs_x, const double offs_y) {
    s_glfw_user_data* const user_data = glfwGetWindowUserPointer(window);

    if (offs_y > 0.0) {
        user_data->mouse_scroll_state = ek_mouse_scroll_state_up;
    } else if (offs_y < 0.0) {
        user_data->mouse_scroll_state = ek_mouse_scroll_state_down;
    } else {
        user_data->mouse_scroll_state = ek_mouse_scroll_state_none;
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

    fprintf(stderr, "Unicode buffer is full!");
}

static GLFWwindow* CreateGLFWWindow(const s_vec_2d_i size, const char* const title, const e_window_flags flags, s_glfw_user_data* const user_data) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* const glfw_window = glfwCreateWindow(size.x, size.y, title, NULL, NULL);

    if (glfw_window) {
        glfwSetWindowAttrib(glfw_window, GLFW_RESIZABLE, flags & ek_window_flags_resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwSetInputMode(glfw_window, GLFW_CURSOR, flags & ek_window_flags_hide_cursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

        glfwMakeContextCurrent(glfw_window);

        glfwSwapInterval(1); // Enables VSync.

        glfwSetWindowUserPointer(glfw_window, user_data);

        glfwSetScrollCallback(glfw_window, GLFWScrollCallback);
        glfwSetCharCallback(glfw_window, GLFWCharCallback);
    } else {
        fprintf(stderr, "Failed to create a GLFW window!\n");
    }

    return glfw_window;
}

static void ResizeGLViewportIfDiff(const s_vec_2d_i size) {
    assert(size.x > 0 && size.y > 0);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    if (viewport[0] != 0 || viewport[1] != 0 || viewport[2] != size.x || viewport[3] != size.y) {
        glViewport(0, 0, size.x, size.y);
    }
}

bool RunGame(const s_game_info* const info) {
    assert(info);
    AssertGameInfoValidity(info);

    s_game_cleanup_info cleanup_info = {0}; // When something needs to be cleaned up after the game ends, the corresponding pointer here can be assigned.

    //
    // Initialisation
    //
    printf("Initialising...\n");

    InitRNG();

    // Initialise memory arenas.
    s_mem_arena perm_mem_arena = {0}; // The memory in here exists for the lifetime of the program, it does not get reset.

    if (!InitMemArena(&perm_mem_arena, PERM_MEM_ARENA_SIZE)) {
        fprintf(stderr, "Failed to initialise the permanent memory arena!\n");
        CleanGame(&cleanup_info);
        return false;
    }

    cleanup_info.perm_mem_arena = &perm_mem_arena;

    s_mem_arena temp_mem_arena = {0}; // While the memory here also exists for the program lifetime, it gets reset after game initialisation and after every frame. Useful if you just need some temporary working space.

    if (!InitMemArena(&temp_mem_arena, TEMP_MEM_ARENA_SIZE)) {
        fprintf(stderr, "Failed to initialise the temporary memory arena!\n");
        CleanGame(&cleanup_info);
        return false;
    }

    cleanup_info.temp_mem_arena = &temp_mem_arena;

    // Initialise GLFW.
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialise GLFW!\n");
        CleanGame(&cleanup_info);
        return false;
    }

    s_glfw_user_data glfw_user_data = {0}; // This is accessible from the GLFW callbacks.
    GLFWwindow* const glfw_window = CreateGLFWWindow(info->window_init_size, info->window_title, info->window_flags, &glfw_user_data);

    if (!glfw_window) {
        CleanGame(&cleanup_info);
        return false;
    }

    cleanup_info.glfw_window = glfw_window;

    // Initialise OpenGL rendering.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to load OpenGL function pointers!\n");
        CleanGame(&cleanup_info);
        return false;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    s_pers_render_data pers_render_data = {0};

    if (!InitPersRenderData(&pers_render_data, info->window_init_size)) {
        CleanGame(&cleanup_info);
        return false;
    }

    cleanup_info.pers_render_data = &pers_render_data;

    s_rendering_state* const rendering_state = MEM_ARENA_PUSH_TYPE(&perm_mem_arena, s_rendering_state);

    if (!rendering_state) {
        CleanGame(&cleanup_info);
        return false;
    }

    // Initialise audio system.
    s_audio_sys audio_sys = {0};

    if (!InitAudioSys(&audio_sys)) {
        CleanGame(&cleanup_info);
        return false;
    }

    cleanup_info.audio_sys = &audio_sys;

    // Initialise user memory.
    void* user_mem = NULL;

    if (info->user_mem_size > 0) {
        user_mem = PushToMemArena(&perm_mem_arena, info->user_mem_size, info->user_mem_alignment);

        if (!user_mem) {
            fprintf(stderr, "Failed to allocate user memory!\n");
            CleanGame(&cleanup_info);
            return false;
        }
    }

    // Run the user-defined game initialisation function.
    {
        const s_game_init_func_data func_data = {
            .user_mem = user_mem,
            .perm_mem_arena = &perm_mem_arena,
            .temp_mem_arena = &temp_mem_arena,
            .window_state = WindowState(glfw_window),
            .audio_sys = &audio_sys
        };

        if (!info->init_func(&func_data)) {
            fprintf(stderr, "Provided game initialisation function failed!\n");
            CleanGame(&cleanup_info);
            return false;
        }
    }

    //
    // Main Loop
    //
    glfwShowWindow(glfw_window);

    ResetMemArena(&temp_mem_arena);

    s_input_state input_state = {0};

    double frame_time_last = glfwGetTime();
    double frame_dur_accum = 0.0;

    printf("Entering the main loop...\n");

    while (!glfwWindowShouldClose(glfw_window)) {
        const s_window_state window_state = WindowState(glfw_window);

        ResizeGLViewportIfDiff(window_state.size);

        if (!Vec2DIsEqual(pers_render_data.surfs.size, window_state.size)) {
            if (!ResizeRenderSurfaces(&pers_render_data.surfs, window_state.size)) {
                fprintf(stderr, "Failed to resize render surfaces!\n");
                info->clean_func(user_mem);
                CleanGame(&cleanup_info);
                return false;
            }
        }

        const double frame_time = glfwGetTime();
        frame_dur_accum += frame_time - frame_time_last; // Update accumulator with delta time.
        frame_time_last = frame_time;

        // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run a tick.
        if (frame_dur_accum >= TARG_TICK_INTERVAL) {
            const s_input_state input_state_last = input_state;
            RefreshInputState(&input_state, glfw_window, glfw_user_data.mouse_scroll_state);

            UpdateAudioSys(&audio_sys);

            {
                // Execute the user-defined tick function.
                const s_game_tick_func_data func_data = {
                    .user_mem = user_mem,
                    .perm_mem_arena = &perm_mem_arena,
                    .temp_mem_arena = &temp_mem_arena,
                    .window_state = window_state,
                    .input_state = &input_state,
                    .input_state_last = &input_state_last,
                    .unicode_buf = &glfw_user_data.unicode_buf,
                    .audio_sys = &audio_sys
                };

                const e_game_tick_func_result res = info->tick_func(&func_data);

                ZERO_OUT(glfw_user_data.unicode_buf);
                glfw_user_data.mouse_scroll_state = ek_mouse_scroll_state_none;

                if (res == ek_game_tick_func_result_exit) {
                    info->clean_func(user_mem);
                    CleanGame(&cleanup_info);
                    return true;
                }

                if (res == ek_game_tick_func_result_error) {
                    info->clean_func(user_mem);
                    CleanGame(&cleanup_info);
                    return false;
                }
            }

            // Execute rendering step.
            BeginRendering(rendering_state);

            {
                // Execute the user-defined render function.
                const s_game_render_func_data func_data = {
                    .user_mem = user_mem,
                    .perm_mem_arena = &perm_mem_arena,
                    .temp_mem_arena = &temp_mem_arena,
                    .rendering_context = {
                        .pers = &pers_render_data,
                        .state = rendering_state,
                        .display_size = window_state.size
                    },
                    .input_state = &input_state,
                    .input_state_last = &input_state_last
                };

                if (!info->render_func(&func_data)) {
                    info->clean_func(user_mem);
                    CleanGame(&cleanup_info);
                    return false;
                }

                assert(HasFlushed(rendering_state) && "User-defined rendering function completed, but not everything has been flushed!");
            }

            glfwSwapBuffers(glfw_window);

            // Reset the accumulator so there is a delay until the next tick.
            frame_dur_accum = 0;
        }

        glfwPollEvents();

        ResetMemArena(&temp_mem_arena);
    }

    info->clean_func(user_mem);
    CleanGame(&cleanup_info);

    return true;
}
