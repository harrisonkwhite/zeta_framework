#include <zf.h>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <miniaudio.h>

namespace zf {
    // ============================================================
    // @section: Declarations and Data
    // ============================================================

    // This is all global to remove the need for the framework user to have to pass a window pointer or input state around just so they can access READ-ONLY info (changes to window state are never done directly by them and instead can only be requested).
    struct {
        GLFWwindow* glfw_window;

        struct {
            s_static_bit_vec<eks_key_code_cnt> keys_down;
            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_down;

            struct {
                s_static_bit_vec<eks_key_code_cnt> keys_pressed;
                s_static_bit_vec<eks_key_code_cnt> keys_released;

                s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_pressed;
                s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_released;
            } events;
        } input;
    } g_game;

    constexpr e_key_code ConvertGLFWKeyCode(const t_s32 glfw_key);
    constexpr e_mouse_button_code ConvertGLFWMouseButtonCode(const t_s32 glfw_button);

    using t_gl_id = GLuint;

    struct s_mesh_gl_ids {
        t_gl_id vert_arr;
        t_gl_id vert_buf;
        t_gl_id elem_buf;
    };

    static s_mesh_gl_ids MakeGLMesh(const t_f32* const verts_raw, const t_size verts_len, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_s32> vert_attr_lens);
    static void ReleaseGLMesh(const s_mesh_gl_ids& gl_ids);
    static t_gl_id MakeGLShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_mem_arena& temp_mem_arena);
    static t_gl_id MakeGLTexture(const s_rgba_texture_data_rdonly& tex_data);
    static s_v2<t_s32> GLTextureSizeLimit();
    static s_rect<t_s32> GLViewport();
    static t_gl_id CurGLShaderProg();
    [[nodiscard]] static t_b8 AttachGLFramebufferTexture(const t_gl_id fb_gl_id, const t_gl_id tex_gl_id, const s_v2<t_s32> tex_size);

    struct s_gfx_resource {
        e_gfx_resource_type type;

        union {
            struct {
                t_gl_id gl_id;
                s_v2<t_s32> size;
            } tex;

            struct {
                s_font_arrangement arrangement;
                s_array<t_gl_id> atlas_gl_ids;
            } font;

            struct {
                t_gl_id fb_gl_id;
                t_gl_id tex_gl_id;
                s_v2<t_s32> size;
            } surf;

            struct {
                t_gl_id gl_id;
            } surf_shader_prog;
        } type_data;

        s_gfx_resource* next;
    };

    static s_gfx_resource* PushGFXResource(s_gfx_resource_arena& res_arena);
    [[nodiscard]] static t_b8 MakeFontAtlasGLTextures(const s_array_rdonly<t_font_atlas_rgba> atlas_rgbas, s_array<t_gl_id>& o_gl_ids);

    struct s_batch_vert {
        s_v2<t_f32> vert_coord;
        s_v2<t_f32> pos;
        s_v2<t_f32> size;
        t_f32 rot;
        s_v2<t_f32> tex_coord;
        s_color_rgba32f blend;
    };

    constexpr s_static_array<t_s32, 6> g_batch_vert_attr_lens = {
        {2, 2, 2, 1, 2, 4} // This has to match the number of components per attribute above.
    };

    constexpr t_size g_batch_vert_component_cnt = ZF_SIZE_OF(s_batch_vert) / ZF_SIZE_OF(t_f32);

    static_assert([]() {
        t_size sum = 0;

        for (t_size i = 0; i < g_batch_vert_attr_lens.g_len; i++) {
            sum += g_batch_vert_attr_lens[i];
        }

        return sum == g_batch_vert_component_cnt;
    }(), "Mismatch between specified batch vertex attribute lengths and component count!");

    constexpr t_size g_batch_slot_cnt = 1 << 8;
    static_assert(g_batch_slot_cnt <= 1 << 16, "Batch slot count is too large (need to account for range limits of elements).");

    constexpr t_size g_batch_slot_vert_cnt = 4;
    constexpr t_size g_batch_slot_elem_cnt = 6;

    using t_batch_slot = s_static_array<s_batch_vert, g_batch_slot_vert_cnt>;

    static const s_str_rdonly g_batch_vert_shader_src = R"(#version 460 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec2 a_pos;
layout (location = 2) in vec2 a_size;
layout (location = 3) in float a_rot;
layout (location = 4) in vec2 a_tex_coord;
layout (location = 5) in vec4 a_blend;

out vec2 v_tex_coord;
out vec4 v_blend;

uniform mat4 u_view;
uniform mat4 u_proj;

void main() {
    float rot_cos = cos(a_rot);
    float rot_sin = -sin(a_rot);

    mat4 model = mat4(
        vec4(a_size.x * rot_cos, a_size.x * rot_sin, 0.0, 0.0),
        vec4(a_size.y * -rot_sin, a_size.y * rot_cos, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(a_pos.x, a_pos.y, 0.0, 1.0)
    );

    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0, 1.0);
    v_tex_coord = a_tex_coord;
    v_blend = a_blend;
}
)";

    static const s_str_rdonly g_batch_frag_shader_src = R"(#version 460 core

in vec2 v_tex_coord;
in vec4 v_blend;

out vec4 o_frag_color;

uniform sampler2D u_tex;

void main() {
    vec4 tex_color = texture(u_tex, v_tex_coord);
    o_frag_color = tex_color * v_blend;
}
)";

    static const s_str_rdonly g_default_surface_vert_shader_src = R"(#version 460 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec2 a_tex_coord;

out vec2 v_tex_coord;

uniform vec2 u_pos;
uniform vec2 u_size;
uniform mat4 u_proj;

void main() {
    mat4 model = mat4(
        vec4(u_size.x, 0.0, 0.0, 0.0),
        vec4(0.0, u_size.y, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(u_pos.x, u_pos.y, 0.0, 1.0)
    );

    gl_Position = u_proj * model * vec4(a_vert, 0.0, 1.0);
    v_tex_coord = a_tex_coord;
}
)";

    static const s_str_rdonly g_default_surface_frag_shader_src = R"(#version 460 core

in vec2 v_tex_coord;
out vec4 o_frag_color;

uniform sampler2D u_tex;

void main() {
    o_frag_color = texture(u_tex, v_tex_coord);
}
)";

    struct s_rendering_basis {
        s_mesh_gl_ids batch_mesh_gl_ids;
        t_gl_id batch_shader_prog_gl_id;

        s_mesh_gl_ids surf_mesh_gl_ids;

        s_gfx_resource_arena res_arena;
        s_gfx_resource* px_tex;
        s_gfx_resource* default_surf_shader_prog;
    };

    struct s_rendering_state {
        s_static_array<t_batch_slot, g_batch_slot_cnt> batch_slots;
        t_size batch_slots_used_cnt;

        s_matrix_4x4 view_mat; // The view matrix to be used when flushing.
        t_gl_id tex_gl_id; // The texture to be used when flushing.

        s_static_stack<const s_gfx_resource*, 32> surfs;
    };

    struct s_rendering_context {
        const s_rendering_basis* basis;
        s_rendering_state* state;
    };

    [[nodiscard]] static t_b8 MakeRenderingBasis(s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena, s_rendering_basis& o_basis);
    static void ReleaseRenderingBasis(s_rendering_basis& basis);
    static void Flush(const s_rendering_context& rc);
    static void Draw(const s_rendering_context& rc, const t_gl_id tex_gl_id, const s_rect<t_f32> tex_coords, s_v2<t_f32> pos, s_v2<t_f32> size, s_v2<t_f32> origin, const t_f32 rot, const s_color_rgba32f blend);

    struct s_sound_type {
        s_sound_meta meta;
        s_array_rdonly<t_f32> pcm;

        s_sound_type* next;
    };

    constexpr t_size g_snd_inst_limit = 32;

    struct s_audio_context {
        ma_engine ma_eng;

        struct {
            s_static_array<ma_sound, g_snd_inst_limit> ma_snds;
            s_static_array<ma_audio_buffer_ref, g_snd_inst_limit> ma_buf_refs;
            s_static_array<const s_sound_type*, g_snd_inst_limit> types;
            s_static_bit_vec<g_snd_inst_limit> activity;
            s_static_array<t_size, g_snd_inst_limit> versions;
        } snd_insts;
    };

    struct s_sound_id {
        t_size index;
        t_size version;
    };

    [[nodiscard]] static t_b8 InitAudio(s_audio_context& o_ac);
    static void ShutdownAudio(s_audio_context& ac);
    void ProcFinishedSounds(s_audio_context& ac);

    // ============================================================
    // @section: Game
    // ============================================================
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

            s_mem_arena frame_mem_arena;

            if (!MakeSubMemArena(mem_arena, info.frame_mem_arena_size, frame_mem_arena)) {
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

            //
            // GLFW Callbacks
            //
            glfwSetKeyCallback(g_game.glfw_window,
                [](GLFWwindow* const, const t_s32 key, const t_s32, const t_s32 action, const t_s32) {
                    const e_key_code key_code = ConvertGLFWKeyCode(key);

                    if (key_code == eks_key_code_none) {
                        return;
                    }

                    if (action == GLFW_PRESS) {
                        SetBit(g_game.input.keys_down, key_code);
                        SetBit(g_game.input.events.keys_pressed, key_code);
                    } else if (action == GLFW_RELEASE) {
                        UnsetBit(g_game.input.keys_down, key_code);
                        SetBit(g_game.input.events.keys_released, key_code);
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
                        SetBit(g_game.input.mouse_buttons_down, mb_code);
                        SetBit(g_game.input.events.mouse_buttons_pressed, mb_code);
                    } else if (action == GLFW_RELEASE) {
                        UnsetBit(g_game.input.mouse_buttons_down, mb_code);
                        SetBit(g_game.input.events.mouse_buttons_released, mb_code);
                    }
                }
            );

            //
            // Rendering Setup
            //

            // Initialise OpenGL function pointers.
            if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
                ZF_REPORT_ERROR();
                return false;
            }

            // Enable blending.
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Set up rendering basis.
            s_rendering_basis rendering_basis;

            if (!MakeRenderingBasis(mem_arena, temp_mem_arena, rendering_basis)) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ ReleaseRenderingBasis(rendering_basis); });

            //
            // Audio Setup
            //
            s_audio_context audio_context;

            if (!InitAudio(audio_context)) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ ShutdownAudio(audio_context); });

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
                    .temp_mem_arena = &temp_mem_arena,
                    .gfx_res_arena = &rendering_basis.res_arena,
                    .audio_context = &audio_context
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

            //
            // Main Loop
            //
            glfwShowWindow(g_game.glfw_window);

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
                    RewindMemArena(frame_mem_arena, 0);

                    ProcFinishedSounds(audio_context);

                    // Run possibly multiple ticks.
                    do {
                        ZF_DEFER({ ZeroOut(g_game.input.events); });

                        const s_game_tick_context context = {
                            .dev_mem = dev_mem,
                            .mem_arena = &mem_arena,
                            .temp_mem_arena = &temp_mem_arena,
                            .gfx_res_arena = &rendering_basis.res_arena,
                            .audio_context = &audio_context
                        };

                        if (!info.tick_func(context)) {
                            ZF_REPORT_ERROR();
                            return false;
                        }

                        frame_dur_accum -= targ_tick_interval;
                    } while (frame_dur_accum >= targ_tick_interval);

                    // Perform a single render.
                    const s_rendering_context rc = {
                        .basis = &rendering_basis,
                        .state = PushToMemArena<s_rendering_state>(frame_mem_arena)
                    };

                    if (!rc.state) {
                        ZF_REPORT_ERROR();
                        return false;
                    }

                    rc.state->view_mat = MakeIdentityMatrix4x4();

                    Clear(rc, s_color_rgba8(147, 207, 249, 255));

                    {
                        const s_game_render_context context = {
                            .dev_mem = dev_mem,
                            .mem_arena = &mem_arena,
                            .temp_mem_arena = &temp_mem_arena,
                            .rendering_context = &rc
                        };

                        if (!info.render_func(context)) {
                            ZF_REPORT_ERROR();
                            return false;
                        }
                    }

                    Flush(rc);

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
    s_v2<t_s32> WindowSize() {
        t_s32 w, h;
        glfwGetWindowSize(g_game.glfw_window, &w, &h);
        return {w, h};
    }

    s_v2<t_s32> WindowFramebufferSize() {
        t_s32 w, h;
        glfwGetFramebufferSize(g_game.glfw_window, &w, &h);
        return {w, h};
    }

    // ============================================================
    // @section: Input
    // ============================================================
    t_b8 IsKeyDown(const e_key_code kc) {
        return IsBitSet(g_game.input.keys_down, kc);
    }

    t_b8 IsKeyPressed(const e_key_code kc) {
        return IsBitSet(g_game.input.events.keys_pressed, kc);
    }

    t_b8 IsKeyReleased(const e_key_code kc) {
        return IsBitSet(g_game.input.events.keys_released, kc);
    }

    t_b8 IsMouseButtonDown(const e_mouse_button_code mbc) {
        return IsBitSet(g_game.input.mouse_buttons_down, mbc);
    }

    t_b8 IsMouseButtonPressed(const e_mouse_button_code mbc) {
        return IsBitSet(g_game.input.events.mouse_buttons_pressed, mbc);
    }

    t_b8 IsMouseButtonReleased(const e_mouse_button_code mbc) {
        return IsBitSet(g_game.input.events.mouse_buttons_released, mbc);
    }

    s_v2<t_f32> MousePos() {
        t_f64 mx, my;
        glfwGetCursorPos(g_game.glfw_window, &mx, &my);
        return {static_cast<t_f32>(mx), static_cast<t_f32>(my)};
    }

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

    // ============================================================
    // @section: OpenGL Helpers
    // ============================================================
    s_mesh_gl_ids MakeGLMesh(const t_f32* const verts_raw, const t_size verts_len, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_s32> vert_attr_lens) {
        s_mesh_gl_ids gl_ids = {};

        glGenVertexArrays(1, &gl_ids.vert_arr);
        glBindVertexArray(gl_ids.vert_arr);

        glGenBuffers(1, &gl_ids.vert_buf);
        glBindBuffer(GL_ARRAY_BUFFER, gl_ids.vert_buf);
        glBufferData(GL_ARRAY_BUFFER, ZF_SIZE_OF(t_f32) * verts_len, verts_raw, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &gl_ids.elem_buf);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_ids.elem_buf);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(ArraySizeInBytes(elems)), elems.buf_raw, GL_STATIC_DRAW);

        const t_size stride = [vert_attr_lens]() {
            t_size res = 0;

            for (t_size i = 0; i < vert_attr_lens.len; i++) {
                res += ZF_SIZE_OF(t_f32) * static_cast<t_size>(vert_attr_lens[i]);
            }

            return res;
        }();

        t_s32 offs = 0;

        for (t_size i = 0; i < vert_attr_lens.len; i++) {
            const t_s32 attr_len = vert_attr_lens[i];

            glVertexAttribPointer(static_cast<GLuint>(i), attr_len, GL_FLOAT, false, static_cast<GLsizei>(stride), reinterpret_cast<void*>(ZF_SIZE_OF(t_f32) * offs));
            glEnableVertexAttribArray(static_cast<GLuint>(i));

            offs += attr_len;
        }

        glBindVertexArray(0);

        return gl_ids;
    }

    void ReleaseGLMesh(const s_mesh_gl_ids& gl_ids) {
        glDeleteBuffers(1, &gl_ids.elem_buf);
        glDeleteBuffers(1, &gl_ids.vert_buf);
        glDeleteVertexArrays(1, &gl_ids.vert_arr);
    }

    t_gl_id MakeGLShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_mem_arena& temp_mem_arena) {
        s_str vert_src_terminated;
        s_str frag_src_terminated;

        if (!CloneStrButAddTerminator(vert_src, temp_mem_arena, vert_src_terminated)
            || !CloneStrButAddTerminator(frag_src, temp_mem_arena, frag_src_terminated)) {
            return 0;
        }

        // Generate the individual shaders.
        const auto gen_shader = [&temp_mem_arena](const s_str_rdonly src, const t_b8 is_frag) -> t_gl_id {
            const t_gl_id shader_gl_id = glCreateShader(is_frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);

            const auto src_raw = StrRaw(src);
            glShaderSource(shader_gl_id, 1, &src_raw, nullptr);

            glCompileShader(shader_gl_id);

            t_s32 success;
            glGetShaderiv(shader_gl_id, GL_COMPILE_STATUS, &success);

            if (!success) {
                ZF_DEFER({ glDeleteShader(shader_gl_id); });

                // Try getting the OpenGL compile error message.
                t_s32 log_chr_cnt;
                glGetShaderiv(shader_gl_id, GL_INFO_LOG_LENGTH, &log_chr_cnt);

                if (log_chr_cnt >= 1) {
                    s_array<char> log_chrs;

                    if (MakeArray(temp_mem_arena, log_chr_cnt, log_chrs)) {
                        glGetShaderInfoLog(shader_gl_id, static_cast<GLsizei>(log_chrs.len), nullptr, log_chrs.buf_raw);
                        LogErrorType("OpenGL Shader Compilation", "%", StrFromRaw(log_chrs.buf_raw));
                    } else {
                        LogError("Failed to reserve memory for OpenGL shader compilation error log!");
                    }
                } else {
                    LogError("OpenGL shader compilation failed, but no error message available!");
                }

                return 0;
            }

            return shader_gl_id;
        };

        const t_gl_id vert_gl_id = gen_shader(vert_src_terminated, false);

        if (!vert_gl_id) {
            return 0;
        }

        ZF_DEFER({ glDeleteShader(vert_gl_id); });

        const t_gl_id frag_gl_id = gen_shader(frag_src_terminated, true);

        if (!frag_gl_id) {
            return 0;
        }

        ZF_DEFER({ glDeleteShader(frag_gl_id); });

        // Set up the shader program.
        const t_gl_id prog_gl_id = glCreateProgram();
        glAttachShader(prog_gl_id, vert_gl_id);
        glAttachShader(prog_gl_id, frag_gl_id);
        glLinkProgram(prog_gl_id);

        // @todo: Check for link success.

        return prog_gl_id;
    }

    t_gl_id MakeGLTexture(const s_rgba_texture_data_rdonly& tex_data) {
        const s_v2<t_s32> tex_size_limit = GLTextureSizeLimit();

        if (tex_data.size_in_pxs.x > tex_size_limit.x || tex_data.size_in_pxs.y > tex_size_limit.y) {
            LogError("Texture size % exceeds limits %!", tex_data.size_in_pxs, tex_size_limit);
            ZF_REPORT_ERROR();
            return 0;
        }

        t_gl_id gl_id;
        glGenTextures(1, &gl_id);

        glBindTexture(GL_TEXTURE_2D, gl_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_data.size_in_pxs.x, tex_data.size_in_pxs.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.px_data.buf_raw);

        return gl_id;
    }

    s_v2<t_s32> GLTextureSizeLimit() {
        t_s32 size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
        return { size, size };
    }

    s_rect<t_s32> GLViewport() {
        s_rect<t_s32> v;
        glGetIntegerv(GL_VIEWPORT, reinterpret_cast<t_s32*>(&v));
        return v;
    }

    t_gl_id CurGLShaderProg() {
        t_s32 prog;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        return static_cast<t_gl_id>(prog);
    }

    t_b8 AttachGLFramebufferTexture(const t_gl_id fb_gl_id, const t_gl_id tex_gl_id, const s_v2<t_s32> tex_size) {
        glBindTexture(GL_TEXTURE_2D, tex_gl_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(tex_size.x), static_cast<GLsizei>(tex_size.y), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_gl_id, 0);

        const t_b8 success = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return success;
    }

    // ============================================================
    // @section: GFX Resources
    // ============================================================
    void ReleaseGFXResources(const s_gfx_resource_arena& res_arena) {
        const s_gfx_resource* res = res_arena.head;

        while (res) {
            switch (res->type) {
                case ek_gfx_resource_type_texture:
                    glDeleteTextures(1, &res->type_data.tex.gl_id);
                    break;

                case ek_gfx_resource_type_font:
                    glDeleteTextures(static_cast<GLsizei>(res->type_data.font.atlas_gl_ids.len), res->type_data.font.atlas_gl_ids.buf_raw);
                    break;

                case ek_gfx_resource_type_surface:
                    glDeleteTextures(1, &res->type_data.surf.tex_gl_id);
                    glDeleteFramebuffers(1, &res->type_data.surf.fb_gl_id);
                    break;

                case ek_gfx_resource_type_surface_shader_prog:
                    glDeleteProgram(res->type_data.surf_shader_prog.gl_id);
                    break;

                default:
                    ZF_ASSERT(false);
                    break;
            }

            res = res->next;
        }
    }

    s_gfx_resource* PushGFXResource(s_gfx_resource_arena& res_arena) {
        const auto res = PushToMemArena<s_gfx_resource>(*res_arena.mem_arena);

        if (!res) {
            return nullptr;
        }

        if (!res_arena.head) {
            res_arena.head = res;
            res_arena.tail = res;
        } else {
            res_arena.tail->next = res;
            res_arena.tail = res;
        }

        return res;
    }

    t_b8 LoadTexture(const s_rgba_texture_data_rdonly& tex_data, s_gfx_resource_arena& res_arena, s_gfx_resource*& o_tex) {
        const t_gl_id gl_id = MakeGLTexture(tex_data);

        if (!gl_id) {
            return false;
        }

        o_tex = PushGFXResource(res_arena);

        if (!o_tex) {
            return false;
        }

        o_tex->type = ek_gfx_resource_type_texture;
        o_tex->type_data.tex = {.gl_id = gl_id, .size = tex_data.size_in_pxs};

        return true;
    }

    s_v2<t_s32> TextureSize(const s_gfx_resource* const res) {
        ZF_ASSERT(res);
        ZF_ASSERT(res->type == ek_gfx_resource_type_texture);

        return res->type_data.tex.size;
    }

    t_b8 LoadFontFromRaw(const s_str_rdonly file_path, const t_s32 height, const t_unicode_code_pt_bit_vec& code_pts, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_font, e_font_load_from_raw_result* const o_load_from_raw_res, t_unicode_code_pt_bit_vec* const o_unsupported_code_pts) {
        s_font_arrangement arrangement;
        s_array<t_font_atlas_rgba> atlas_rgbas;

        const auto res = zf::LoadFontFromRaw(file_path, height, code_pts, *res_arena.mem_arena, temp_mem_arena, temp_mem_arena, arrangement, atlas_rgbas, o_unsupported_code_pts);

        if (o_load_from_raw_res) {
            *o_load_from_raw_res = res;
        }

        if (res != ek_font_load_from_raw_result_success) {
            return false;
        }

        s_array<t_gl_id> atlas_gl_ids;

        if (!MakeFontAtlasGLTextures(atlas_rgbas, atlas_gl_ids)) {
            return false;
        }

        o_font = PushGFXResource(res_arena);

        if (!o_font) {
            return false;
        }

        o_font->type = ek_gfx_resource_type_font;
        o_font->type_data.font = {.arrangement = arrangement, .atlas_gl_ids = atlas_gl_ids};

        return true;
    }

    t_b8 LoadFontFromPacked(const s_str_rdonly file_path, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_font) {
        s_font_arrangement arrangement;
        s_array<t_font_atlas_rgba> atlas_rgbas;

        if (!zf::UnpackFont(file_path, *res_arena.mem_arena, temp_mem_arena, temp_mem_arena, arrangement, atlas_rgbas)) {
            return false;
        }

        s_array<t_gl_id> atlas_gl_ids;

        if (!MakeFontAtlasGLTextures(atlas_rgbas, atlas_gl_ids)) {
            return false;
        }

        o_font = PushGFXResource(res_arena);

        if (!o_font) {
            return false;
        }

        o_font->type = ek_gfx_resource_type_font;
        o_font->type_data.font = {.arrangement = arrangement, .atlas_gl_ids = atlas_gl_ids};

        return true;
    }

    t_b8 MakeFontAtlasGLTextures(const s_array_rdonly<t_font_atlas_rgba> atlas_rgbas, s_array<t_gl_id>& o_gl_ids) {
        o_gl_ids = {
            .buf_raw = static_cast<t_gl_id*>(malloc(static_cast<size_t>(ZF_SIZE_OF(t_gl_id) * atlas_rgbas.len))),
            .len = atlas_rgbas.len
        };

        if (!o_gl_ids.buf_raw) {
            return false;
        }

        for (t_size i = 0; i < atlas_rgbas.len; i++) {
            o_gl_ids[i] = MakeGLTexture({g_font_atlas_size, atlas_rgbas[i]});

            if (!o_gl_ids[i]) {
                return false;
            }
        }

        return true;
    }

    t_b8 MakeSurface(const s_v2<t_s32> size, s_gfx_resource_arena& res_arena, s_gfx_resource*& o_surf) {
        t_gl_id fb_gl_id;
        glGenFramebuffers(1, &fb_gl_id);

        t_gl_id tex_gl_id;
        glGenTextures(1, &tex_gl_id);

        if (!AttachGLFramebufferTexture(fb_gl_id, tex_gl_id, size)) {
            return false;
        }

        o_surf = PushGFXResource(res_arena);

        if (!o_surf) {
            glDeleteTextures(1, &tex_gl_id);
            glDeleteFramebuffers(1, &fb_gl_id);
            return false;
        }

        o_surf->type = ek_gfx_resource_type_surface;

        o_surf->type_data.surf = {
            .fb_gl_id = fb_gl_id,
            .tex_gl_id = tex_gl_id,
            .size = size
        };

        return true;
    }

    t_b8 ResizeSurface(s_gfx_resource* const surf, const s_v2<t_s32> size) {
        ZF_ASSERT(surf && surf->type == ek_gfx_resource_type_surface);
        ZF_ASSERT(surf->type_data.surf.size != size && "Unnecessarily resizing a surface - new surface size is the same.");

        t_gl_id new_fb_gl_id;
        glGenFramebuffers(1, &new_fb_gl_id);

        t_gl_id new_tex_gl_id;
        glGenTextures(1, &new_tex_gl_id);

        if (!AttachGLFramebufferTexture(new_fb_gl_id, new_tex_gl_id, size)) {
            glDeleteTextures(1, &new_tex_gl_id);
            glDeleteFramebuffers(1, &new_fb_gl_id);
            return false;
        }

        glDeleteTextures(1, &surf->type_data.surf.tex_gl_id);
        glDeleteFramebuffers(1, &surf->type_data.surf.fb_gl_id);

        surf->type_data.surf = {
            .fb_gl_id = new_fb_gl_id,
            .tex_gl_id = new_tex_gl_id,
            .size = size
        };

        return true;
    }

    t_b8 MakeSurfaceShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_prog) {
        const t_gl_id gl_id = MakeGLShaderProg(vert_src, frag_src, temp_mem_arena);

        if (!gl_id) {
            return false;
        }

        o_prog = PushGFXResource(res_arena);

        if (!o_prog) {
            return false;
        }

        o_prog->type = ek_gfx_resource_type_surface_shader_prog;
        o_prog->type_data.surf_shader_prog = {.gl_id = gl_id};

        return true;
    }

    // ============================================================
    // @section: Rendering
    // ============================================================
    t_b8 MakeRenderingBasis(s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena, s_rendering_basis& o_basis) {
        ZeroOut(o_basis);

        t_b8 clean_up = false;

        // Create the batch mesh.
        {
            s_array<t_u16> elems;

            if (!MakeArray(temp_mem_arena, g_batch_slot_elem_cnt * g_batch_slot_cnt, elems)) {
                ZF_REPORT_ERROR();
                clean_up = true;
                return false;
            }

            for (t_size i = 0; i < g_batch_slot_cnt; i++) {
                elems[(i * 6) + 0] = static_cast<t_u16>((i * 4) + 0);
                elems[(i * 6) + 1] = static_cast<t_u16>((i * 4) + 1);
                elems[(i * 6) + 2] = static_cast<t_u16>((i * 4) + 2);
                elems[(i * 6) + 3] = static_cast<t_u16>((i * 4) + 2);
                elems[(i * 6) + 4] = static_cast<t_u16>((i * 4) + 3);
                elems[(i * 6) + 5] = static_cast<t_u16>((i * 4) + 0);
            }

            constexpr t_size verts_len = g_batch_vert_component_cnt * g_batch_slot_vert_cnt * g_batch_slot_cnt;
            o_basis.batch_mesh_gl_ids = MakeGLMesh(nullptr, verts_len, elems, g_batch_vert_attr_lens);
        }

        ZF_DEFER({
            if (clean_up) {
                ReleaseGLMesh(o_basis.batch_mesh_gl_ids);
            }
        });

        // Create the batch shader program.
        o_basis.batch_shader_prog_gl_id = MakeGLShaderProg(g_batch_vert_shader_src, g_batch_frag_shader_src, temp_mem_arena);

        if (!o_basis.batch_shader_prog_gl_id) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        ZF_DEFER({
            if (clean_up) {
                glDeleteProgram(o_basis.batch_shader_prog_gl_id);
            }
        });

        // Create the surface mesh.
        {
            constexpr s_static_array<t_f32, 16> verts = {{
                0.0f, 1.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 0.0f,
                1.0f, 0.0f, 1.0f, 1.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            }};

            constexpr s_static_array<t_u16, 6> elems = {{
                0, 1, 2,
                2, 3, 0
            }};

            constexpr s_static_array<t_s32, 2> vert_attr_lens = {{2, 2}};

            o_basis.surf_mesh_gl_ids = MakeGLMesh(verts.buf_raw, verts.g_len, elems, vert_attr_lens);
        }

        ZF_DEFER({
            if (clean_up) {
                ReleaseGLMesh(o_basis.surf_mesh_gl_ids);
            }
        });

        // Set up resource arena.
        o_basis.res_arena = MakeGFXResourceArena(mem_arena);

        ZF_DEFER({
            if (clean_up) {
                ReleaseGFXResources(o_basis.res_arena);
            }
        });

        // Set up pixel texture.
        {
            const s_static_array<t_u8, 4> rgba = {{255, 255, 255, 255}};

            if (!LoadTexture({{1, 1}, rgba}, o_basis.res_arena, o_basis.px_tex)) {
                ZF_REPORT_ERROR();
                clean_up = true;
                return false;
            }
        }

        // Set up default surface shader program.
        if (!MakeSurfaceShaderProg(g_default_surface_vert_shader_src, g_default_surface_frag_shader_src, o_basis.res_arena, temp_mem_arena, o_basis.default_surf_shader_prog)) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        return true;
    }

    void ReleaseRenderingBasis(s_rendering_basis& basis) {
        ReleaseGFXResources(basis.res_arena);
        ReleaseGLMesh(basis.surf_mesh_gl_ids);
        glDeleteProgram(basis.batch_shader_prog_gl_id);
        ReleaseGLMesh(basis.batch_mesh_gl_ids);
    }

    void Flush(const s_rendering_context& rc) {
        if (rc.state->batch_slots_used_cnt == 0) {
            // Nothing to flush!
            return;
        }

        //
        // Submitting Vertex Data to GPU
        //
        glBindVertexArray(rc.basis->batch_mesh_gl_ids.vert_arr);
        glBindBuffer(GL_ARRAY_BUFFER, rc.basis->batch_mesh_gl_ids.vert_buf);

        {
            const t_size write_size = ZF_SIZE_OF(t_batch_slot) * rc.state->batch_slots_used_cnt;
            glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, rc.state->batch_slots.buf_raw);
        }

        //
        // Rendering the Batch
        //
        glUseProgram(rc.basis->batch_shader_prog_gl_id);

        const t_s32 view_uniform_loc = glGetUniformLocation(rc.basis->batch_shader_prog_gl_id, "u_view");
        glUniformMatrix4fv(view_uniform_loc, 1, false, reinterpret_cast<const t_f32*>(&rc.state->view_mat));

        const s_rect<t_s32> viewport = GLViewport(); // @todo: Cache this.
        const auto proj_mat = MakeOrthographicMatrix4x4(0.0f, static_cast<t_f32>(viewport.width), static_cast<t_f32>(viewport.height), 0.0f, -1.0f, 1.0f);
        const t_s32 proj_uniform_loc = glGetUniformLocation(rc.basis->batch_shader_prog_gl_id, "u_proj");
        glUniformMatrix4fv(proj_uniform_loc, 1, false, reinterpret_cast<const t_f32*>(&proj_mat));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rc.state->tex_gl_id);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rc.basis->batch_mesh_gl_ids.elem_buf);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_batch_slot_elem_cnt * rc.state->batch_slots_used_cnt), GL_UNSIGNED_SHORT, nullptr);

        glUseProgram(0);

        rc.state->batch_slots_used_cnt = 0;
    }

    void Clear(const s_rendering_context& rc, const s_color_rgba32f col) {
        ZF_ASSERT(IsColorValid(col));

        glClearColor(col.r, col.g, col.b, col.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void SetViewMatrix(const s_rendering_context& rc, const s_matrix_4x4& mat) {
        Flush(rc);
        rc.state->view_mat = mat;
    }

    void Draw(const s_rendering_context& rc, const t_gl_id tex_gl_id, const s_rect<t_f32> tex_coords, s_v2<t_f32> pos, s_v2<t_f32> size, s_v2<t_f32> origin, const t_f32 rot, const s_color_rgba32f blend) {
        if (rc.state->batch_slots_used_cnt == 0) {
            // This is the first draw to the batch, so set the texture associated with the batch to the one we're trying to render.
            rc.state->tex_gl_id = tex_gl_id;
        } else if (rc.state->batch_slots_used_cnt == g_batch_slot_cnt || tex_gl_id != rc.state->tex_gl_id) {
            // Flush the batch and then try this same render operation again but on a fresh batch.
            Flush(rc);
            Draw(rc, tex_gl_id, tex_coords, pos, size, origin, rot, blend);
            return;
        }

        // Write the vertex data to the next slot.
        const s_static_array<s_v2<t_f32>, 4> vert_coords = {{
            {0.0f - origin.x, 0.0f - origin.y},
            {1.0f - origin.x, 0.0f - origin.y},
            {1.0f - origin.x, 1.0f - origin.y},
            {0.0f - origin.x, 1.0f - origin.y}
        }};

        const s_static_array<s_v2<t_f32>, 4> tex_coords_per_vert = {{
            {RectLeft(tex_coords), RectTop(tex_coords)},
            {RectRight(tex_coords), RectTop(tex_coords)},
            {RectRight(tex_coords), RectBottom(tex_coords)},
            {RectLeft(tex_coords), RectBottom(tex_coords)}
        }};

        t_batch_slot& slot = rc.state->batch_slots[rc.state->batch_slots_used_cnt];

        for (t_size i = 0; i < slot.g_len; i++) {
            slot[i] = {
                .vert_coord = vert_coords[i],
                .pos = pos,
                .size = size,
                .rot = rot,
                .tex_coord = tex_coords_per_vert[i],
                .blend = blend
            };
        }

        // Update the count - we've used a slot!
        rc.state->batch_slots_used_cnt++;
    }

    void DrawTexture(const s_rendering_context& rc, const s_gfx_resource* const tex, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect, const s_v2<t_f32> origin, const s_v2<t_f32> scale, const t_f32 rot, const s_color_rgba32f blend) {
        ZF_ASSERT(tex && tex->type == ek_gfx_resource_type_texture);
        ZF_ASSERT(IsOriginValid(origin));
        ZF_ASSERT(IsColorValid(blend));

        const auto tex_size = tex->type_data.tex.size;

        s_rect<t_s32> src_rect_to_use;

        if (src_rect == s_rect<t_s32>()) {
            // If the source rectangle wasn't set, just go with the whole texture.
            src_rect_to_use = {0, 0, tex_size.x, tex_size.y};
        } else {
            ZF_ASSERT(RectLeft(src_rect) >= 0 && RectTop(src_rect) >= 0 && RectRight(src_rect) <= tex_size.x && RectTop(src_rect) <= tex_size.y);
            src_rect_to_use = src_rect;
        }

        const s_rect tex_coords = CalcTextureCoords(src_rect_to_use, tex_size);

        const s_v2<t_f32> size = {
            static_cast<t_f32>(src_rect_to_use.width) * scale.x, static_cast<t_f32>(src_rect_to_use.height) * scale.y
        };

        Draw(rc, tex->type_data.tex.gl_id, tex_coords, pos, size, origin, rot, blend);
    }

    void DrawRect(const s_rendering_context& rc, const s_rect<t_f32> rect, const s_color_rgba32f color) {
        DrawTexture(rc, rc.basis->px_tex, RectPos(rect), {}, {}, RectSize(rect), 0.0f, color);
    }

    void DrawRectOpaqueOutlined(const s_rendering_context& rc, const s_rect<t_f32> rect, const s_color_rgba24f fill_color, const s_color_rgba32f outline_color, const t_f32 outline_thickness) {
        ZF_ASSERT(outline_thickness > 0.0f);

        DrawRect(rc, {rect.x - outline_thickness, rect.y - outline_thickness, rect.width + (outline_thickness * 2.0f), rect.height + (outline_thickness * 2.0f)}, fill_color);
        DrawRect(rc, rect, fill_color);
    }

    void DrawRectRot(const s_rendering_context& rc, const s_v2<t_f32> pos, const s_v2<t_f32> size, const s_v2<t_f32> origin, const t_f32 rot, const s_color_rgba32f color) {
        DrawTexture(rc, rc.basis->px_tex, pos, {}, origin, size, rot, color);
    }

    void DrawRectRotOpaqueOutlined(const s_rendering_context& rc, const s_v2<t_f32> pos, const s_v2<t_f32> size, const s_v2<t_f32> origin, const t_f32 rot, const s_color_rgba24f fill_color, const s_color_rgba32f outline_color, const t_f32 outline_thickness) {
        ZF_ASSERT(outline_thickness > 0.0f);

        DrawRectRot(rc, pos, {size.x + (outline_thickness * 2.0f), size.y + (outline_thickness * 2.0f)}, origin, rot, outline_color);
        DrawRectRot(rc, pos, size, origin, rot, fill_color);
    }

    void DrawLine(const s_rendering_context& rc, const s_v2<t_f32> a, const s_v2<t_f32> b, const s_color_rgba32f blend, const t_f32 thickness) {
        ZF_ASSERT(thickness > 0.0f);

        const t_f32 len = CalcDist(a, b);
        const t_f32 dir = CalcDirInRads(a, b);
        DrawTexture(rc, rc.basis->px_tex, a, {}, origins::g_centerleft, {len, thickness}, dir, blend);
    }

    t_b8 DrawStr(const s_rendering_context& rc, const s_str_rdonly str, const s_gfx_resource* const font, const s_v2<t_f32> pos, const s_v2<t_f32> alignment, const s_color_rgba32f blend, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsValidUTF8Str(str));
        ZF_ASSERT(font && font->type == ek_gfx_resource_type_font);
        ZF_ASSERT(IsAlignmentValid(alignment));
        ZF_ASSERT(IsColorValid(blend));

        if (IsStrEmpty(str)) {
            return true;
        }

        const auto& font_arrangement = font->type_data.font.arrangement;
        const auto& font_atlas_gl_ids = font->type_data.font.atlas_gl_ids;

        s_array<s_v2<t_f32>> chr_positions;

        if (!LoadStrChrDrawPositions(str, font_arrangement, pos, alignment, temp_mem_arena, chr_positions)) {
            return false;
        }

        t_size chr_index = 0;

        ZF_WALK_STR(str, chr_info) {
            if (chr_info.code_pt == ' ' || chr_info.code_pt == '\n') {
                chr_index++;
                continue;
            }

            s_font_glyph_info glyph_info;

            if (!HashMapGet(font_arrangement.code_pts_to_glyph_infos, chr_info.code_pt, &glyph_info)) {
                ZF_ASSERT(false);
                return false;
            }

            const auto chr_tex_coords = CalcTextureCoords(glyph_info.atlas_rect, g_font_atlas_size);

            Draw(rc, font_atlas_gl_ids[glyph_info.atlas_index], chr_tex_coords, chr_positions[chr_index], static_cast<s_v2<t_f32>>(RectSize(glyph_info.atlas_rect)), {}, 0.0f, blend);

            chr_index++;
        };

        return true;
    }

    void SetSurface(const s_rendering_context& rc, const s_gfx_resource* const surf) {
        ZF_ASSERT(surf && surf->type == ek_gfx_resource_type_surface);

        if (IsStackFull(rc.state->surfs)) {
            LogError("Attempting to set a surface even though the limit has been reached!");
            ZF_REPORT_ERROR();
            return;
        }

        Flush(rc);

        glBindFramebuffer(GL_FRAMEBUFFER, surf->type_data.surf.fb_gl_id);
        glViewport(0, 0, static_cast<GLsizei>(surf->type_data.surf.size.x), static_cast<GLsizei>(surf->type_data.surf.size.y));

        StackPush(rc.state->surfs, surf);
    }

    void UnsetSurface(const s_rendering_context& rc) {
        if (IsStackEmpty(rc.state->surfs)) {
            LogError("Attempting to unset a surface even though none are set!");
            ZF_REPORT_ERROR();
            return;
        }

        Flush(rc);

        StackPop(rc.state->surfs);

        t_gl_id fb_gl_id;
        s_v2<t_s32> viewport_size;

        if (IsStackEmpty(rc.state->surfs)) {
            fb_gl_id = 0;
            viewport_size = WindowFramebufferSize();
        } else {
            const auto new_surf = StackTop(rc.state->surfs);
            fb_gl_id = new_surf->type_data.surf.fb_gl_id;
            viewport_size = new_surf->type_data.surf.size;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id);
        glViewport(0, 0, static_cast<GLsizei>(viewport_size.x), static_cast<GLsizei>(viewport_size.y));
    }

    void SetSurfaceShaderProg(const s_rendering_context& rc, const s_gfx_resource* const prog) {
        ZF_ASSERT(CurGLShaderProg() == 0 && "Potential attempted double-assignment of surface shader program?");
        ZF_ASSERT(prog && prog->type == ek_gfx_resource_type_surface_shader_prog);
        glUseProgram(prog->type_data.surf_shader_prog.gl_id);
    }

    t_b8 SetSurfaceShaderProgUniform(const s_rendering_context& rc, const s_str_rdonly name, const s_surface_shader_prog_uniform_val& val, s_mem_arena& temp_mem_arena) {
        const t_gl_id cur_prog_gl_id = CurGLShaderProg();

        ZF_ASSERT(cur_prog_gl_id != 0 && "Surface shader program must be set before setting uniforms!");

        s_str name_terminated;

        if (!CloneStrButAddTerminator(name, temp_mem_arena, name_terminated)) {
            return false;
        }

        const t_s32 loc = glGetUniformLocation(cur_prog_gl_id, StrRaw(name_terminated));
        ZF_ASSERT(loc != -1 && "Failed to get location of shader uniform!");

        switch (val.type) {
            case ek_surface_shader_prog_uniform_val_type_s32:
                glUniform1i(loc, val.type_data.s32);
                break;

            case ek_surface_shader_prog_uniform_val_type_u32:
                glUniform1ui(loc, val.type_data.u32);
                break;

            case ek_surface_shader_prog_uniform_val_type_f32:
                glUniform1f(loc, val.type_data.f32);
                break;

            case ek_surface_shader_prog_uniform_val_type_v2:
                glUniform2f(loc, val.type_data.v2.x, val.type_data.v2.y);
                break;

            case ek_surface_shader_prog_uniform_val_type_v3:
                glUniform3f(loc, val.type_data.v3.x, val.type_data.v3.y, val.type_data.v3.z);
                break;

            case ek_surface_shader_prog_uniform_val_type_v4:
                glUniform4f(loc, val.type_data.v4.x, val.type_data.v4.y, val.type_data.v4.z, val.type_data.v4.w);
                break;

            case ek_surface_shader_prog_uniform_val_type_mat4x4:
                glUniformMatrix4fv(loc, 1, false, reinterpret_cast<const t_f32*>(&val.type_data.mat4x4));
                break;
        }

        return true;
    }

    void DrawSurface(const s_rendering_context& rc, const s_gfx_resource* const surf, const s_v2<t_f32> pos) {
        ZF_ASSERT(surf && surf->type == ek_gfx_resource_type_surface);

        if (CurGLShaderProg() == 0) {
            glUseProgram(rc.basis->default_surf_shader_prog->type_data.surf_shader_prog.gl_id);
        }

        glBindVertexArray(rc.basis->surf_mesh_gl_ids.vert_arr);
        glBindBuffer(GL_ARRAY_BUFFER, rc.basis->surf_mesh_gl_ids.vert_buf);

        {
            constexpr s_v2<t_f32> scale = {1.0f, 1.0f}; // @todo: Make this customisable, or remove entirely.

            constexpr s_static_array<t_f32, 16> verts = {{
                0.0f,    scale.y, 0.0f, 0.0f,
                scale.x, scale.y, 1.0f, 0.0f,
                scale.x, 0.0f,    1.0f, 1.0f,
                0.0f,    0.0f,    0.0f, 1.0f
            }};

            glBufferSubData(GL_ARRAY_BUFFER, 0, ArraySizeInBytes(verts), verts.buf_raw);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, surf->type_data.surf.tex_gl_id);

        const t_s32 proj_uniform_loc = glGetUniformLocation(CurGLShaderProg(), "u_proj");
        ZF_ASSERT(proj_uniform_loc != -1); // @todo: Remove, do at load time.

        const auto viewport = GLViewport();
        const s_matrix_4x4 proj_mat = MakeOrthographicMatrix4x4(0.0f, static_cast<t_f32>(viewport.width), static_cast<t_f32>(viewport.height), 0.0f, -1.0f, 1.0f);
        glUniformMatrix4fv(proj_uniform_loc, 1, false, reinterpret_cast<const t_f32*>(&proj_mat));

        const t_s32 pos_uniform_loc = glGetUniformLocation(CurGLShaderProg(), "u_pos");
        ZF_ASSERT(pos_uniform_loc != -1); // @todo: Remove, do at load time.

        glUniform2fv(pos_uniform_loc, 1, reinterpret_cast<const t_f32*>(&pos));

        const t_s32 size_uniform_loc = glGetUniformLocation(CurGLShaderProg(), "u_size");
        ZF_ASSERT(size_uniform_loc != -1); // @todo: Remove, do at load time.

        const auto size_f32 = static_cast<s_v2<t_f32>>(surf->type_data.surf.size);
        glUniform2fv(size_uniform_loc, 1, reinterpret_cast<const t_f32*>(&size_f32));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rc.basis->surf_mesh_gl_ids.elem_buf);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

        glUseProgram(0);
    }

    // ============================================================
    // @section: Audio
    // ============================================================
    t_b8 InitAudio(s_audio_context& o_ac) {
        ZeroOut(o_ac);

        if (ma_engine_init(nullptr, &o_ac.ma_eng) != MA_SUCCESS) {
            ZF_REPORT_ERROR();
            return false;
        }

        return true;
    }

    void ShutdownAudio(s_audio_context& ac) {
        ZF_FOR_EACH_SET_BIT(ac.snd_insts.activity, i) {
            ma_sound_stop(&ac.snd_insts.ma_snds[i]);
            ma_sound_uninit(&ac.snd_insts.ma_snds[i]);

            ma_audio_buffer_ref_uninit(&ac.snd_insts.ma_buf_refs[i]);
        }

        ma_engine_uninit(&ac.ma_eng);
    }

    t_b8 CreateSoundTypeFromRaw(const s_str_rdonly file_path, s_sound_type_arena& type_arena, s_mem_arena& temp_mem_arena, s_sound_type*& o_type) {
        s_sound_meta meta;
        s_array<t_f32> pcm;

        if (!LoadSoundFromRaw(file_path, *type_arena.mem_arena, temp_mem_arena, meta, pcm)) {
            return false;
        }

        o_type = PushToMemArena<s_sound_type>(*type_arena.mem_arena);

        if (!o_type) {
            return false;
        }

        *o_type = {
            .meta = meta,
            .pcm = pcm
        };

        if (!type_arena.head) {
            type_arena.head = o_type;
            type_arena.tail = o_type;
        } else {
            type_arena.tail->next = o_type;
            type_arena.tail = o_type;
        }

        return true;
    }

    void DestroySoundTypes(s_audio_context& ac, s_sound_type_arena& type_arena) {
        const s_sound_type* type = type_arena.head;

        while (type) {
            ZF_FOR_EACH_SET_BIT(ac.snd_insts.activity, i) {
                if (ac.snd_insts.types[i] == type) {
                    StopSound(ac, {i, ac.snd_insts.versions[i]});
                }
            }

            type = type->next;
        }
    }

    t_b8 PlaySound(s_audio_context& ac, const s_sound_type* const type, s_sound_id* const o_id, const t_f32 vol, const t_f32 pan, const t_f32 pitch, const t_b8 loop) {
        ZF_ASSERT(vol >= 0.0f && vol <= 1.0f);
        ZF_ASSERT(pan >= -1.0f && pan <= 1.0f);
        ZF_ASSERT(pitch > 0.0f);

        t_b8 clean_up = false;

        const t_size index = IndexOfFirstUnsetBit(ac.snd_insts.activity);

        if (index == -1) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        ma_sound& ma_snd = ac.snd_insts.ma_snds[index];
        ma_audio_buffer_ref& ma_buf_ref = ac.snd_insts.ma_buf_refs[index];

        ac.snd_insts.types[index] = type;

        if (ma_audio_buffer_ref_init(ma_format_f32, static_cast<ma_uint32>(type->meta.channel_cnt), type->pcm.buf_raw, static_cast<ma_uint64>(type->meta.frame_cnt), &ma_buf_ref) != MA_SUCCESS) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        ZF_DEFER({
            if (clean_up) {
                ma_audio_buffer_ref_uninit(&ma_buf_ref);
            }
        });

        ma_buf_ref.sampleRate = static_cast<ma_uint32>(type->meta.sample_rate);

        if (ma_sound_init_from_data_source(&ac.ma_eng, &ma_buf_ref, 0, nullptr, &ma_snd) != MA_SUCCESS) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        ZF_DEFER({
            if (clean_up) {
                ma_sound_uninit(&ma_snd);
            }
        });

        ma_sound_set_volume(&ma_snd, vol);
        ma_sound_set_pan(&ma_snd, pan);
        ma_sound_set_pitch(&ma_snd, pitch);
        ma_sound_set_looping(&ma_snd, loop);

        if (ma_sound_start(&ma_snd) != MA_SUCCESS) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        SetBit(ac.snd_insts.activity, index);
        ac.snd_insts.versions[index]++;

        if (o_id) {
            *o_id = {index, ac.snd_insts.versions[index]};
        }

        return true;
    }

    void StopSound(s_audio_context& ac, const s_sound_id id) {
        ZF_ASSERT(IsBitSet(ac.snd_insts.activity, id.index) && ac.snd_insts.versions[id.index] == id.version);

        ma_sound_stop(&ac.snd_insts.ma_snds[id.index]);
        ma_sound_uninit(&ac.snd_insts.ma_snds[id.index]);
        ma_audio_buffer_ref_uninit(&ac.snd_insts.ma_buf_refs[id.index]);

        UnsetBit(ac.snd_insts.activity, id.index);
    }

    t_b8 IsSoundPlaying(s_audio_context& ac, const s_sound_id id) {
        ZF_ASSERT(id.version <= ac.snd_insts.versions[id.index]);

        if (!IsBitSet(ac.snd_insts.activity, id.index) || id.version != ac.snd_insts.versions[id.index]) {
            return false;
        }

        return ma_sound_is_playing(&ac.snd_insts.ma_snds[id.index]);
    }

    void ProcFinishedSounds(s_audio_context& ac) {
        ZF_FOR_EACH_SET_BIT(ac.snd_insts.activity, i) {
            ma_sound& snd = ac.snd_insts.ma_snds[i];

            if (!ma_sound_is_playing(&snd)) {
                ma_sound_uninit(&snd);
                ma_audio_buffer_ref_uninit(&ac.snd_insts.ma_buf_refs[i]);

                UnsetBit(ac.snd_insts.activity, i);
            }
        }
    }
}
