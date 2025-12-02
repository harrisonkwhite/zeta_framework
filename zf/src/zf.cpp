#include <zf.h>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace zf {
    struct {
        GLFWwindow* glfw_window;

        s_static_bit_vec<eks_key_code_cnt> keys_down;
        s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_down;

        struct {
            s_static_bit_vec<eks_key_code_cnt> keys_pressed;
            s_static_bit_vec<eks_key_code_cnt> keys_released;

            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_pressed;
            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_released;
        } input_events;
    } g_game;

    constexpr e_key_code ConvertGLFWKeyCode(const t_s32 glfw_key) {
        switch (glfw_key) {
            case GLFW_KEY_SPACE: return ek_key_code_space;
            case GLFW_KEY_0: return ek_key_code_0;
            case GLFW_KEY_1: return ek_key_code_1;
            case GLFW_KEY_2: return ek_key_code_2;
            case GLFW_KEY_3: return ek_key_code_3;
            case GLFW_KEY_4: return ek_key_code_4;
            case GLFW_KEY_5: return ek_key_code_5;
            case GLFW_KEY_6: return ek_key_code_6;
            case GLFW_KEY_7: return ek_key_code_7;
            case GLFW_KEY_8: return ek_key_code_8;
            case GLFW_KEY_9: return ek_key_code_9;
            case GLFW_KEY_A: return ek_key_code_a;
            case GLFW_KEY_B: return ek_key_code_b;
            case GLFW_KEY_C: return ek_key_code_c;
            case GLFW_KEY_D: return ek_key_code_d;
            case GLFW_KEY_E: return ek_key_code_e;
            case GLFW_KEY_F: return ek_key_code_f;
            case GLFW_KEY_G: return ek_key_code_g;
            case GLFW_KEY_H: return ek_key_code_h;
            case GLFW_KEY_I: return ek_key_code_i;
            case GLFW_KEY_J: return ek_key_code_j;
            case GLFW_KEY_K: return ek_key_code_k;
            case GLFW_KEY_L: return ek_key_code_l;
            case GLFW_KEY_M: return ek_key_code_m;
            case GLFW_KEY_N: return ek_key_code_n;
            case GLFW_KEY_O: return ek_key_code_o;
            case GLFW_KEY_P: return ek_key_code_p;
            case GLFW_KEY_Q: return ek_key_code_q;
            case GLFW_KEY_R: return ek_key_code_r;
            case GLFW_KEY_S: return ek_key_code_s;
            case GLFW_KEY_T: return ek_key_code_t;
            case GLFW_KEY_U: return ek_key_code_u;
            case GLFW_KEY_V: return ek_key_code_v;
            case GLFW_KEY_W: return ek_key_code_w;
            case GLFW_KEY_X: return ek_key_code_x;
            case GLFW_KEY_Y: return ek_key_code_y;
            case GLFW_KEY_Z: return ek_key_code_z;
            case GLFW_KEY_ESCAPE: return ek_key_code_escape;
            case GLFW_KEY_ENTER: return ek_key_code_enter;
            case GLFW_KEY_BACKSPACE: return ek_key_code_backspace;
            case GLFW_KEY_TAB: return ek_key_code_tab;
            case GLFW_KEY_RIGHT: return ek_key_code_right;
            case GLFW_KEY_LEFT: return ek_key_code_left;
            case GLFW_KEY_DOWN: return ek_key_code_down;
            case GLFW_KEY_UP: return ek_key_code_up;
            case GLFW_KEY_F1: return ek_key_code_f1;
            case GLFW_KEY_F2: return ek_key_code_f2;
            case GLFW_KEY_F3: return ek_key_code_f3;
            case GLFW_KEY_F4: return ek_key_code_f4;
            case GLFW_KEY_F5: return ek_key_code_f5;
            case GLFW_KEY_F6: return ek_key_code_f6;
            case GLFW_KEY_F7: return ek_key_code_f7;
            case GLFW_KEY_F8: return ek_key_code_f8;
            case GLFW_KEY_F9: return ek_key_code_f9;
            case GLFW_KEY_F10: return ek_key_code_f10;
            case GLFW_KEY_F11: return ek_key_code_f11;
            case GLFW_KEY_F12: return ek_key_code_f12;
            case GLFW_KEY_LEFT_SHIFT: return ek_key_code_left_shift;
            case GLFW_KEY_LEFT_CONTROL: return ek_key_code_left_control;
            case GLFW_KEY_LEFT_ALT: return ek_key_code_left_alt;
            case GLFW_KEY_RIGHT_SHIFT: return ek_key_code_right_shift;
            case GLFW_KEY_RIGHT_CONTROL: return ek_key_code_right_control;
            case GLFW_KEY_RIGHT_ALT: return ek_key_code_right_alt;

            default: return eks_key_code_none;
        }
    }

    constexpr e_mouse_button_code ConvertGLFWMouseButtonCode(const t_s32 glfw_button) {
        switch (glfw_button) {
            case GLFW_MOUSE_BUTTON_LEFT: return ek_mouse_button_code_left;
            case GLFW_MOUSE_BUTTON_RIGHT: return ek_mouse_button_code_right;
            case GLFW_MOUSE_BUTTON_MIDDLE: return ek_mouse_button_code_middle;

            default: return eks_mouse_button_code_none;
        }
    }

    t_b8 RunGame(const s_game_info& info) {
        AssertGameInfoValidity(info);

        ZeroOut(g_game);

        const t_b8 success = [&info]() {
#ifndef ZF_DEBUG
            // Redirect stderr to crash log file.
            freopen("error.log", "w", stderr);
#endif

            //
            // Memory Arena Setup
            //
            s_mem_arena mem_arena;

            if (!AllocMemArena(info.mem_arena_size, mem_arena)) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ FreeMemArena(mem_arena); });

            s_mem_arena temp_mem_arena;

            if (!MakeSubMemArena(mem_arena, info.temp_mem_arena_size, temp_mem_arena)) {
                ZF_REPORT_ERROR();
                return false;
            }

            //
            // Window Creation
            //
            if (!glfwInit()) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ glfwTerminate(); });

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_VISIBLE, false);

            s_str window_init_title_terminated;

            if (!CloneStrButAddTerminator(info.window_init_title, temp_mem_arena, window_init_title_terminated)) {
                ZF_REPORT_ERROR();
                return false;
            }

            g_game.glfw_window = glfwCreateWindow(info.window_init_size.x, info.window_init_size.y, StrRaw(window_init_title_terminated), nullptr, nullptr);

            if (!g_game.glfw_window) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ glfwDestroyWindow(g_game.glfw_window); });

            glfwMakeContextCurrent(g_game.glfw_window);

            // Set up GLFW callbacks.
            glfwSetKeyCallback(g_game.glfw_window,
                [](GLFWwindow* const, const t_s32 key, const t_s32, const t_s32 action, const t_s32) {
                    const e_key_code key_code = ConvertGLFWKeyCode(key);

                    if (key_code == eks_key_code_none) {
                        return;
                    }

                    if (action == GLFW_PRESS) {
                        SetBit(g_game.keys_down, key_code);
                        SetBit(g_game.input_events.keys_pressed, key_code);
                    } else if (action == GLFW_RELEASE) {
                        UnsetBit(g_game.keys_down, key_code);
                        SetBit(g_game.input_events.keys_released, key_code);
                    }
                }
            );

            glfwSetMouseButtonCallback(g_game.glfw_window,
                [](GLFWwindow* const, const t_s32 button, const t_s32 action, const t_s32) {
                    const e_mouse_button_code mb_code = ConvertGLFWMouseButtonCode(button);

                    if (mb_code == eks_mouse_button_code_none) {
                        return;
                    }

                    if (action == GLFW_PRESS) {
                        SetBit(g_game.mouse_buttons_down, mb_code);
                        SetBit(g_game.input_events.mouse_buttons_pressed, mb_code);
                    } else if (action == GLFW_RELEASE) {
                        UnsetBit(g_game.mouse_buttons_down, mb_code);
                        SetBit(g_game.input_events.mouse_buttons_released, mb_code);
                    }
                }
            );

            //
            // Developer Initialisation
            //

            // Initialise developer memory.
            void* dev_mem = nullptr;

            if (info.dev_mem_size > 0) {
                dev_mem = PushToMemArena(mem_arena, info.dev_mem_size, info.dev_mem_alignment);

                if (!dev_mem) {
                    ZF_REPORT_ERROR();
                    return false;
                }
            }

            // Run the developer's initialisation function.
            {
                const s_game_init_context context = {
                    .dev_mem = dev_mem,
                    .mem_arena = &mem_arena,
                    .temp_mem_arena = &temp_mem_arena
                };

                if (!info.init_func(context)) {
                    ZF_REPORT_ERROR();
                    return false;
                }
            }

            ZF_DEFER({
                if (info.clean_func) {
                    info.clean_func(dev_mem);
                }
            });

            // Now that everything is set up, we can show the window.
            glfwShowWindow(g_game.glfw_window);

            //
            // Main Loop
            //
            t_f64 frame_time_last = glfwGetTime();
            t_f64 frame_dur_accum = 0.0;

            while (!glfwWindowShouldClose(g_game.glfw_window)) {
                RewindMemArena(temp_mem_arena, 0);

                const t_f64 frame_time = glfwGetTime();
                const t_f64 frame_time_delta = frame_time - frame_time_last;
                frame_dur_accum += frame_time_delta;
                frame_time_last = frame_time;

                const t_f64 targ_tick_interval = 1.0 / info.targ_ticks_per_sec;

                // Once enough time has passed (i.e. the time accumulator has reached the tick interval), run at least a single tick and update the display.
                if (frame_dur_accum >= targ_tick_interval) {
                    // Run possibly multiple ticks.
                    do {
                        const s_game_tick_context context = {
                            .dev_mem = dev_mem,
                            .mem_arena = &mem_arena,
                            .temp_mem_arena = &temp_mem_arena
                        };

                        if (!info.tick_func(context)) {
                            ZF_REPORT_ERROR();
                            return false;
                        }

                        frame_dur_accum -= targ_tick_interval;
                    } while (frame_dur_accum >= targ_tick_interval);

                    // Perform a single render.
                    glfwSwapBuffers(g_game.glfw_window);
                }

                glfwPollEvents();
            }

            return true;
        }();

#ifndef ZF_DEBUG
        if (!success) {
            ShowErrorBox("Error", "A fatal error occurred! Please check \"error.log\" for details.");
        }
#endif

        return success;
    }

    // ============================================================
    // @section: Window
    // ============================================================
    s_v2<t_s32> WindowSize();
    s_v2<t_s32> WindowFramebufferSize();

    // ============================================================
    // @section: Input
    // ============================================================
    t_b8 IsKeyDown(const e_key_code kc);
    t_b8 IsKeyPressed(const e_key_code kc);
    t_b8 IsKeyReleased(const e_key_code kc);

    t_b8 IsMouseButtonDown(const e_mouse_button_code mbc);
    t_b8 IsMouseButtonPressed(const e_mouse_button_code mbc);
    t_b8 IsMouseButtonReleased(const e_mouse_button_code mbc);

    s_v2<t_f32> MousePos();
}
