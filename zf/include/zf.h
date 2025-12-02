#pragma once

#include <zc.h>

namespace zf {
    struct s_game_init_context {
        void* dev_mem;

        s_mem_arena* mem_arena;
        s_mem_arena* temp_mem_arena;
    };

    struct s_game_tick_context {
        void* dev_mem;

        s_mem_arena* mem_arena;
        s_mem_arena* temp_mem_arena;
    };

    struct s_rendering_context;

    struct s_game_render_context {
        const void* dev_mem;

        s_mem_arena* mem_arena;
        s_mem_arena* temp_mem_arena;

        s_rendering_context* rendering_context;
    };

    enum class e_window_init_flags : t_u8;

    struct s_game_info {
        t_size mem_arena_size;
        t_size temp_mem_arena_size;

        s_v2<t_s32> window_init_size;
        s_str_rdonly window_init_title;

        t_size dev_mem_size;
        t_size dev_mem_alignment;

        t_s32 targ_ticks_per_sec;

        // Below are pointers to functions that the framework will call for you. The provided struct pointers expose parts of the framework state for you to work with.
        t_b8 (* init_func)(const s_game_init_context& zf_context); // Called once as one of the last steps of the game initialisation phase.
        t_b8 (* tick_func)(const s_game_tick_context& zf_context); // Called once every tick (which can occur multiple times a frame).
        t_b8 (* render_func)(const s_game_render_context& zf_context); // Called after all ticks of the frame have been run.
        void (* clean_func)(void* const dev_mem); // Optional. Called when the game ends (including if it ends in error). This is not called if your initialisation function failed or hasn't yet been called.
    };

    inline void AssertGameInfoValidity(const s_game_info& info) {
        ZF_ASSERT(info.mem_arena_size > 0);
        ZF_ASSERT(info.temp_mem_arena_size > 0 && info.temp_mem_arena_size <= info.mem_arena_size);

        ZF_ASSERT(info.window_init_size.x > 0 && info.window_init_size.y > 0);

        ZF_ASSERT((info.dev_mem_size == 0 && info.dev_mem_alignment == 0)
            || (info.dev_mem_size > 0 && IsAlignmentValid(info.dev_mem_alignment)));

        ZF_ASSERT(info.targ_ticks_per_sec > 0);

        ZF_ASSERT(info.init_func);
        ZF_ASSERT(info.tick_func);
        ZF_ASSERT(info.render_func);
    }

    t_b8 RunGame(const s_game_info& info);

    // ============================================================
    // @section: Window
    // ============================================================
    enum class e_window_init_flags : t_u8 {
        none = 0,
        resizable = 1 << 0,
        hide_cursor = 1 << 1
    };

    s_v2<t_s32> WindowSize();
    s_v2<t_s32> WindowFramebufferSize();

    // ============================================================
    // @section: Input
    // ============================================================
    enum e_key_code : t_s32 {
        eks_key_code_none = -1,

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
    };

    enum e_mouse_button_code : t_s32 {
        eks_mouse_button_code_none = -1,

        ek_mouse_button_code_left,
        ek_mouse_button_code_right,
        ek_mouse_button_code_middle,

        eks_mouse_button_code_cnt
    };

    t_b8 IsKeyDown(const e_key_code kc);
    t_b8 IsKeyPressed(const e_key_code kc);
    t_b8 IsKeyReleased(const e_key_code kc);

    t_b8 IsMouseButtonDown(const e_mouse_button_code mbc);
    t_b8 IsMouseButtonPressed(const e_mouse_button_code mbc);
    t_b8 IsMouseButtonReleased(const e_mouse_button_code mbc);

    s_v2<t_f32> MousePos();

    // ============================================================
    // @section: GFX Resources
    // ============================================================
    enum e_gfx_resource_type {
        ek_gfx_resource_type_invalid,
        ek_gfx_resource_type_texture,
        ek_gfx_resource_type_font,
        ek_gfx_resource_type_surface,
        ek_gfx_resource_type_surface_shader_prog
    };

    struct s_gfx_resource;

    struct s_gfx_resource_arena {
        s_mem_arena* mem_arena;
        s_gfx_resource* head;
        s_gfx_resource* tail;
    };

    inline s_gfx_resource_arena MakeGFXResourceArena(s_mem_arena& mem_arena) {
        return {
            .mem_arena = &mem_arena
        };
    }

    void ReleaseGFXResources(const s_gfx_resource_arena& res_arena);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 LoadTexture(const s_rgba_texture_data_rdonly& tex_data, s_gfx_resource*& o_tex, s_gfx_resource_arena* const res_arena = nullptr);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] inline t_b8 LoadTextureFromRaw(const s_str_rdonly file_path, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_tex, s_gfx_resource_arena* const res_arena = nullptr) {
        s_rgba_texture_data tex_data;

        if (!LoadRGBATextureDataFromRaw(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTexture(tex_data, o_tex, res_arena);
    }

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] inline t_b8 LoadTextureFromPacked(const s_str_rdonly file_path, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_tex, s_gfx_resource_arena* const res_arena = nullptr) {
        s_rgba_texture_data tex_data;

        if (!UnpackTexture(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTexture(tex_data, o_tex, res_arena);
    }

    s_v2<t_s32> TextureSize(const s_gfx_resource* const res);

    // ============================================================
    // @section: Rendering
    // ============================================================
    void Clear(const s_color_rgba32f col = {});
    void SetViewMatrix(const s_matrix_4x4& mat);
    void DrawTexture(const s_gfx_resource* const tex, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect = {}, const s_v2<t_f32> origin = origins::g_topleft, const s_v2<t_f32> scale = {1.0f, 1.0f}, const t_f32 rot = 0.0f, const s_color_rgba32f blend = colors::g_white);
}
