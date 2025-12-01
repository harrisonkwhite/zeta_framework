#include <zf/zf_rendering.h>

#include <zf/zf_window.h>

namespace zf {


#if 0
    struct s_rendering_state {
        s_static_array<t_batch_slot, g_batch_slot_cnt> batch_slots;
        t_size batch_slots_used_cnt;

        s_matrix_4x4 batch_view_mat;

        gfx::s_resource_handle batch_tex_hdl;

        s_static_stack<gfx::s_resource_handle, g_surf_stack_cap> surf_hdls;

#ifdef ZF_DEBUG
        struct {
            t_size batch_flush_cnt;
        } debug;
#endif
    };

    static s_str_rdonly g_batch_vert_shader_src = R"(#version 460 core

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

    static s_str_rdonly g_batch_frag_shader_src = R"(#version 460 core

in vec2 v_tex_coord;
in vec4 v_blend;

out vec4 o_frag_color;

uniform sampler2D u_tex;

void main() {
    vec4 tex_color = texture(u_tex, v_tex_coord);
    o_frag_color = tex_color * v_blend;
}
)";

    static s_str_rdonly g_surface_default_vert_shader_src = R"(#version 430 core

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

    const s_str_rdonly g_surface_default_frag_shader_src = R"(#version 430 core

in vec2 v_tex_coord;
out vec4 o_frag_color;

uniform sampler2D u_tex;

void main() {
    o_frag_color = texture(u_tex, v_tex_coord);
}
)";

    static gfx::s_resource_handle MakeBatchMesh(gfx::s_resource_arena& gfx_res_arena, s_mem_arena& temp_mem_arena) {
        s_array<t_u16> elems;

        if (!MakeArray(temp_mem_arena, g_batch_slot_elem_cnt * g_batch_slot_cnt, elems)) {
            ZF_REPORT_ERROR();
            return {};
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

        return gfx::MakeMesh(gfx_res_arena, nullptr, verts_len, elems, g_batch_vert_attr_lens);
    }

    static gfx::s_resource_handle MakeSurfaceMesh(gfx::s_resource_arena& gfx_res_arena) {
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

        return gfx::MakeMesh(gfx_res_arena, verts.buf_raw, verts.g_len, elems, vert_attr_lens);
    }

    t_b8 MakeRenderingBasis(gfx::s_resource_arena& gfx_res_arena, s_mem_arena& temp_mem_arena, s_rendering_basis& o_basis) {
        // Generate the batch mesh.
        o_basis.batch_mesh_hdl = MakeBatchMesh(gfx_res_arena, temp_mem_arena);

        if (!gfx::IsResourceHandleValid(o_basis.batch_mesh_hdl)) {
            ZF_REPORT_ERROR();
            return false;
        }

        // Generate batch shader program.
        o_basis.batch_shader_prog_hdl = gfx::MakeShaderProg(gfx_res_arena, g_batch_vert_shader_src, g_batch_frag_shader_src, temp_mem_arena);

        if (!gfx::IsResourceHandleValid(o_basis.batch_shader_prog_hdl)) {
            ZF_REPORT_ERROR();
            return false;
        }

        // Generate surface mesh.
        o_basis.surf_mesh_hdl = MakeSurfaceMesh(gfx_res_arena);

        if (!gfx::IsResourceHandleValid(o_basis.surf_mesh_hdl)) {
            ZF_REPORT_ERROR();
            return false;
        }

        // Generate surface shader program.
        o_basis.surf_default_shader_prog_hdl = gfx::MakeShaderProg(gfx_res_arena, g_surface_default_vert_shader_src, g_surface_default_frag_shader_src, temp_mem_arena);

        if (!gfx::IsResourceHandleValid(o_basis.surf_default_shader_prog_hdl)) {
            ZF_REPORT_ERROR();
            return false;
        }

        // Generate the pixel texture.
        const s_static_array<t_u8, 4> px_rgba = {
            {255, 255, 255, 255}
        };

        if (!gfx::LoadTextureAsset({{1, 1}, px_rgba}, gfx_res_arena, o_basis.px_tex)) {
            ZF_REPORT_ERROR();
            return false;
        }

        return true;
    }

    static void Flush(const s_rendering_context& rc) {
        if (rc.state->batch_slots_used_cnt == 0) {
            // Nothing to flush!
            return;
        }

        //
        // Submitting Vertex Data to GPU
        //
        glBindVertexArray(rc.basis->batch_mesh_hdl.raw.mesh.vert_arr_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, rc.basis->batch_mesh_hdl.raw.mesh.vert_buf_gl_id);

        {
            const t_size write_size = ZF_SIZE_OF(t_batch_slot) * rc.state->batch_slots_used_cnt;
            glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, rc.state->batch_slots.buf_raw);
        }

        //
        // Rendering the Batch
        //
        const gfx::t_gl_id prog_gl_id = rc.basis->batch_shader_prog_hdl.raw.shader_prog.gl_id;

        glUseProgram(prog_gl_id);

        const t_s32 view_uniform_loc = glGetUniformLocation(prog_gl_id, "u_view");
        glUniformMatrix4fv(view_uniform_loc, 1, false, &rc.state->batch_view_mat.elems[0][0]);

        const s_rect<t_s32> viewport = []() {
            s_rect<t_s32> vp;
            glGetIntegerv(GL_VIEWPORT, reinterpret_cast<t_s32*>(&vp));
            return vp;
        }();

        const auto proj_mat = MakeOrthographicMatrix4x4(0.0f, static_cast<t_f32>(viewport.width), static_cast<t_f32>(viewport.height), 0.0f, -1.0f, 1.0f);
        const t_s32 proj_uniform_loc = glGetUniformLocation(prog_gl_id, "u_proj");
        glUniformMatrix4fv(proj_uniform_loc, 1, false, &proj_mat.elems[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rc.state->batch_tex_hdl.raw.tex.gl_id);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rc.basis->batch_mesh_hdl.raw.mesh.elem_buf_gl_id);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_batch_slot_elem_cnt * rc.state->batch_slots_used_cnt), GL_UNSIGNED_SHORT, nullptr);

        glUseProgram(0);

        rc.state->batch_slots_used_cnt = 0;

#ifdef ZF_DEBUG
        rc.state->debug.batch_flush_cnt++;
#endif
    }

    s_rendering_state* PrepareRenderingPhase(s_mem_arena& mem_arena) {
        const auto rs = PushToMemArena<s_rendering_state>(mem_arena);

        if (rs) {
            rs->batch_view_mat = MakeIdentityMatrix4x4();
            DrawClear(g_default_bg_color);
        }

        return rs;
    }

    t_b8 CompleteRenderingPhase(const s_rendering_context& rc) {
        if (!IsStackEmpty(rc.state->surf_hdls)) {
            ZF_REPORT_ERROR();
            return false;
        }

        Flush(rc);

        return true;
    }

    void DrawClear(const s_color_rgba32f col) {
        glClearColor(col.r, col.g, col.b, col.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void UpdateViewMatrix(const s_rendering_context& rc, const s_matrix_4x4& mat) {
        Flush(rc);
        rc.state->batch_view_mat = mat;
    }

    static void Draw(const s_rendering_context& rc, const gfx::s_resource_handle tex_hdl, const s_rect<t_f32> tex_coords, s_v2<t_f32> pos, s_v2<t_f32> size, s_v2<t_f32> origin, const t_f32 rot, const s_color_rgba32f blend) {
        ZF_ASSERT(gfx::IsResourceHandleValid(tex_hdl));

        if (rc.state->batch_slots_used_cnt == 0) {
            // This is the first draw to the batch, so set the texture associated with the batch to the one we're trying to render.
            rc.state->batch_tex_hdl = tex_hdl;
        } else if (rc.state->batch_slots_used_cnt == g_batch_slot_cnt || !gfx::AreResourcesEqual(tex_hdl, rc.state->batch_tex_hdl)) {
            // Flush the batch and then try this same render operation again but on a fresh batch.
            Flush(rc);
            Draw(rc, tex_hdl, tex_coords, pos, size, origin, rot, blend);
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

    static s_rect<t_f32> CalcTextureCoords(const s_rect<t_s32> src_rect, const s_v2<t_s32> tex_size) {
        const s_v2<t_f32> half_texel = {
            0.5f / static_cast<t_f32>(tex_size.x),
            0.5f / static_cast<t_f32>(tex_size.y)
        };

        return {
            (static_cast<t_f32>(src_rect.x) + half_texel.x) / static_cast<t_f32>(tex_size.x),
            (static_cast<t_f32>(src_rect.y) + half_texel.y) / static_cast<t_f32>(tex_size.y),
            (static_cast<t_f32>(src_rect.width) - half_texel.x) / static_cast<t_f32>(tex_size.x),
            (static_cast<t_f32>(src_rect.height) - half_texel.y) / static_cast<t_f32>(tex_size.y)
        };
    }

    void DrawTexture(const s_rendering_context& rc, const gfx::s_texture_asset& tex, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect, const s_v2<t_f32> origin, const s_v2<t_f32> scale, const t_f32 rot, const s_color_rgba32f blend) {
        ZF_ASSERT(gfx::IsResourceHandleValid(tex.hdl));
        ZF_ASSERT(origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f); // @todo: Generic function for this check?
        // @todo: Add more assertions here!

        s_rect<t_s32> src_rect_to_use;

        if (src_rect == s_rect<t_s32>()) {
            // If the source rectangle wasn't set, just go with the whole texture.
            src_rect_to_use = {0, 0, tex.size_cache.x, tex.size_cache.y};
        } else {
            ZF_ASSERT(RectLeft(src_rect) >= 0 && RectTop(src_rect) >= 0 && RectRight(src_rect) <= tex.size_cache.x && RectTop(src_rect) <= tex.size_cache.y);
            src_rect_to_use = src_rect;
        }

        const s_rect tex_coords = CalcTextureCoords(src_rect_to_use, tex.size_cache);

        const s_v2<t_f32> size = {
            static_cast<t_f32>(src_rect_to_use.width) * scale.x, static_cast<t_f32>(src_rect_to_use.height) * scale.y
        };

        Draw(rc, tex.hdl, tex_coords, pos, size, origin, rot, blend);
    }

    t_b8 DrawStr(const s_rendering_context& rc, const s_str_rdonly str, const gfx::s_font_asset& font, const s_v2<t_f32> pos, const s_v2<t_f32> alignment, const s_color_rgba32f blend, s_mem_arena& temp_mem_arena) {
        if (IsStrEmpty(str)) {
            return true;
        }

        s_array<s_v2<t_f32>> chr_positions;

        if (!LoadStrChrDrawPositions(str, font.arrangement, pos, alignment, temp_mem_arena, chr_positions)) {
            return false;
        }

        t_size chr_index = 0;

        ZF_WALK_STR(str, chr_info) {
            if (chr_info.code_pt == ' ' || chr_info.code_pt == '\n') {
                chr_index++;
                continue;
            }

            s_font_glyph_info glyph_info;

            if (!HashMapGet(font.arrangement.code_pts_to_glyph_infos, chr_info.code_pt, &glyph_info)) {
                ZF_ASSERT(false);
                return false;
            }

            const auto chr_tex_coords = CalcTextureCoords(glyph_info.atlas_rect, g_font_atlas_size);

            Draw(rc, font.atlas_tex_hdls[glyph_info.atlas_index], chr_tex_coords, chr_positions[chr_index], static_cast<s_v2<t_f32>>(RectSize(glyph_info.atlas_rect)), {}, 0.0f, blend);

            chr_index++;
        };

        return true;
    }

    void SetSurface(const s_rendering_context& rc, const gfx::s_resource_handle& surf_hdl) {
        if (IsStackFull(rc.state->surf_hdls)) {
            ZF_REPORT_ERROR();
            return;
        }

        Flush(rc);

        glBindFramebuffer(GL_FRAMEBUFFER, surf_hdl.raw.surf.fb_gl_id);
        glViewport(0, 0, static_cast<GLsizei>(surf_hdl.raw.surf.size.x), static_cast<GLsizei>(surf_hdl.raw.surf.size.y));

        StackPush(rc.state->surf_hdls, surf_hdl);
    }

    void UnsetSurface(const s_rendering_context& rc) {
        if (IsStackEmpty(rc.state->surf_hdls)) {
            ZF_REPORT_ERROR();
            return;
        }

        Flush(rc);

        StackPop(rc.state->surf_hdls);

        gfx::t_gl_id fb_gl_id;
        s_v2<t_size> viewport_size;

        if (IsStackEmpty(rc.state->surf_hdls)) {
            fb_gl_id = 0;
            viewport_size = GetWindowSize();
        } else {
            const auto new_surf = StackTop(rc.state->surf_hdls);
            fb_gl_id = new_surf.raw.surf.fb_gl_id;
            viewport_size = new_surf.raw.surf.size;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id);
        glViewport(0, 0, static_cast<GLsizei>(viewport_size.x), static_cast<GLsizei>(viewport_size.y));
    }

    static inline gfx::t_gl_id CurGLShaderProg() {
        t_s32 prog;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        return static_cast<gfx::t_gl_id>(prog);
    }

    void SetSurfaceShaderProg(const s_rendering_context& rc, const gfx::s_resource_handle& prog) {
        ZF_ASSERT(CurGLShaderProg() == 0 && "Potential attempted double-assignment of surface shader program?");
        glUseProgram(prog.raw.shader_prog.gl_id);
    }

    t_b8 SetSurfaceShaderProgUniform(const s_rendering_context& rc, const s_str_rdonly name, const gfx::s_shader_prog_uniform_val& val, s_mem_arena& temp_mem_arena) {
        const gfx::t_gl_id cur_prog_gl_id = CurGLShaderProg();

        ZF_ASSERT(cur_prog_gl_id != 0 && "Surface shader program must be set before setting uniforms!");

        s_str name_terminated;

        if (!CloneStrButAddTerminator(name, temp_mem_arena, name_terminated)) {
            return false;
        }

        const t_s32 loc = glGetUniformLocation(cur_prog_gl_id, StrRaw(name_terminated));
        ZF_ASSERT(loc != -1 && "Failed to get location of shader uniform!"); // @todo: Possibly too strict? If a uniform is optimised out, it might be annoying to have to deal with these assert trips.

        switch (val.type) {
            case gfx::ek_shader_prog_uniform_val_type_s32:
                glUniform1i(loc, val.type_data.s32);
                break;

            case gfx::ek_shader_prog_uniform_val_type_u32:
                glUniform1ui(loc, val.type_data.u32);
                break;

            case gfx::ek_shader_prog_uniform_val_type_f32:
                glUniform1f(loc, val.type_data.f32);
                break;

            case gfx::ek_shader_prog_uniform_val_type_v2:
                glUniform2f(loc, val.type_data.v2.x, val.type_data.v2.y);
                break;

            case gfx::ek_shader_prog_uniform_val_type_v3:
                glUniform3f(loc, val.type_data.v3.x, val.type_data.v3.y, val.type_data.v3.z);
                break;

            case gfx::ek_shader_prog_uniform_val_type_v4:
                glUniform4f(loc, val.type_data.v4.x, val.type_data.v4.y, val.type_data.v4.z, val.type_data.v4.w);
                break;

            case gfx::ek_shader_prog_uniform_val_type_mat4x4:
                glUniformMatrix4fv(loc, 1, false, reinterpret_cast<const t_f32*>(&val.type_data.mat4x4));
                break;
        }

        return true;
    }

    static inline s_rect<t_s32> GLViewport() {
        s_rect<t_s32> v;
        glGetIntegerv(GL_VIEWPORT, reinterpret_cast<t_s32*>(&v));
        return v;
    }

    void DrawSurface(const s_rendering_context& rc, const gfx::s_resource_handle& surf_hdl, const s_v2<t_f32> pos, const s_v2<t_f32> scale) {
        ZF_ASSERT(surf_hdl.type == gfx::ek_resource_type_surface);

        ZF_ASSERT(CurGLShaderProg() != 0 && "Surface shader program must be set before rendering a surface!");

        const auto& surf_gl_mesh = rc.basis->surf_mesh_hdl.raw.mesh;

        glBindVertexArray(surf_gl_mesh.vert_arr_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, surf_gl_mesh.vert_buf_gl_id);

        {
            const s_static_array<t_f32, 16> verts = {{
                0.0f,    scale.y, 0.0f, 0.0f,
                scale.x, scale.y, 1.0f, 0.0f,
                scale.x, 0.0f,    1.0f, 1.0f,
                0.0f,    0.0f,    0.0f, 1.0f
            }};

            glBufferSubData(GL_ARRAY_BUFFER, 0, ArraySizeInBytes(verts), verts.buf_raw);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, surf_hdl.raw.surf.tex_gl_id);

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

        const auto surf_size = static_cast<s_v2<t_f32>>(surf_hdl.raw.surf.size);
        glUniform2fv(size_uniform_loc, 1, reinterpret_cast<const t_f32*>(&surf_size));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surf_gl_mesh.elem_buf_gl_id);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

        glUseProgram(0);
    }
#endif
}
