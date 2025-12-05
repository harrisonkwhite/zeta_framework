#pragma once

#include <zgl/public/zgl_gfx.h>

namespace zf {
    // ============================================================
    // @section: OpenGL Helpers
    // ============================================================
    using t_gl_id = GLuint;

    struct s_mesh_gl_ids {
        t_gl_id vert_arr;
        t_gl_id vert_buf;
        t_gl_id elem_buf;
    };

    static s_mesh_gl_ids MakeGLMesh(const t_f32* const verts_raw, const t_size verts_len, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_s32> vert_attr_lens) {
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

    static void ReleaseGLMesh(const s_mesh_gl_ids& gl_ids) {
        glDeleteBuffers(1, &gl_ids.elem_buf);
        glDeleteBuffers(1, &gl_ids.vert_buf);
        glDeleteVertexArrays(1, &gl_ids.vert_arr);
    }

    static t_gl_id MakeGLShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_mem_arena& temp_mem_arena) {
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

    static s_v2<t_s32> GLTextureSizeLimit() {
        t_s32 size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
        return { size, size };
    }

    static t_gl_id MakeGLTexture(const s_rgba_texture_data_rdonly& tex_data) {
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

    static t_gl_id CurGLShaderProg() {
        t_s32 prog;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        return static_cast<t_gl_id>(prog);
    }

    static t_b8 AttachGLFramebufferTexture(const t_gl_id fb_gl_id, const t_gl_id tex_gl_id, const s_v2<t_s32> tex_size) {
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
    enum e_gfx_resource_type {
        ek_gfx_resource_type_invalid,
        ek_gfx_resource_type_texture,
        ek_gfx_resource_type_font,
        ek_gfx_resource_type_surface,
        ek_gfx_resource_type_surface_shader_prog
    };

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

    static s_gfx_resource* PushGFXResource(s_gfx_resource_arena& res_arena) {
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

    static t_b8 MakeFontAtlasGLTextures(const s_array_rdonly<t_font_atlas_rgba> atlas_rgbas, s_array<t_gl_id>& o_gl_ids) {
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

    t_b8 InitGFX(s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena, s_rendering_basis*& o_rendering_basis) {
        t_b8 clean_up = false;

        o_rendering_basis = PushToMemArena<s_rendering_basis>(mem_arena);

        if (!o_rendering_basis) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        // Enable blending.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
            o_rendering_basis->batch_mesh_gl_ids = MakeGLMesh(nullptr, verts_len, elems, g_batch_vert_attr_lens);
        }

        ZF_DEFER({
            if (clean_up) {
                ReleaseGLMesh(o_rendering_basis->batch_mesh_gl_ids);
            }
        });

        // Create the batch shader program.
        o_rendering_basis->batch_shader_prog_gl_id = MakeGLShaderProg(g_batch_vert_shader_src, g_batch_frag_shader_src, temp_mem_arena);

        if (!o_rendering_basis->batch_shader_prog_gl_id) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        ZF_DEFER({
            if (clean_up) {
                glDeleteProgram(o_rendering_basis->batch_shader_prog_gl_id);
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

            o_rendering_basis->surf_mesh_gl_ids = MakeGLMesh(verts.buf_raw, verts.g_len, elems, vert_attr_lens);
        }

        ZF_DEFER({
            if (clean_up) {
                ReleaseGLMesh(o_rendering_basis->surf_mesh_gl_ids);
            }
        });

        // Set up resource arena.
        o_rendering_basis->res_arena = MakeGFXResourceArena(mem_arena);

        ZF_DEFER({
            if (clean_up) {
                ReleaseGFXResources(o_rendering_basis->res_arena);
            }
        });

        // Set up pixel texture.
        {
            const s_static_array<t_u8, 4> rgba = {{255, 255, 255, 255}};

            if (!LoadTexture({{1, 1}, rgba}, o_rendering_basis->res_arena, o_rendering_basis->px_tex)) {
                ZF_REPORT_ERROR();
                clean_up = true;
                return false;
            }
        }

        // Set up default surface shader program.
        if (!MakeSurfaceShaderProg(g_default_surface_vert_shader_src, g_default_surface_frag_shader_src, o_rendering_basis->res_arena, temp_mem_arena, o_rendering_basis->default_surf_shader_prog)) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        return true;
    }

    void ShutdownGFX(const s_rendering_basis& rendering_basis) {
        ReleaseGFXResources(rendering_basis.res_arena);
        ReleaseGLMesh(rendering_basis.surf_mesh_gl_ids);
        glDeleteProgram(rendering_basis.batch_shader_prog_gl_id);
        ReleaseGLMesh(rendering_basis.batch_mesh_gl_ids);
    }

    t_b8 BeginFrame(const s_rendering_basis& rendering_basis, const s_v2<t_s32> framebuffer_size_cache, s_mem_arena& mem_arena, s_rendering_context& o_rendering_context) {
        o_rendering_context = {
            .framebuffer_size_cache = framebuffer_size_cache,
            .basis = &rendering_basis,
            .state = PushToMemArena<s_rendering_state>(mem_arena)
        };

        if (!o_rendering_context.state) {
            return false;
        }

        o_rendering_context.state->view_mat = MakeIdentityMatrix4x4();
        glViewport(0, 0, framebuffer_size_cache.x, framebuffer_size_cache.y);
        Clear(o_rendering_context, s_color_rgba8(147, 207, 249, 255));

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

        const auto proj_mat = MakeOrthographicMatrix4x4(0.0f, static_cast<t_f32>(rc.framebuffer_size_cache.x), static_cast<t_f32>(rc.framebuffer_size_cache.y), 0.0f, -1.0f, 1.0f);
        const t_s32 proj_uniform_loc = glGetUniformLocation(rc.basis->batch_shader_prog_gl_id, "u_proj");
        glUniformMatrix4fv(proj_uniform_loc, 1, false, reinterpret_cast<const t_f32*>(&proj_mat));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rc.state->tex_gl_id);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rc.basis->batch_mesh_gl_ids.elem_buf);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_batch_slot_elem_cnt * rc.state->batch_slots_used_cnt), GL_UNSIGNED_SHORT, nullptr);

        glUseProgram(0);

        rc.state->batch_slots_used_cnt = 0;
    }

    void CompleteFrame(const s_rendering_context& rc) {
        Flush(rc);
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
            viewport_size = rc.framebuffer_size_cache;
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

        const s_matrix_4x4 proj_mat = MakeOrthographicMatrix4x4(0.0f, static_cast<t_f32>(rc.framebuffer_size_cache.x), static_cast<t_f32>(rc.framebuffer_size_cache.y), 0.0f, -1.0f, 1.0f);
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
}
