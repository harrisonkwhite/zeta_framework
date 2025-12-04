#pragma once

#include <zcl.h>

namespace zf {
    // ============================================================
    // @section: Game
    // ============================================================
    struct s_input_state;
    struct s_gfx_resource_arena;
    struct s_audio_context;

    struct s_game_init_context {
        void* dev_mem;

        s_mem_arena* mem_arena;
        s_mem_arena* temp_mem_arena;

        s_gfx_resource_arena* gfx_res_arena;

        s_audio_context* audio_context;
    };

    struct s_game_tick_context {
        void* dev_mem;

        s_mem_arena* mem_arena;
        s_mem_arena* temp_mem_arena;

        const s_input_state* input_state;

        s_gfx_resource_arena* gfx_res_arena;

        s_audio_context* audio_context;
    };

    struct s_rendering_context;

    struct s_game_render_context {
        const void* dev_mem;

        s_mem_arena* mem_arena;
        s_mem_arena* temp_mem_arena;

        const s_rendering_context* rendering_context;
    };

    struct s_game_info {
        t_size mem_arena_size;
        t_size temp_mem_arena_size;
        t_size frame_mem_arena_size;

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
        ZF_ASSERT(info.frame_mem_arena_size > 0 && info.frame_mem_arena_size <= info.mem_arena_size);

        ZF_ASSERT((info.dev_mem_size == 0 && info.dev_mem_alignment == 0)
            || (info.dev_mem_size > 0 && IsAlignmentValid(info.dev_mem_alignment)));

        ZF_ASSERT(info.targ_ticks_per_sec > 0);

        ZF_ASSERT(info.init_func);
        ZF_ASSERT(info.tick_func);
        ZF_ASSERT(info.render_func);
    }

    t_b8 RunGame(const s_game_info& info);

    void SetWindowTitle(const s_str_rdonly title);

    s_v2<t_s32> WindowSize();
    void SetWindowSize(const s_v2<t_s32> size);
    void SetWindowResizability(const t_b8 resizable);

    s_v2<t_s32> WindowFramebufferSize();

    void SetCursorVisibility(const t_b8 visible);

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

    struct s_input_state;

    t_b8 IsKeyDown(const s_input_state& is, const e_key_code kc);
    t_b8 IsKeyPressed(const s_input_state& is, const e_key_code kc);
    t_b8 IsKeyReleased(const s_input_state& is, const e_key_code kc);

    t_b8 IsMouseButtonDown(const s_input_state& is, const e_mouse_button_code mbc);
    t_b8 IsMouseButtonPressed(const s_input_state& is, const e_mouse_button_code mbc);
    t_b8 IsMouseButtonReleased(const s_input_state& is, const e_mouse_button_code mbc);

    s_v2<t_f32> CursorPos(const s_input_state& is);

    // +Y: Scroll up / away from you
    // -Y: Scroll down / towards you
    // +X: Scroll right
    // +X: Scroll left
    s_v2<t_f32> GetScroll(const s_input_state& is);

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

    // The memory lifetime of the given memory arena must encompass that of the GFX resource arena.
    inline s_gfx_resource_arena MakeGFXResourceArena(s_mem_arena& mem_arena) {
        return {
            .mem_arena = &mem_arena
        };
    }

    void ReleaseGFXResources(const s_gfx_resource_arena& res_arena);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 LoadTexture(const s_rgba_texture_data_rdonly& tex_data, s_gfx_resource_arena& res_arena, s_gfx_resource*& o_tex);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] inline t_b8 LoadTextureFromRaw(const s_str_rdonly file_path, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_tex) {
        s_rgba_texture_data tex_data;

        if (!LoadRGBATextureDataFromRaw(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTexture(tex_data, res_arena, o_tex);
    }

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] inline t_b8 LoadTextureFromPacked(const s_str_rdonly file_path, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_tex) {
        s_rgba_texture_data tex_data;

        if (!UnpackTexture(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTexture(tex_data, res_arena, o_tex);
    }

    s_v2<t_s32> TextureSize(const s_gfx_resource* const res);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 LoadFontFromRaw(const s_str_rdonly file_path, const t_s32 height, const t_unicode_code_pt_bit_vec& code_pts, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_font, e_font_load_from_raw_result* const o_load_from_raw_res = nullptr, t_unicode_code_pt_bit_vec* const o_unsupported_code_pts = nullptr);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 LoadFontFromPacked(const s_str_rdonly file_path, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_font);

    // Returns false on failure. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 MakeSurface(const s_v2<t_s32> size, s_gfx_resource_arena& res_arena, s_gfx_resource*& o_surf);

    // Returns true iff the operation was successful. If it failed, the old surface state with its old size is left intact.
    [[nodiscard]] t_b8 ResizeSurface(s_gfx_resource* const surf, const s_v2<t_s32> size);

    // Returns false on failure. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 MakeSurfaceShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_prog);

    // ============================================================
    // @section: Rendering
    // ============================================================
    void Clear(const s_rendering_context& rc, const s_color_rgba32f col = {});
    void SetViewMatrix(const s_rendering_context& rc, const s_matrix_4x4& mat);
    void DrawTexture(const s_rendering_context& rc, const s_gfx_resource* const tex, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect = {}, const s_v2<t_f32> origin = origins::g_topleft, const s_v2<t_f32> scale = {1.0f, 1.0f}, const t_f32 rot = 0.0f, const s_color_rgba32f blend = colors::g_white);
    void DrawRect(const s_rendering_context& rc, const s_rect<t_f32> rect, const s_color_rgba32f color);
    void DrawRectOpaqueOutlined(const s_rendering_context& rc, const s_rect<t_f32> rect, const s_color_rgba24f fill_color, const s_color_rgba32f outline_color, const t_f32 outline_thickness = 1.0f);
    void DrawRectRot(const s_rendering_context& rc, const s_v2<t_f32> pos, const s_v2<t_f32> size, const s_v2<t_f32> origin, const t_f32 rot, const s_color_rgba32f color);
    void DrawRectRotOpaqueOutlined(const s_rendering_context& rc, const s_v2<t_f32> pos, const s_v2<t_f32> size, const s_v2<t_f32> origin, const t_f32 rot, const s_color_rgba24f fill_color, const s_color_rgba32f outline_color, const t_f32 outline_thickness = 1.0f);
    void DrawLine(const s_rendering_context& rc, const s_v2<t_f32> a, const s_v2<t_f32> b, const s_color_rgba32f blend, const t_f32 thickness = 1.0f);
    [[nodiscard]] t_b8 DrawStr(const s_rendering_context& rc, const s_str_rdonly str, const s_gfx_resource* const font, const s_v2<t_f32> pos, const s_v2<t_f32> alignment, const s_color_rgba32f blend, s_mem_arena& temp_mem_arena);

    void SetSurface(const s_rendering_context& rc, const s_gfx_resource* const surf);
    void UnsetSurface(const s_rendering_context& rc);

    enum e_surface_shader_prog_uniform_val_type : t_s32 {
        ek_surface_shader_prog_uniform_val_type_s32,
        ek_surface_shader_prog_uniform_val_type_u32,
        ek_surface_shader_prog_uniform_val_type_f32,
        ek_surface_shader_prog_uniform_val_type_v2,
        ek_surface_shader_prog_uniform_val_type_v3,
        ek_surface_shader_prog_uniform_val_type_v4,
        ek_surface_shader_prog_uniform_val_type_mat4x4
    };

    struct s_surface_shader_prog_uniform_val {
        e_surface_shader_prog_uniform_val_type type;

        union {
            t_s32 s32;
            t_u32 u32;
            t_f32 f32;
            s_v2<t_f32> v2;
            s_v3<t_f32> v3;
            s_v4<t_f32> v4;
            s_matrix_4x4 mat4x4;
        } type_data;

        s_surface_shader_prog_uniform_val() = default;
        s_surface_shader_prog_uniform_val(const t_s32 v) : type(ek_surface_shader_prog_uniform_val_type_s32), type_data({.s32 = v}) {};
        s_surface_shader_prog_uniform_val(const t_u32 v) : type(ek_surface_shader_prog_uniform_val_type_u32), type_data({.u32 = v}) {};
        s_surface_shader_prog_uniform_val(const t_f32 v) : type(ek_surface_shader_prog_uniform_val_type_f32), type_data({.f32 = v}) {};
        s_surface_shader_prog_uniform_val(const s_v2<t_f32> v) : type(ek_surface_shader_prog_uniform_val_type_v2), type_data({.v2 = v}) {};
        s_surface_shader_prog_uniform_val(const s_v3<t_f32> v) : type(ek_surface_shader_prog_uniform_val_type_v3), type_data({.v3 = v}) {};
        s_surface_shader_prog_uniform_val(const s_v4<t_f32> v) : type(ek_surface_shader_prog_uniform_val_type_v4), type_data({.v4 = v}) {};
        s_surface_shader_prog_uniform_val(const s_matrix_4x4& v) : type(ek_surface_shader_prog_uniform_val_type_mat4x4), type_data({.mat4x4 = v}) {};
    };

    void SetSurfaceShaderProg(const s_rendering_context& rc, const s_gfx_resource* const prog);
    [[nodiscard]] t_b8 SetSurfaceShaderProgUniform(const s_rendering_context& rc, const s_str_rdonly name, const s_surface_shader_prog_uniform_val& val, s_mem_arena& temp_mem_arena);

    void DrawSurface(const s_rendering_context& rc, const s_gfx_resource* const surf, const s_v2<t_f32> pos);

    // ============================================================
    // @section: Audio
    // ============================================================
    struct s_sound_type;

    struct s_sound_type_arena {
        s_mem_arena* mem_arena;
        s_sound_type* head;
        s_sound_type* tail;
    };

    // The memory lifetime of the given memory arena must encompass that of the sound type arena.
    inline s_sound_type_arena MakeSoundTypeArena(s_mem_arena& mem_arena) {
        return {
            .mem_arena = &mem_arena
        };
    }

    [[nodiscard]] t_b8 CreateSoundTypeFromRaw(const s_str_rdonly file_path, s_sound_type_arena& type_arena, s_mem_arena& temp_mem_arena, s_sound_type*& o_type);

    void DestroySoundTypes(s_audio_context& ac, s_sound_type_arena& type_arena);

    struct s_sound_id;

    [[nodiscard]] t_b8 PlaySound(s_audio_context& ac, const s_sound_type* const type, s_sound_id* const o_id = nullptr, const t_f32 vol = 1.0f, const t_f32 pan = 0.0f, const t_f32 pitch = 1.0f, const t_b8 loop = false);
    void StopSound(s_audio_context& ac, const s_sound_id id);
    t_b8 IsSoundPlaying(s_audio_context& ac, const s_sound_id id);
}
