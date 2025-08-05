#include "zfw_game.h"

#include <stdio.h>
#include "zfw_rng.h"

#define PERM_MEM_ARENA_SIZE MEGABYTES(80)
#define TEMP_MEM_ARENA_SIZE MEGABYTES(40)

#define GL_RESOURCE_ARENA_RES_LIMIT 1024

#define TARG_TICKS_PER_SEC 60
#define TARG_TICK_INTERVAL (1.0 / TARG_TICKS_PER_SEC)

typedef enum {
    ek_game_run_stage_nothing_initted,
    ek_game_run_stage_perm_mem_arena_initted,
    ek_game_run_stage_temp_mem_arena_initted,
    ek_game_run_stage_glfw_initted,
    ek_game_run_stage_glfw_window_created,
    ek_game_run_stage_gl_res_arena_initted,
    ek_game_run_stage_audio_sys_initted,
    ek_game_run_stage_dev_init_func_ran_and_succeeded
} e_game_run_stage;

typedef struct {
    e_game_run_stage run_stage; // Used to determine what needs to be cleaned up.

    s_mem_arena perm_mem_arena; // The memory in here exists for the lifetime of the program, it does not get reset.
    s_mem_arena temp_mem_arena; // While the memory here also exists for the program lifetime, it gets reset after game initialisation and after every frame. Useful if you just need some temporary working space.

    GLFWwindow* glfw_window;

    zfw_s_input_events input_events; // Events such as key presses and mouse wheel scrolls are stored here via GLFW callbacks, accessible in the next tick and then cleared. Ensures that events occurring between ticks are not lost.

    zfw_s_gl_resource_arena gl_res_arena; // Contains all OpenGL resources used over the lifetime of the game, so they can be cleaned up all at once (simplifies resource management).
    zfw_s_rendering_basis rendering_basis; // Foundational rendering data used throughout the lifetime of the game.

    zfw_s_audio_sys audio_sys;

    void* dev_mem; // Memory optionally reserved by the developer for their own use, accessible in their defined functions through the provided ZFW context.
} s_game;

static zfw_s_window_state WindowState(GLFWwindow* const glfw_window) {
    assert(glfw_window);

    zfw_s_window_state state = {
        .fullscreen = glfwGetWindowMonitor(glfw_window) != NULL
    };
    glfwGetWindowPos(glfw_window, &state.pos.x, &state.pos.y);
    glfwGetWindowSize(glfw_window, &state.size.x, &state.size.y);

    return state;
}

static void ResizeGLViewportIfDifferent(const s_v2_int size) {
    assert(size.x > 0 && size.y > 0);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    if (viewport[0] != 0 || viewport[1] != 0 || viewport[2] != size.x || viewport[3] != size.y) {
        glViewport(0, 0, size.x, size.y);
    }
}

static bool ExecGameInitAndMainLoop(s_game* const game, const zfw_s_game_info* const info) {
    assert(game && IS_ZERO(*game));
    ZFW_AssertGameInfoValidity(info);

    //
    // Initialisation
    //
    ZFW_InitRNG();

    // Initialise memory arenas.
    if (!InitMemArena(&game->perm_mem_arena, PERM_MEM_ARENA_SIZE)) {
        LOG_ERROR("Failed to initialise the permanent memory arena!");
        return false;
    }

    game->run_stage = ek_game_run_stage_perm_mem_arena_initted;

    if (!InitMemArena(&game->temp_mem_arena, TEMP_MEM_ARENA_SIZE)) {
        LOG_ERROR("Failed to initialise the temporary memory arena!");
        return false;
    }

    game->run_stage = ek_game_run_stage_temp_mem_arena_initted;

    // Initialise GLFW.
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialise GLFW!");
        return false;
    }

    game->run_stage = ek_game_run_stage_glfw_initted;

    // Set up the GLFW window.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ZFW_GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ZFW_GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, false);

    game->glfw_window = glfwCreateWindow(info->window_init_size.x, info->window_init_size.y, info->window_title, NULL, NULL);

    if (!game->glfw_window) {
        LOG_ERROR("Failed to create a GLFW window!");
        return false;
    }

    game->run_stage = ek_game_run_stage_glfw_window_created;

    glfwSetWindowAttrib(game->glfw_window, GLFW_RESIZABLE, info->window_flags & zfw_ek_window_flags_resizable ? true : false);
    glfwSetInputMode(game->glfw_window, GLFW_CURSOR, info->window_flags & zfw_ek_window_flags_hide_cursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

    glfwMakeContextCurrent(game->glfw_window);

    glfwSwapInterval(1); // Enables VSync.

    // Set up GLFW callbacks.
    glfwSetWindowUserPointer(game->glfw_window, &game->input_events);

    glfwSetKeyCallback(game->glfw_window, ZFW_GLFWKeyCallback);
    glfwSetMouseButtonCallback(game->glfw_window, ZFW_GLFWMouseButtonCallback);
    glfwSetScrollCallback(game->glfw_window, ZFW_GLFWScrollCallback);
    glfwSetCharCallback(game->glfw_window, ZFW_GLFWCharCallback);

    // Initialise rendering.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Failed to load OpenGL function pointers!");
        return false;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!ZFW_InitGLResourceArena(&game->gl_res_arena, &game->perm_mem_arena, GL_RESOURCE_ARENA_RES_LIMIT)) {
        LOG_ERROR("Failed to initialise OpenGL resource arena!");
        return false;
    }

    game->run_stage = ek_game_run_stage_gl_res_arena_initted;

    if (!ZFW_InitRenderingBasis(&game->rendering_basis, &game->gl_res_arena, &game->perm_mem_arena, &game->temp_mem_arena)) {
        LOG_ERROR("Failed to initialise the rendering basis!");
        return false;
    }

    // Initialise audio.
    if (!ZFW_InitAudioSys(&game->audio_sys)) {
        LOG_ERROR("Failed to initialise the audio system!");
        return false;
    }

    game->run_stage = ek_game_run_stage_audio_sys_initted;

    // Initialise developer memory.
    if (info->dev_mem_size > 0) {
        game->dev_mem = PushToMemArena(&game->perm_mem_arena, info->dev_mem_size, info->dev_mem_alignment);

        if (!game->dev_mem) {
            LOG_ERROR("Failed to reserve developer memory!");
            return false;
        }
    }

    // Run the developer's initialisation function.
    {
        const zfw_s_game_init_context context = {
            .dev_mem = game->dev_mem,
            .perm_mem_arena = &game->perm_mem_arena,
            .temp_mem_arena = &game->temp_mem_arena,
            .window_state = WindowState(game->glfw_window),
            .gl_res_arena = &game->gl_res_arena,
            .rendering_basis = &game->rendering_basis,
            .audio_sys = &game->audio_sys
        };

        if (!info->init_func(&context)) {
            LOG_ERROR("Developer game initialisation function failed!");
            return false;
        }
    }

    game->run_stage = ek_game_run_stage_dev_init_func_ran_and_succeeded;

    // Now that everything is set up, we can show the window.
    glfwShowWindow(game->glfw_window);

    //
    // Main Loop
    //
    double frame_time_last = glfwGetTime();
    double frame_dur_accum = 0.0;

    while (!glfwWindowShouldClose(game->glfw_window)) {
        RewindMemArena(&game->temp_mem_arena, 0);

        const zfw_s_window_state window_state = WindowState(game->glfw_window);

        ResizeGLViewportIfDifferent(window_state.size);

        const double frame_time = glfwGetTime();
        const double frame_time_delta = frame_time - frame_time_last;
        frame_dur_accum += frame_time_delta;
        frame_time_last = frame_time;

        // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick and update the display.
        if (frame_dur_accum >= TARG_TICK_INTERVAL) {
            const zfw_s_input_state input_state = ZFW_InputState(game->glfw_window);

            ZFW_UpdateAudioSys(&game->audio_sys);

            // Run ticks.
            do {
                // Execute the developer's tick function.
                const zfw_s_game_tick_context context = {
                    .dev_mem = game->dev_mem,
                    .perm_mem_arena = &game->perm_mem_arena,
                    .temp_mem_arena = &game->temp_mem_arena,
                    .window_state = window_state,
                    .input_context = {
                        .state = &input_state,
                        .events = &game->input_events
                    },
                    .gl_res_arena = &game->gl_res_arena,
                    .rendering_basis = &game->rendering_basis,
                    .audio_sys = &game->audio_sys
                };

                const zfw_e_game_tick_result res = info->tick_func(&context);

                ZERO_OUT(game->input_events);

                if (res == zfw_ek_game_tick_result_exit) {
                    LOG("Exit request detected from developer game tick function...");
                    glfwSetWindowShouldClose(game->glfw_window, true);
                }

                if (res == zfw_ek_game_tick_result_error) {
                    LOG_ERROR("Developer game tick function failed!");
                    return false;
                }

                frame_dur_accum -= TARG_TICK_INTERVAL;
            } while (frame_dur_accum >= TARG_TICK_INTERVAL);

            // Render the game.
            zfw_s_rendering_state* const rendering_state = ZFW_PushRenderingState(&game->temp_mem_arena);

            if (!rendering_state) {
                LOG_ERROR("Failed to reserve memory for rendering state!");
                return false;
            }

            {
                // Execute the developer's render function.
                const zfw_s_game_render_context context = {
                    .dev_mem = game->dev_mem,
                    .perm_mem_arena = &game->perm_mem_arena,
                    .temp_mem_arena = &game->temp_mem_arena,
                    .mouse_pos = input_state.mouse_pos,
                    .rendering_context = {
                        .basis = &game->rendering_basis,
                        .state = rendering_state,
                        .window_size = window_state.size
                    }
                };

                if (!info->render_func(&context)) {
                    LOG_ERROR("Developer game render function failed!");
                    return false;
                }

                ZFW_SubmitBatch(&context.rendering_context);
            }

            glfwSwapBuffers(game->glfw_window);
        }

        glfwPollEvents();
    }

    return true;
}

bool ZFW_RunGame(const zfw_s_game_info* const info) {
    ZFW_AssertGameInfoValidity(info);

    s_game game = {0};

    const bool result = ExecGameInitAndMainLoop(&game, info);

    // Clean up.
    for (int i = game.run_stage; i >= 0; i--) {
        switch ((e_game_run_stage)i) {
            case ek_game_run_stage_nothing_initted:
                break;

            case ek_game_run_stage_perm_mem_arena_initted:
                CleanMemArena(&game.perm_mem_arena);
                break;

            case ek_game_run_stage_temp_mem_arena_initted:
                CleanMemArena(&game.temp_mem_arena);
                break;

            case ek_game_run_stage_glfw_initted:
                glfwTerminate();
                break;

            case ek_game_run_stage_glfw_window_created:
                assert(game.glfw_window);
                glfwDestroyWindow(game.glfw_window);
                break;

            case ek_game_run_stage_gl_res_arena_initted:
                ZFW_CleanGLResourceArena(&game.gl_res_arena);
                break;

            case ek_game_run_stage_audio_sys_initted:
                ZFW_CleanAudioSys(&game.audio_sys);
                break;

            case ek_game_run_stage_dev_init_func_ran_and_succeeded:
                if (info->clean_func) {
                    info->clean_func(game.dev_mem);
                }

                break;
        }
    }

    return result;
}
