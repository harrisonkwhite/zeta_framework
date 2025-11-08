#include <zf/rendering.h>

namespace zf {
    bool s_rendering_basis::Init(c_gfx_resource_arena& gfx_res_arena, c_mem_arena& temp_mem_arena) {
        // Generate the pixel texture.
        const s_static_array<t_byte, 4> px_rgba = {
            {255, 255, 255, 255}
        };

        px_tex_hdl = gfx_res_arena.AddTexture({{1, 1}, px_rgba});

        if (!px_tex_hdl.IsValid()) {
            return false;
        }

        return true;

#if 0
        if (!InitTextureGroup(&basis->builtin_textures, eks_builtin_texture_cnt, BuiltinTextureRGBAGenerator, mem_arena, gl_res_arena, temp_mem_arena)) {
            LOG_ERROR("Failed to generate built-in textures for rendering basis!");
            return false;
        }

        {
            const s_shader_prog_gen_info gen_infos[] = {
                [ek_builtin_shader_prog_batch] = {
                    .holds_srcs = true,
                    .vert_src = g_batch_vert_shader_src,
                    .frag_src = g_batch_frag_shader_src
                },
                [ek_builtin_shader_prog_surface_default] = {
                    .holds_srcs = true,
                    .vert_src = g_surface_default_vert_shader_src,
                    .frag_src = g_surface_default_frag_shader_src
                },
                [ek_builtin_shader_prog_surface_blend] = {
                    .holds_srcs = true,
                    .vert_src = g_surface_blend_vert_shader_src,
                    .frag_src = g_surface_blend_frag_shader_src
                }
            };

            if (!InitShaderProgGroup(&basis->builtin_shader_progs, (s_shader_prog_gen_info_array_view)ARRAY_FROM_STATIC(gen_infos), gl_res_arena, temp_mem_arena)) {
                LOG_ERROR("Failed to generate built-in shader programs for rendering basis!");
                return false;
            }
        }

        for (t_s32 i = 0; i < eks_renderable_cnt; i++) {
            s_renderable* const renderable = STATIC_ARRAY_ELEM(basis->renderables, i);

            *renderable = GenRenderableOfType(gl_res_arena, i, temp_mem_arena);

            if (IS_ZERO(*renderable)) {
                LOG_ERROR("Failed to generate renderable of index %d for rendering basis!", i);
                return false;
            }
        }
#endif
    }
}
