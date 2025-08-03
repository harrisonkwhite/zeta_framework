#include "zfw_game.h"

#include <stdio.h>
#include "zfw_rng.h"

#define PERM_MEM_ARENA_SIZE MEGABYTES(80)
#define TEMP_MEM_ARENA_SIZE MEGABYTES(40)

#define TARG_TICKS_PER_SEC 60
#define TARG_TICK_INTERVAL (1.0 / TARG_TICKS_PER_SEC)

typedef struct {
    zfw_e_mouse_scroll_state mouse_scroll_state; // When mouse scroll is detected, this can be updated via callback. It can be reset after the next tick (so there is a chance for the developer using ZFW to detect the scroll).
    zfw_t_unicode_buf unicode_buf; // This is filled up as characters are typed, and zeroed out after the next tick. This way, the developer using ZFW can see everything that has been typed since the last tick.
} s_glfw_callback_data;

static void AssertGameInfoValidity(const zfw_s_game_info* const info) {
    assert(info);

    assert(info->window_title);
    assert(info->window_init_size.x > 0 && info->window_init_size.y > 0);

    assert(info->dev_mem_size >= 0);
    assert(info->dev_mem_size == 0 || IsAlignmentValid(info->dev_mem_alignment));

    assert(info->init_func);
    assert(info->tick_func);
    assert(info->render_func);
}

static zfw_s_window_state WindowState(GLFWwindow* const glfw_window) {
    assert(glfw_window);

    zfw_s_window_state state = {
        .fullscreen = glfwGetWindowMonitor(glfw_window) != NULL
    };
    glfwGetWindowPos(glfw_window, &state.pos.x, &state.pos.y);
    glfwGetWindowSize(glfw_window, &state.size.x, &state.size.y);

    return state;
}

static void GLFWScrollCallback(GLFWwindow* const window, const double offs_x, const double offs_y) {
    s_glfw_callback_data* const cb_data = glfwGetWindowUserPointer(window);

    if (offs_y > 0.0) {
        cb_data->mouse_scroll_state = zfw_ek_mouse_scroll_state_up;
    } else if (offs_y < 0.0) {
        cb_data->mouse_scroll_state = zfw_ek_mouse_scroll_state_down;
    } else {
        cb_data->mouse_scroll_state = zfw_ek_mouse_scroll_state_none;
    }
}

static void GLFWCharCallback(GLFWwindow* const window, const unsigned int codepoint) {
    s_glfw_callback_data* const cb_data = glfwGetWindowUserPointer(window);

    for (int i = 0; i < sizeof(cb_data->unicode_buf); i++) {
        if (!cb_data->unicode_buf[i]) {
            cb_data->unicode_buf[i] = (char)codepoint;
            return;
        }
    }

    LOG_WARNING("Unicode buffer is full!");
}

static void ResizeGLViewportIfDifferent(const zfw_s_vec_2d_s32 size) {
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
    ZFW_InitRNG();

    // Initialise memory arenas.
    s_mem_arena perm_mem_arena = {0}; // The memory in here exists for the lifetime of the program, it does not get reset.

    if (!InitMemArena(&perm_mem_arena, PERM_MEM_ARENA_SIZE)) {
        LOG_ERROR("Failed to initialise the permanent memory arena!");
        error = true;
        goto clean_nothing;
    }

    s_mem_arena temp_mem_arena = {0}; // While the memory here also exists for the program lifetime, it gets reset after game initialisation and after every frame. Useful if you just need some temporary working space.

    if (!InitMemArena(&temp_mem_arena, TEMP_MEM_ARENA_SIZE)) {
        LOG_ERROR("Failed to initialise the temporary memory arena!");
        error = true;
        goto clean_perm_mem_arena;
    }

    // Initialise GLFW.
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialise GLFW!");
        error = true;
        goto clean_temp_mem_arena;
    }

    // Set up the GLFW window.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ZFW_GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ZFW_GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* const glfw_window = glfwCreateWindow(info->window_init_size.x, info->window_init_size.y, info->window_title, NULL, NULL);

    if (!glfw_window) {
        LOG_ERROR("Failed to create a GLFW window!");
        error = true;
        goto clean_glfw;
    }

    glfwSetWindowAttrib(glfw_window, GLFW_RESIZABLE, info->window_flags & zfw_ek_window_flags_resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwSetInputMode(glfw_window, GLFW_CURSOR, info->window_flags & zfw_ek_window_flags_hide_cursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

    glfwMakeContextCurrent(glfw_window);

    glfwSwapInterval(1); // Enables VSync.

    // Set up GLFW callbacks.
    s_glfw_callback_data glfw_callback_data = {0};
    glfwSetWindowUserPointer(glfw_window, &glfw_callback_data);

    glfwSetScrollCallback(glfw_window, GLFWScrollCallback);
    glfwSetCharCallback(glfw_window, GLFWCharCallback);

    // Initialise OpenGL rendering.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Failed to load OpenGL function pointers!");
        error = true;
        goto clean_glfw_window;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    zfw_s_gl_resource_arena gl_res_arena = ZFW_GenGLResourceArena(&perm_mem_arena, KILOBYTES(1));

    if (IS_ZERO(gl_res_arena)) {
        LOG_ERROR("Failed to generate the OpenGL resource arena!");
        error = true;
        goto clean_glfw_window;
    }

    zfw_s_rendering_basis rendering_basis = {0};

    if (!ZFW_InitRenderingBasis(&rendering_basis, &gl_res_arena, &perm_mem_arena, &temp_mem_arena)) {
        LOG_ERROR("Failed to initialise the rendering basis!");
        error = true;
        goto clean_gl_res_arena;
    }

    zfw_s_rendering_state* const rendering_state = MEM_ARENA_PUSH_TYPE(&perm_mem_arena, zfw_s_rendering_state);

    if (!rendering_state) {
        LOG_ERROR("Failed to reserve memory for the rendering state!");
        error = true;
        goto clean_gl_res_arena;
    }

    // Initialise audio system.
    zfw_s_audio_sys audio_sys = {0};

    if (!ZFW_InitAudioSys(&audio_sys)) {
        LOG_ERROR("Failed to initialise the audio system!");
        error = true;
        goto clean_gl_res_arena;
    }

    // Initialise developer memory.
    void* dev_mem = NULL;

    if (info->dev_mem_size > 0) {
        dev_mem = PushToMemArena(&perm_mem_arena, info->dev_mem_size, info->dev_mem_alignment);

        if (!dev_mem) {
            LOG_ERROR("Failed to reserve developer memory!");
            error = true;
            goto clean_audio_sys;
        }
    }

    // Run the developer's initialisation function.
    {
        const zfw_s_game_init_context context = {
            .dev_mem = dev_mem,
            .perm_mem_arena = &perm_mem_arena,
            .temp_mem_arena = &temp_mem_arena,
            .window_state = WindowState(glfw_window),
            .gl_res_arena = &gl_res_arena,
            .rendering_basis = &rendering_basis,
            .audio_sys = &audio_sys
        };

        if (!info->init_func(&context)) {
            LOG_ERROR("Developer game initialisation function failed!");
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

    while (!glfwWindowShouldClose(glfw_window) && running) {
        glfwPollEvents();

        RewindMemArena(&temp_mem_arena, 0);

        const zfw_s_window_state window_state = WindowState(glfw_window);

        ResizeGLViewportIfDifferent(window_state.size);

        const double frame_time = glfwGetTime();
        frame_dur_accum += frame_time - frame_time_last; // Update accumulator with delta time.
        frame_time_last = frame_time;

        // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick and update the display.
        if (frame_dur_accum >= TARG_TICK_INTERVAL) {
            const zfw_s_input_state input_state_last = input_state;
            ZFW_RefreshInputState(&input_state, glfw_window, glfw_callback_data.mouse_scroll_state);

            ZFW_UpdateAudioSys(&audio_sys);

            // Run ticks.
            do {
                // Execute the developer's tick function.
                const zfw_s_game_tick_context context = {
                    .dev_mem = dev_mem,
                    .perm_mem_arena = &perm_mem_arena,
                    .temp_mem_arena = &temp_mem_arena,
                    .window_state = window_state,
                    .input_state = &input_state,
                    .input_state_last = &input_state_last,
                    .unicode_buf = &glfw_callback_data.unicode_buf,
                    .gl_res_arena = &gl_res_arena,
                    .rendering_basis = &rendering_basis,
                    .audio_sys = &audio_sys
                };

                const zfw_e_game_tick_result res = info->tick_func(&context);

                ZERO_OUT(glfw_callback_data.unicode_buf);
                glfw_callback_data.mouse_scroll_state = zfw_ek_mouse_scroll_state_none;

                if (res == zfw_ek_game_tick_result_exit) {
                    LOG("Exit request detected from developer game tick function...");
                    running = false;
                }

                if (res == zfw_ek_game_tick_result_error) {
                    LOG_ERROR("Developer game tick function failed!");
                    error = true;
                    goto clean_dev_game;
                }

                frame_dur_accum -= TARG_TICK_INTERVAL;
            } while (frame_dur_accum >= TARG_TICK_INTERVAL);

            // Update the display.
            ZERO_OUT(*rendering_state);
            ZFW_InitRenderingState(rendering_state);

            {
                // Execute the developer's render function.
                const zfw_s_game_render_context context = {
                    .dev_mem = dev_mem,
                    .perm_mem_arena = &perm_mem_arena,
                    .temp_mem_arena = &temp_mem_arena,
                    .mouse_pos = input_state.mouse_pos,
                    .rendering_context = {
                        .basis = &rendering_basis,
                        .state = rendering_state,
                        .window_size = window_state.size
                    }
                };

                if (!info->render_func(&context)) {
                    LOG_ERROR("Developer game render function failed!");
                    error = true;
                    goto clean_dev_game;
                }

                assert(rendering_state->batch.num_slots_used == 0 && "Developer rendering function completed, but not everything has been flushed!");
            }

            glfwSwapBuffers(glfw_window);
        }
    }

    //
    // Cleanup
    //
clean_dev_game:
    if (info->clean_func) {
        info->clean_func(dev_mem);
    }

clean_audio_sys:
    ZFW_CleanAudioSys(&audio_sys);

clean_gl_res_arena:
    ZFW_CleanGLResourceArena(&gl_res_arena);

clean_glfw_window:
    glfwDestroyWindow(glfw_window);

clean_glfw:
    glfwTerminate();

clean_temp_mem_arena:
    CleanMemArena(&temp_mem_arena);

clean_perm_mem_arena:
    CleanMemArena(&perm_mem_arena);

clean_nothing:

    return !error;
}
