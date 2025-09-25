#include "zf_rendering.h"

namespace zf {
    const c_string_view g_surface_default_vert_shader_src = "#version 430 core\n"
        "\n"
        "layout (location = 0) in vec2 a_vert;\n"
        "layout (location = 1) in vec2 a_tex_coord;\n"
        "\n"
        "out vec2 v_tex_coord;\n"
        "\n"
        "uniform vec2 u_pos;\n"
        "uniform vec2 u_size;\n"
        "uniform mat4 u_proj;\n"
        "\n"
        "void main() {\n"
        "    mat4 model = mat4(\n"
        "        vec4(u_size.x, 0.0, 0.0, 0.0),\n"
        "        vec4(0.0, u_size.y, 0.0, 0.0),\n"
        "        vec4(0.0, 0.0, 1.0, 0.0),\n"
        "        vec4(u_pos.x, u_pos.y, 0.0, 1.0)\n"
        "    );\n"
        "\n"
        "    gl_Position = u_proj * model * vec4(a_vert, 0.0, 1.0);\n"
        "    v_tex_coord = a_tex_coord;\n"
        "}\n";

    const c_string_view g_surface_default_frag_shader_src = "#version 430 core\n"
        "\n"
        "in vec2 v_tex_coord;\n"
        "out vec4 o_frag_color;\n"
        "\n"
        "uniform sampler2D u_tex;\n"
        "\n"
        "void main() {\n"
            "o_frag_color = texture(u_tex, v_tex_coord);\n"
        "}\n";

    const c_string_view g_surface_blend_vert_shader_src = "#version 430 core\n" \
        "\n" \
        "layout (location = 0) in vec2 a_vert;\n" \
        "layout (location = 1) in vec2 a_tex_coord;\n" \
        "\n" \
        "out vec2 v_tex_coord;\n" \
        "\n" \
        "uniform vec2 u_pos;\n" \
        "uniform vec2 u_size;\n" \
        "uniform mat4 u_proj;\n" \
        "\n" \
        "void main() {\n" \
            "mat4 model = mat4(\n" \
                "vec4(u_size.x, 0.0, 0.0, 0.0),\n" \
                "vec4(0.0, u_size.y, 0.0, 0.0),\n" \
                "vec4(0.0, 0.0, 1.0, 0.0),\n" \
                "vec4(u_pos.x, u_pos.y, 0.0, 1.0)\n" \
            ");\n" \
        "\n" \
            "gl_Position = u_proj * model * vec4(a_vert, 0.0, 1.0);\n" \
            "v_tex_coord = a_tex_coord;\n" \
        "}\n";

    const c_string_view g_surface_blend_frag_shader_src = "#version 430 core\n" \
        "\n" \
        "in vec2 v_tex_coord;\n" \
        "out vec4 o_frag_color;\n" \
        "\n" \
        "uniform sampler2D u_tex;\n" \
        "uniform vec3 u_col;\n" \
        "uniform float u_intensity;\n" \
        "\n" \
        "void main() {\n" \
            "vec4 tex_col = texture(u_tex, v_tex_coord);\n" \
        "\n" \
            "if (tex_col.a > 0.0) {\n" \
                "o_frag_color = vec4(\n" \
                    "mix(tex_col.r, u_col.r, u_intensity),\n" \
                    "mix(tex_col.g, u_col.g, u_intensity),\n" \
                    "mix(tex_col.b, u_col.b, u_intensity),\n" \
                    "tex_col.a\n" \
                ");\n" \
            "} else {\n" \
                "o_frag_color = tex_col;\n" \
            "}\n" \
        "}\n";

    static bool AttachFramebufferTexture(const t_gl_id fb_gl_id, const t_gl_id tex_gl_id, const s_v2_s32 tex_size) {
        glBindTexture(GL_TEXTURE_2D, tex_gl_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size.x, tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_gl_id, 0);

        const bool success = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return success;
    }

    bool InitSurface(s_surface& surf, const s_v2_s32 size, s_gl_resource_arena& gl_res_arena) {
        auto fb_gl_id = PushToGLResourceArena(gl_res_arena, ek_gl_resource_type_framebuffer);

        if (!fb_gl_id) {
            //LOG_ERROR("Failed to reserve OpenGL framebuffer ID for surface!");
            return false;
        }

        auto fb_tex_gl_id = PushToGLResourceArena(gl_res_arena, ek_gl_resource_type_texture);

        if (!fb_tex_gl_id) {
            //LOG_ERROR("Failed to reserve OpenGL texture ID for surface framebuffer!");
            return false;
        }

        glGenFramebuffers(1, &fb_gl_id);
        glGenTextures(1, &fb_tex_gl_id);

        if (!AttachFramebufferTexture(fb_gl_id, fb_tex_gl_id, size)) {
            //LOG_ERROR("Failed to attach framebuffer texture for surface!");
            return false;
        }

        surf = {
            .fb_gl_id = fb_gl_id,
            .fb_tex_gl_id = fb_tex_gl_id,
            .size = size
        };

        return true;
    }

    bool ResizeSurface(s_surface& surf, const s_v2_s32 size) {
        assert(surf.size.x != size.x || surf.size.y != size.y);

        // Delete old texture.
        glDeleteTextures(1, &surf.fb_tex_gl_id);

        // Generate and attach new texture of the new size.
        glGenTextures(1, &surf.fb_tex_gl_id);

        if (!AttachFramebufferTexture(surf.fb_gl_id, surf.fb_tex_gl_id, size)) {
            //LOG_ERROR("Failed to attach framebuffer texture for surface during resize!");
            return false;
        }

        surf.size = size;

        return true;
    }

    static void PushToSurfaceFramebufferGLIDStack(s_surface_framebuffer_gl_id_stack& stack, const t_gl_id gl_id) {
        assert(glIsFramebuffer(gl_id));
        assert(stack.height < stack.buf.Len());

#ifndef NDEBUG
        for (int i = 0; i < stack.height; i++) {
            assert(stack.buf[i] != gl_id && "Trying to push a surface framebuffer OpenGL ID that is already on the stack!");
        }
#endif

        stack.buf[stack.height] = gl_id;
        stack.height++;
    }

    static t_gl_id PopFromSurfaceFramebufferGLIDStack(s_surface_framebuffer_gl_id_stack& stack) {
        assert(stack.height > 0 && "Trying to pop from an empty stack!");

        stack.height--;
        return stack.buf[stack.height];
    }

    void SetSurface(const s_rendering_context& rendering_context, const s_surface& surf) {
        assert(surf.fb_gl_id != BoundGLFramebuffer() && "Trying to set a surface that is already set!");

        SubmitBatch(rendering_context);

        const t_gl_id bound_fb_gl_id = BoundGLFramebuffer();

        if (bound_fb_gl_id != 0) {
            PushToSurfaceFramebufferGLIDStack(rendering_context.state.surf_fb_gl_id_stack, bound_fb_gl_id);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, surf.fb_gl_id);
        glViewport(0, 0, surf.size.x, surf.size.y);
    }

    void UnsetSurface(const s_rendering_context& rendering_context) {
        assert(BoundGLFramebuffer() != 0 && "Trying to unset surface but no OpenGL framebuffer is bound!");

        SubmitBatch(rendering_context);

        if (rendering_context.state.surf_fb_gl_id_stack.height > 0) {
            const t_gl_id fb_gl_id = PopFromSurfaceFramebufferGLIDStack(rendering_context.state.surf_fb_gl_id_stack);
            glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id);

            // TODO: The below is pretty bad. Really should be accessing the surface data directly.
            t_gl_id fb_tex_gl_id;
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, reinterpret_cast<t_s32*>(&fb_tex_gl_id));
            assert(glIsTexture(fb_tex_gl_id));
        
            s_v2_s32 fb_tex_size;
            glBindTexture(GL_TEXTURE_2D, fb_tex_gl_id);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &fb_tex_size.x);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &fb_tex_size.y);

            glViewport(0, 0, fb_tex_size.x, fb_tex_size.y);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, rendering_context.window_size.x, rendering_context.window_size.y);
        }
    }

    static inline t_gl_id CurrentGLShaderProgram() {
        t_s32 prog;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        return prog;
    }

    void SetSurfaceShaderProg(const s_rendering_context& rendering_context, const s_shader_prog_group& progs, const t_s32 prog_index) {
        assert(CurrentGLShaderProgram() == 0 && "Potential attempted double-assignment of surface shader program?");
        glUseProgram(progs.gl_ids[prog_index]);
    }

    void SetSurfaceShaderProgUniform(const s_rendering_context& rendering_context, const char* const name, const s_shader_prog_uniform_value val) {
        assert(name);

        const t_gl_id prog_gl_id = CurrentGLShaderProgram();

        assert(prog_gl_id != 0 && "Surface shader program must be set before setting uniforms!");

        const t_s32 loc = glGetUniformLocation(prog_gl_id, name);
        assert(loc != -1 && "Failed to get location of shader uniform!");

        switch (val.type) {
            case ek_shader_prog_uniform_value_type_s32:
                glUniform1i(loc, val.as_s32);
                break;

            case ek_shader_prog_uniform_value_type_r32:
                glUniform1f(loc, val.as_r32);
                break;

            case ek_shader_prog_uniform_value_type_v2:
                glUniform2f(loc, val.as_v2.x, val.as_v2.y);
                break;

            case ek_shader_prog_uniform_value_type_v3:
                glUniform3f(loc, val.as_v3.x, val.as_v3.y, val.as_v3.z);
                break;

            case ek_shader_prog_uniform_value_type_v4:
                glUniform4f(loc, val.as_v4.x, val.as_v4.y, val.as_v4.z, val.as_v4.w);
                break;

            case ek_shader_prog_uniform_value_type_mat4x4:
                glUniformMatrix4fv(loc, 1, false, &val.as_mat4x4.elems[0][0]);
                break;
        }
    }

    void RenderSurface(const s_rendering_context& rendering_context, const s_surface& surf, const s_v2 pos, const s_v2 scale, const bool blend) {
        assert(surf.fb_gl_id != BoundGLFramebuffer() && "Trying to render the currently set surface! Unset the surface first.");

#ifndef NDEBUG
        for (int i = 0; i < rendering_context.state.surf_fb_gl_id_stack.height; i++) {
            assert(rendering_context.state.surf_fb_gl_id_stack.buf[i] != surf.fb_gl_id && "Trying to render a surface currently in the stack! Unset the surface first.");
        }
#endif

        assert(CurrentGLShaderProgram() != 0 && "Surface shader program must be set before rendering a surface!");

        if (!blend) {
            glDisable(GL_BLEND);
        }

        const s_renderable& renderable = rendering_context.basis.renderables[ek_renderable_surface];

        glBindVertexArray(renderable.vert_array_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, renderable.vert_buf_gl_id);

        {
            const float verts[] = {
                0.0f, scale.y, 0.0f, 0.0f,
                scale.x, scale.y, 1.0f, 0.0f,
                scale.x, 0.0f, 1.0f, 1.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };

            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, surf.fb_tex_gl_id);

        const t_s32 proj_uniform_loc = glGetUniformLocation(CurrentGLShaderProgram(), "u_proj");
        assert(proj_uniform_loc != -1);
        const s_rect_s32 viewport = GLViewport();
        const s_matrix_4x4 proj_mat = s_matrix_4x4::Orthographic(0.0f, viewport.width, viewport.height, 0.0f, -1.0f, 1.0f);
        glUniformMatrix4fv(proj_uniform_loc, 1, false, reinterpret_cast<const float*>(proj_mat.elems));

        const t_s32 pos_uniform_loc = glGetUniformLocation(CurrentGLShaderProgram(), "u_pos");
        assert(pos_uniform_loc != -1);
        glUniform2fv(pos_uniform_loc, 1, reinterpret_cast<const float*>(&pos));

        const t_s32 size_uniform_loc = glGetUniformLocation(CurrentGLShaderProgram(), "u_size");
        assert(size_uniform_loc != -1);
        const s_v2 surf_size_float = {static_cast<float>(surf.size.x), static_cast<float>(surf.size.y)};
        glUniform2fv(size_uniform_loc, 1, reinterpret_cast<const float*>(&surf_size_float));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderable.elem_buf_gl_id);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

        glUseProgram(0);

        if (!blend) {
            glEnable(GL_BLEND);
        }
    }
}
