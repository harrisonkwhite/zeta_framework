#pragma once

#include <climits>
#include <zc.h>
#include "zf_gl.h"

namespace zf {
    using t_texture_group_rgba_loader_func = bool (*)(s_rgba_texture& rgba, const t_s32 tex_index, c_mem_arena& mem_arena);

    class c_texture_group {
    public:
        s_v2_s32 GetTextureSize(const int index);

    private:
        c_array<s_v2_s32> m_sizes;
        c_array<t_gl_id> m_gl_ids;
    };

    class c_font_group {
    public:
        void LoadFromPacked(c_file_reader& file_reader, c_mem_arena& mem_arena, c_rendering_resource_arena& rendering_res_arena);

    private:
        c_array<const s_font_arrangement> m_arrangements;
        c_array<const s_font_texture_meta> m_tex_metas;
        c_array<const t_gl_id> m_tex_gl_ids;
    };

    enum class ec_rendering_resource_type {
        texture
    };

    class c_rendering_resource_arena() {
    public:
        bool Init();
        void Clean();

    private:
    }

    enum e_builtin_texture {
        ek_builtin_texture_pixel,
        eks_builtin_texture_cnt
    };

    struct s_font_group {
        c_array<const s_font_arrangement> arrangements;
        c_array<const s_font_texture_meta> tex_metas;
        c_array<const t_gl_id> tex_gl_ids;
    };

    struct s_shader_prog_group {
        c_array<const t_gl_id> gl_ids;
    };

    struct s_shader_prog_gen_info {
        bool holds_srcs;

        union {
            struct {
                c_string_view file_path;
            };

            struct {
                c_string_view vert_src;
                c_string_view frag_src;
            };
        };
    };

    enum e_shader_prog_uniform_value_type {
        ek_shader_prog_uniform_value_type_s32,
        ek_shader_prog_uniform_value_type_r32,
        ek_shader_prog_uniform_value_type_v2,
        ek_shader_prog_uniform_value_type_v3,
        ek_shader_prog_uniform_value_type_v4,
        ek_shader_prog_uniform_value_type_mat4x4,
    };

    struct s_shader_prog_uniform_value {
        e_shader_prog_uniform_value_type type;

       union {
            t_s32 as_s32;
            float as_r32;
            s_v2 as_v2;
            s_v3 as_v3;
            s_v4 as_v4;
            s_matrix_4x4 as_mat4x4;
        };
    };

    enum e_builtin_shader_prog {
        ek_builtin_shader_prog_batch,
        eks_builtin_shader_prog_cnt
    };

    enum e_renderable {
        ek_renderable_batch,
        eks_renderable_cnt
    };

    struct s_batch_vert {
        s_v2 vert_coord;
        s_v2 pos;
        s_v2 size;
        float rot;
        s_v2 tex_coord;
        s_v4 blend;
    };

    constexpr t_s32 g_batch_slot_cnt = 8192;
    constexpr t_s32 g_batch_slot_vert_len = sizeof(s_batch_vert) / sizeof(float);
    constexpr t_s32 g_batch_slot_vert_cnt = 4;
    constexpr t_s32 g_batch_slot_elem_cnt = 6;
    static_assert(g_batch_slot_elem_cnt * g_batch_slot_cnt <= USHRT_MAX, "Batch slot count is too high!");

    using t_batch_slot = s_static_array<s_batch_vert, g_batch_slot_vert_cnt>;

    struct s_batch_slot_write_info {
        t_gl_id tex_gl_id;
        s_rect_edges tex_coords;
        s_v2 pos;
        s_v2 size;
        s_v2 origin;
        float rot;
        s_v4 blend;
    };

    struct s_batch_state {
        t_batch_slot slots[g_batch_slot_cnt];
        t_s32 num_slots_used;

        t_gl_id tex_gl_id;
    };

    struct s_rendering_basis {
        s_gl_resource_arena& m_gl_res_arena;

        s_texture_group m_builtin_textures;
        s_shader_prog_group m_builtin_shader_progs;

        s_renderable m_gl_renderables[eks_renderable_cnt];

        s_rendering_basis(s_gl_resource_arena& gl_res_arena) : m_gl_res_arena(gl_res_arena) {}

        [[nodiscard]] bool Init(c_mem_arena& mem_arena, c_mem_arena& temp_mem_arena) {
            if (!InitTextureGroup(basis.builtin_textures, eks_builtin_texture_cnt, BuiltinTextureRGBALoader, mem_arena, gl_res_arena, temp_mem_arena)) {
                ZF_LOG_ERROR("Failed to generate built-in textures for rendering basis!");
                return false;
            }

            {
                s_shader_prog_gen_info gen_infos[eks_builtin_shader_prog_cnt];

                gen_infos[ek_builtin_shader_prog_batch] = {
                    .holds_srcs = true,
                    .vert_src = g_batch_vert_shader_src,
                    .frag_src = g_batch_frag_shader_src
                };

                gen_infos[ek_builtin_shader_prog_surface_default] = {
                    .holds_srcs = true,
                    .vert_src = g_surface_default_vert_shader_src,
                    .frag_src = g_surface_default_frag_shader_src
                };

                gen_infos[ek_builtin_shader_prog_surface_blend] = {
                    .holds_srcs = true,
                    .vert_src = g_surface_blend_vert_shader_src,
                    .frag_src = g_surface_blend_frag_shader_src
                };

                if (!InitShaderProgGroup(basis.builtin_shader_progs, {gen_infos, eks_builtin_shader_prog_cnt}, gl_res_arena, temp_mem_arena)) {
                    ZF_LOG_ERROR("Failed to generate built-in shader programs for rendering basis!");
                    return false;
                }
            }

            for (t_s32 i = 0; i < eks_renderable_cnt; i++) {
                if (!GenRenderableOfType(basis.renderables[i], gl_res_arena, static_cast<e_renderable>(i), temp_mem_arena)) {
                    ZF_LOG_ERROR("Failed to generate renderable of index %d for rendering basis!", i);
                    return false;
                }
            }

            return true;
        }
    };

    class c_renderer {
    public:
        c_renderer(const s_rendering_basis& basis, const s_v2_s32 window_size) : m_basis(basis) {
            glViewport(0, 0, window_size.x, window_size.y);
        }

        void Clear(const s_v4 col);
        void SetViewMatrix(const s_matrix_4x4& mat);
        void Render(const s_batch_slot_write_info& write_info);
        void RenderTexture(const s_texture_group& textures, const t_s32 tex_index, const s_v2 pos, const s_rect_s32 src_rect = {}, const s_v2 origin = origins::g_origin_center, const s_v2 scale = {1.0f, 1.0f}, const float rot = 0.0f, const s_v4 blend = colors::g_white);
        void RenderRect(const s_rendering_context& rendering_context, const s_rect rect, const s_v4 color);
        void RenderRectWithOutline(const s_rect rect, const s_v4 fill_color, const s_v4 outline_color, const float outline_thickness);
        void RenderRectWithOutlineAndOpaqueFill(const s_rect rect, const s_v3 fill_color, const s_v4 outline_color, const float outline_thickness);
        void RenderBarHor(const s_rect rect, const float perc, const s_v4 front_color, const s_v4 bg_color);
        void RenderBarVertical(const s_rect rect, const float perc, const s_v4 front_color, const s_v4 bg_color); // @todo: These should be one function with a direction parameter!
        [[nodiscard]] bool RenderStr(const s_rendering_context& rendering_context, const c_string_view str, const s_font_group& fonts, t_s32 font_index, s_v2 pos, s_v2 alignment, s_v4 color, c_mem_arena& temp_mem_arena);
        void RenderLine(const s_rendering_context& rendering_context, s_v2 a, s_v2 b, s_v4 blend, float width);
        void Flush();

    private:
        const s_rendering_basis& m_basis;

        s_batch_state batch;
        s_matrix_4x4 view_mat = s_matrix_4x4::Identity();
    };

    s_rect_edges GenTextureCoords(s_rect_s32 src_rect, s_v2_s32 tex_size);
    [[nodiscard]] bool LoadRGBATextureFromPackedFile(s_rgba_texture& tex, c_string_view file_path, c_mem_arena& mem_arena);
    [[nodiscard]] bool InitTextureGroup(s_texture_group& texture_group, t_s32 tex_cnt, t_texture_group_rgba_loader_func rgba_loader_func, c_mem_arena& mem_arena, s_gl_resource_arena& gl_res_arena, c_mem_arena& temp_mem_arena);

    //
    // zf_fonts.c
    //
    [[nodiscard]] bool InitFontGroupFromFiles(s_font_group& font_group, const c_array<const c_string_view> file_paths, c_mem_arena& mem_arena, s_gl_resource_arena& gl_res_arena, c_mem_arena& temp_mem_arena);
    [[nodiscard]] bool GenStrChrRenderPositions(c_array<s_v2>& positions, c_mem_arena& mem_arena, const c_string_view str, const s_font_group& font_group, t_s32 font_index, s_v2 pos, s_v2 alignment);
    [[nodiscard]] bool GenStrCollider(s_rect& rect, const c_string_view str, const s_font_group& font_group, t_s32 font_index, s_v2 pos, s_v2 alignment, c_mem_arena& temp_mem_arena);

    //
    // zf_shaders.c
    //
    [[nodiscard]] bool InitShaderProgGroup(s_shader_prog_group& prog_group, c_array<const s_shader_prog_gen_info> gen_infos, s_gl_resource_arena& gl_res_arena, c_mem_arena& temp_mem_arena);
}
