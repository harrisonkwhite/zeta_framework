#include <zf/rendering.h>

namespace zf {
    static s_gfx_resource_handle MakeBatchMesh(c_gfx_resource_arena& gfx_res_arena, c_mem_arena& temp_mem_arena) {
        const int verts_len = g_batch_slot_vert_len * g_batch_slot_vert_cnt * g_batch_slot_cnt;

        c_array<unsigned short> elems;

        const bool elems_make_failed = [&elems, &temp_mem_arena]() {
            if (!elems.Init(temp_mem_arena, g_batch_slot_elem_cnt * g_batch_slot_cnt)) {
                ZF_LOG_ERROR("Failed to reserve memory for batch renderable elements!");
                return false;
            }

            for (int i = 0; i < g_batch_slot_cnt; i++) {
                elems[(i * 6) + 0] = (i * 4) + 0;
                elems[(i * 6) + 1] = (i * 4) + 1;
                elems[(i * 6) + 2] = (i * 4) + 2;
                elems[(i * 6) + 3] = (i * 4) + 2;
                elems[(i * 6) + 4] = (i * 4) + 3;
                elems[(i * 6) + 5] = (i * 4) + 0;
            }

            return true;
        }();

        const s_static_array<int, 6> vert_attr_lens = {{2, 2, 2, 1, 2, 4}};

        return gfx_res_arena.AddMesh(nullptr, verts_len, elems, vert_attr_lens);
    }

    bool s_rendering_basis::Init(c_gfx_resource_arena& gfx_res_arena, c_mem_arena& temp_mem_arena) {
        // Generate the batch mesh.
        batch_mesh_hdl = MakeBatchMesh(gfx_res_arena, temp_mem_arena);

        if (!batch_mesh_hdl.IsValid()) {
            return false;
        }

        // Generate the pixel texture.
        {
            const s_static_array<t_byte, 4> px_rgba = {
                {255, 255, 255, 255}
            };

            px_tex_hdl = gfx_res_arena.AddTexture({ {1, 1}, px_rgba });

            if (!px_tex_hdl.IsValid()) {
                return false;
            }
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
