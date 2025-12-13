#include <zgl/zgl_gfx.h>

#include <glad/glad.h>
#include <zgl/zgl_platform.h>

namespace zf::gfx {
    t_b8 Init() {
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(platform::internal::GetGLProcAddrFunc()))) {
            return false;
        }

        return true;
    }

    void Shutdown() {
    }

#if 0
    // ============================================================
    // @section: OpenGL Helpers
    // ============================================================
    using t_gl_id = GLuint;

    struct s_mesh_gl_ids {
        t_gl_id vert_arr = 0;
        t_gl_id vert_buf = 0;
        t_gl_id elem_buf = 0;
    };

    static s_mesh_gl_ids CreateGLMesh(const s_ptr<const t_f32> verts_ptr, const t_len verts_len, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_i32> vert_attr_lens) {
        s_mesh_gl_ids gl_ids = {};

        glGenVertexArrays(1, &gl_ids.vert_arr);
        glBindVertexArray(gl_ids.vert_arr);

        glGenBuffers(1, &gl_ids.vert_buf);
        glBindBuffer(GL_ARRAY_BUFFER, gl_ids.vert_buf);
        glBufferData(GL_ARRAY_BUFFER, ZF_SIZE_OF(t_f32) * verts_len, verts_ptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &gl_ids.elem_buf);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_ids.elem_buf);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(elems.SizeInBytes()), elems.Ptr(), GL_STATIC_DRAW);

        const t_len stride = [vert_attr_lens]() {
            t_len res = 0;

            for (t_len i = 0; i < vert_attr_lens.Len(); i++) {
                res += ZF_SIZE_OF(t_f32) * static_cast<t_len>(vert_attr_lens[i]);
            }

            return res;
        }();

        t_i32 offs = 0;

        for (t_len i = 0; i < vert_attr_lens.Len(); i++) {
            const t_i32 attr_len = vert_attr_lens[i];

            glVertexAttribPointer(static_cast<GLuint>(i), attr_len, GL_FLOAT, false, static_cast<GLsizei>(stride), reinterpret_cast<void *>(ZF_SIZE_OF(t_f32) * offs));
            glEnableVertexAttribArray(static_cast<GLuint>(i));

            offs += attr_len;
        }

        glBindVertexArray(0);

        return gl_ids;
    }

    static void DestroyGLMesh(s_mesh_gl_ids &gl_ids) {
        glDeleteBuffers(1, &gl_ids.elem_buf);
        glDeleteBuffers(1, &gl_ids.vert_buf);
        glDeleteVertexArrays(1, &gl_ids.vert_arr);
        gl_ids = {};
    }

    static t_gl_id CreateGLShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_mem_arena &temp_mem_arena) {
        s_str vert_src_terminated = {};
        s_str frag_src_terminated = {};

        if (!AllocStrCloneWithTerminator(vert_src, temp_mem_arena, vert_src_terminated)
            || !AllocStrCloneWithTerminator(frag_src, temp_mem_arena, frag_src_terminated)) {
            return 0;
        }

        //
        // Shader Creation
        //
        const auto create_shader = [&temp_mem_arena](const s_str_rdonly src, const t_b8 is_frag) -> t_gl_id {
            const t_gl_id shader_gl_id = glCreateShader(is_frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);

            const auto src_raw = src.Cstr();
            glShaderSource(shader_gl_id, 1, &src_raw, nullptr);

            glCompileShader(shader_gl_id);

            t_i32 success = 0;
            glGetShaderiv(shader_gl_id, GL_COMPILE_STATUS, &success);

            if (!success) {
                ZF_DEFER({ glDeleteShader(shader_gl_id); });

                // Try getting the OpenGL compile error message.
                t_i32 log_chr_cnt = 0;
                glGetShaderiv(shader_gl_id, GL_INFO_LOG_LENGTH, &log_chr_cnt);

                if (log_chr_cnt > 1) {
                    s_array<char> log_chrs = {};

                    if (AllocArray(log_chr_cnt, temp_mem_arena, log_chrs)) {
                        glGetShaderInfoLog(shader_gl_id, static_cast<GLsizei>(log_chrs.Len()), nullptr, log_chrs.Ptr());
                        LogErrorType(s_cstr_literal("OpenGL Shader Compilation"), s_cstr_literal("%"), FormatStr({log_chrs.ToBytes()}));
                    } else {
                        ZF_REPORT_ERROR();
                    }
                } else {
                    LogError(s_cstr_literal("OpenGL shader compilation failed, but no error message available!"));
                }

                return 0;
            }

            return shader_gl_id;
        };

        const t_gl_id vert_gl_id = create_shader(vert_src_terminated, false);

        if (!vert_gl_id) {
            return 0;
        }

        ZF_DEFER({ glDeleteShader(vert_gl_id); });

        const t_gl_id frag_gl_id = create_shader(frag_src_terminated, true);

        if (!frag_gl_id) {
            return 0;
        }

        ZF_DEFER({ glDeleteShader(frag_gl_id); });

        //
        // Program Creation
        //
        const t_gl_id prog_gl_id = glCreateProgram();

        if (!prog_gl_id) {
            return 0;
        }

        glAttachShader(prog_gl_id, vert_gl_id);
        glAttachShader(prog_gl_id, frag_gl_id);

        glLinkProgram(prog_gl_id);

        // Check link result.
        t_i32 link_status = 0;
        glGetProgramiv(prog_gl_id, GL_LINK_STATUS, &link_status);

        if (!link_status) {
            // Linking failed, try getting an error log.
            t_i32 log_chr_cnt = 0;
            glGetProgramiv(prog_gl_id, GL_INFO_LOG_LENGTH, &log_chr_cnt);

            if (log_chr_cnt > 1) {
                s_array<char> log_chrs = {};

                if (AllocArray(log_chr_cnt, temp_mem_arena, log_chrs)) {
                    glGetProgramInfoLog(prog_gl_id, static_cast<GLsizei>(log_chrs.Len()), nullptr, log_chrs.Ptr());
                    LogErrorType(s_cstr_literal("OpenGL Program Link"), s_cstr_literal("%"), FormatStr({log_chrs.ToBytes()}));
                } else {
                    ZF_REPORT_ERROR();
                }
            } else {
                LogError(s_cstr_literal("OpenGL program link failed, but no error message available!"));
            }

            glDeleteProgram(prog_gl_id);

            return 0;
        }

        return prog_gl_id;
    }

    static s_v2_i GLTextureSizeLimit() {
        t_i32 size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
        return {size, size};
    }

    static t_gl_id CreateGLTexture(const s_texture_data_rdonly tex_data) {
        const s_v2_i tex_size_limit = GLTextureSizeLimit();

        if (tex_data.SizeInPixels().x > tex_size_limit.x || tex_data.SizeInPixels().y > tex_size_limit.y) {
            LogError(s_cstr_literal("Texture size % exceeds limits %!"), tex_data.SizeInPixels(), tex_size_limit);
            ZF_REPORT_ERROR();
            return 0;
        }

        t_gl_id gl_id;
        glGenTextures(1, &gl_id);

        glBindTexture(GL_TEXTURE_2D, gl_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_data.SizeInPixels().x, tex_data.SizeInPixels().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.RGBAPixelData().Ptr());

        return gl_id;
    }

    // ============================================================
    // @section: Resources
    // ============================================================
    enum e_gfx_resource_type : t_i32 {
        ek_gfx_resource_type_invalid,
        ek_gfx_resource_type_texture,
        ek_gfx_resource_type_font
    };

    struct s_gfx_resource {
    public:
        s_ptr<s_gfx_resource> next = nullptr;
        e_gfx_resource_type type = ek_gfx_resource_type_invalid;

        constexpr s_gfx_resource() = default;
        constexpr s_gfx_resource(const s_gfx_resource &) = delete;

        auto &Texture() {
            ZF_ASSERT(type == ek_gfx_resource_type_texture);
            return type_data.tex;
        }

        auto &Texture() const {
            ZF_ASSERT(type == ek_gfx_resource_type_texture);
            return type_data.tex;
        }

        auto &Font() {
            ZF_ASSERT(type == ek_gfx_resource_type_font);
            return type_data.font;
        }

        auto &Font() const {
            ZF_ASSERT(type == ek_gfx_resource_type_font);
            return type_data.font;
        }

    private:
        union {
            struct {
                t_gl_id gl_id;
                s_v2_i size;
            } tex;

            struct {
                s_font_arrangement arrangement;
                s_array<t_gl_id> atlas_gl_ids;
            } font;
        } type_data = {};
    };

    void DestroyGFXResources(s_gfx_resource_arena &res_arena) {
        s_ptr<s_gfx_resource> res = res_arena.head;

        while (res) {
            switch (res->type) {
            case ek_gfx_resource_type_texture:
                glDeleteTextures(1, &res->Texture().gl_id);
                break;

            case ek_gfx_resource_type_font:
                glDeleteTextures(static_cast<GLsizei>(res->Font().atlas_gl_ids.Len()), res->Font().atlas_gl_ids.Ptr());
                break;

            default:
                ZF_ASSERT(false);
                break;
            }

            *res = {};

            res = res->next;
        }
    }

    static s_ptr<s_gfx_resource> PushGFXResource(s_gfx_resource_arena &res_arena) {
        const auto res = Alloc<s_gfx_resource>(*res_arena.mem_arena);

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

    t_b8 CreateTexture(const s_texture_data_rdonly tex_data, s_gfx_resource_arena &res_arena, s_ptr<s_gfx_resource> &o_tex) {
        o_tex = {};

        const t_gl_id gl_id = CreateGLTexture(tex_data);

        if (!gl_id) {
            return false;
        }

        const auto tex = PushGFXResource(res_arena);

        if (!tex) {
            glDeleteTextures(1, &gl_id);
            return false;
        }

        tex->type = ek_gfx_resource_type_texture;
        tex->Texture().gl_id = gl_id;
        tex->Texture().size = tex_data.SizeInPixels();

        return tex;
    }

    s_v2_i TextureSize(const s_gfx_resource &res) {
        return res.Texture().size;
    }

    [[nodiscard]] static t_b8 CreateFontAtlasGLTextures(const s_array_rdonly<t_font_atlas_rgba> atlas_rgbas, s_mem_arena &gl_ids_mem_arena, s_array<t_gl_id> &o_gl_ids) {
        if (!AllocArray(atlas_rgbas.Len(), gl_ids_mem_arena, o_gl_ids)) {
            return false;
        }

        for (t_len i = 0; i < atlas_rgbas.Len(); i++) {
            o_gl_ids[i] = CreateGLTexture({g_font_atlas_size, atlas_rgbas[i]});

            if (!o_gl_ids[i]) {
                if (i > 0) {
                    glDeleteTextures(static_cast<GLsizei>(i), o_gl_ids.Ptr());
                }

                return false;
            }
        }

        return true;
    }

    [[nodiscard]] static t_b8 CreateFont(const s_font_arrangement &arrangement, const s_array<t_font_atlas_rgba> atlas_rgbas, s_gfx_resource_arena &res_arena, s_ptr<s_gfx_resource> &o_font) {
        s_array<t_gl_id> atlas_gl_ids = {};

        if (!CreateFontAtlasGLTextures(atlas_rgbas, *res_arena.mem_arena, atlas_gl_ids)) {
            return false;
        }

        o_font = PushGFXResource(res_arena);

        if (!o_font) {
            glDeleteTextures(static_cast<GLsizei>(atlas_gl_ids.Len()), atlas_gl_ids.Ptr());
            return false;
        }

        o_font->type = ek_gfx_resource_type_font;
        o_font->Font().arrangement = arrangement;
        o_font->Font().atlas_gl_ids = atlas_gl_ids;

        return true;
    }

    t_b8 CreateFontFromRaw(const s_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec &code_pts, s_gfx_resource_arena &res_arena, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_font) {
        s_font_arrangement arrangement = {};
        s_array<t_font_atlas_rgba> atlas_rgbas = {};

        const auto res = zf::LoadFontFromRaw(file_path, height, code_pts, *res_arena.mem_arena, temp_mem_arena, temp_mem_arena, arrangement, atlas_rgbas);

        if (res != ek_font_load_from_raw_result_success) {
            return false;
        }

        return CreateFont(arrangement, atlas_rgbas, res_arena, o_font);
    }

    t_b8 CreateFontFromPacked(const s_str_rdonly file_path, s_gfx_resource_arena &res_arena, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_font) {
        s_font_arrangement arrangement = {};
        s_array<t_font_atlas_rgba> atlas_rgbas = {};

        if (!zf::UnpackFont(file_path, *res_arena.mem_arena, temp_mem_arena, temp_mem_arena, arrangement, atlas_rgbas)) {
            return false;
        }

        return CreateFont(arrangement, atlas_rgbas, res_arena, o_font);
    }

    // ============================================================
    // @section: Rendering
    // ============================================================
    struct s_batch_vert {
        s_v2 vert_coord = {};
        s_v2 pos = {};
        s_v2 size = {};
        t_f32 rot = 0.0f;
        s_v2 tex_coord = {};
        s_color_rgba32f blend = {};
    };

    constexpr s_static_array<t_i32, 6> g_batch_vert_attr_lens = {2, 2, 2, 1, 2, 4}; // This has to match the number of components per attribute above.

    constexpr t_len g_batch_vert_component_cnt = ZF_SIZE_OF(s_batch_vert) / ZF_SIZE_OF(t_f32);

    static_assert(
        []() {
            t_len sum = 0;

            for (t_len i = 0; i < g_batch_vert_attr_lens.g_len; i++) {
                sum += g_batch_vert_attr_lens[i];
            }

            return sum == g_batch_vert_component_cnt;
        }(),
        "Mismatch between specified batch vertex attribute lengths and component count!");

    constexpr t_len g_batch_slot_cnt = 1 << 8;
    static_assert(g_batch_slot_cnt <= 1 << 16, "Batch slot count is too large (need to account for range limits of elements).");

    constexpr t_len g_batch_slot_vert_cnt = 4;
    constexpr t_len g_batch_slot_elem_cnt = 6;

    using t_batch_slot = s_static_array<s_batch_vert, g_batch_slot_vert_cnt>;

    constexpr s_cstr_literal g_batch_vert_shader_src = R"(#version 460 core

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

    constexpr s_cstr_literal g_batch_frag_shader_src = R"(#version 460 core

in vec2 v_tex_coord;
in vec4 v_blend;

out vec4 o_frag_color;

uniform sampler2D u_tex;

void main() {
    vec4 tex_color = texture(u_tex, v_tex_coord);
    o_frag_color = tex_color * v_blend;
}
)";

    struct s_rendering_basis {
        s_mesh_gl_ids batch_mesh_gl_ids = {};
        t_gl_id batch_shader_prog_gl_id = 0;

        s_gfx_resource_arena res_arena = {};
        s_ptr<s_gfx_resource> px_tex = nullptr;
    };

    struct s_rendering_state {
        s_static_array<t_batch_slot, g_batch_slot_cnt> batch_slots = {};
        t_len batch_slots_used_cnt = 0;

        s_mat4x4 view_mat = {}; // The view matrix to be used when flushing.
        t_gl_id tex_gl_id = {}; // The texture to be used when flushing.
    };

    t_b8 internal::BeginFrame(const s_rendering_basis &rendering_basis, const s_v2_i framebuffer_size_cache, s_mem_arena &mem_arena, s_rendering_context &o_rendering_context) {
        o_rendering_context = {
            .basis = &rendering_basis,
            .state = Alloc<s_rendering_state>(mem_arena),
            .framebuffer_size_cache = framebuffer_size_cache,
        };

        if (!o_rendering_context.state) {
            return false;
        }

        o_rendering_context.state->view_mat = CreateIdentityMatrix();
        glViewport(0, 0, framebuffer_size_cache.x, framebuffer_size_cache.y);
        Clear(o_rendering_context, s_color_rgba8(147, 207, 249, 255));

        return true;
    }

    static void Flush(const s_rendering_context rc) {
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
            const t_len write_size = ZF_SIZE_OF(t_batch_slot) * rc.state->batch_slots_used_cnt;
            glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, rc.state->batch_slots.raw);
        }

        //
        // Rendering the Batch
        //
        glUseProgram(rc.basis->batch_shader_prog_gl_id);

        const t_i32 view_uniform_loc = glGetUniformLocation(rc.basis->batch_shader_prog_gl_id, "u_view");
        glUniformMatrix4fv(view_uniform_loc, 1, false, reinterpret_cast<const t_f32 *>(&rc.state->view_mat));

        const auto proj_mat = CreateOrthographicMatrix(0.0f, static_cast<t_f32>(rc.framebuffer_size_cache.x), static_cast<t_f32>(rc.framebuffer_size_cache.y), 0.0f, -1.0f, 1.0f);
        const t_i32 proj_uniform_loc = glGetUniformLocation(rc.basis->batch_shader_prog_gl_id, "u_proj");

        glUniformMatrix4fv(proj_uniform_loc, 1, false, reinterpret_cast<const t_f32 *>(&proj_mat));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rc.state->tex_gl_id);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rc.basis->batch_mesh_gl_ids.elem_buf);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_batch_slot_elem_cnt * rc.state->batch_slots_used_cnt), GL_UNSIGNED_SHORT, nullptr);

        glUseProgram(0);

        rc.state->batch_slots_used_cnt = 0;
    }

    void internal::CompleteFrame(const s_rendering_context rc) {
        Flush(rc);
    }

    void Clear(const s_rendering_context rc, const s_color_rgba32f col) {
        glClearColor(col.R(), col.G(), col.B(), col.A());
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void SetViewMatrix(const s_rendering_context rc, const s_mat4x4 &mat) {
        Flush(rc);
        rc.state->view_mat = mat;
    }

    static void Draw(const s_rendering_context rc, const t_gl_id tex_gl_id, const s_rect_f tex_coords, const s_v2 pos, const s_v2 size, const s_v2 origin, const t_f32 rot, const s_color_rgba32f blend) {
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
        const s_static_array<s_v2, 4> vert_coords = {
            {0.0f - origin.x, 0.0f - origin.y},
            {1.0f - origin.x, 0.0f - origin.y},
            {1.0f - origin.x, 1.0f - origin.y},
            {0.0f - origin.x, 1.0f - origin.y},
        };

        const s_static_array<s_v2, 4> tex_coords_per_vert = {
            {tex_coords.Left(), tex_coords.Top()},
            {tex_coords.Right(), tex_coords.Top()},
            {tex_coords.Right(), tex_coords.Bottom()},
            {tex_coords.Left(), tex_coords.Bottom()},
        };

        t_batch_slot &slot = rc.state->batch_slots[rc.state->batch_slots_used_cnt];

        for (t_len i = 0; i < slot.g_len; i++) {
            slot[i] = {
                .vert_coord = vert_coords[i],
                .pos = pos,
                .size = size,
                .rot = rot,
                .tex_coord = tex_coords_per_vert[i],
                .blend = blend,
            };
        }

        // Update the count - we've used a slot!
        rc.state->batch_slots_used_cnt++;
    }

    void DrawTexture(const s_rendering_context rc, const s_gfx_resource &tex, const s_v2 pos, const s_rect_i src_rect, const s_v2 origin, const s_v2 scale, const t_f32 rot, const s_color_rgba32f blend) {
        ZF_ASSERT(IsOriginValid(origin));

        const auto tex_size = tex.Texture().size;

        s_rect_i src_rect_to_use;

        if (src_rect == s_rect_i()) {
            // If the source rectangle wasn't set, just go with the whole texture.
            src_rect_to_use = {0, 0, tex_size.x, tex_size.y};
        } else {
            ZF_ASSERT(src_rect.Left() >= 0 && src_rect.Top() >= 0 && src_rect.Right() <= tex_size.x && src_rect.Top() <= tex_size.y);
            src_rect_to_use = src_rect;
        }

        const s_rect_f tex_coords = CalcTextureCoords(src_rect_to_use, tex_size);

        const s_v2 size = {
            static_cast<t_f32>(src_rect_to_use.width) * scale.x,
            static_cast<t_f32>(src_rect_to_use.height) * scale.y,
        };

        Draw(rc, tex.Texture().gl_id, tex_coords, pos, size, origin, rot, blend);
    }

    void DrawRect(const s_rendering_context rc, const s_rect_f rect, const s_color_rgba32f color) {
        DrawTexture(rc, *rc.basis->px_tex, rect.Pos(), {}, {}, rect.Size(), 0.0f, color);
    }

    void DrawRectOpaqueOutlined(const s_rendering_context rc, const s_rect_f rect, const s_color_rgb24f fill_color, const s_color_rgba32f outline_color, const t_f32 outline_thickness) {
        ZF_ASSERT(outline_thickness > 0.0f);

        const s_rect_f outline_rect = {
            rect.x - outline_thickness,
            rect.y - outline_thickness,
            rect.width + (outline_thickness * 2.0f),
            rect.height + (outline_thickness * 2.0f),
        };

        DrawRect(rc, outline_rect, fill_color);

        DrawRect(rc, rect, fill_color);
    }

    void DrawRectRot(const s_rendering_context rc, const s_v2 pos, const s_v2 size, const s_v2 origin, const t_f32 rot, const s_color_rgba32f color) {
        DrawTexture(rc, *rc.basis->px_tex, pos, {}, origin, size, rot, color);
    }

    void DrawRectRotOpaqueOutlined(const s_rendering_context rc, const s_v2 pos, const s_v2 size, const s_v2 origin, const t_f32 rot, const s_color_rgb24f fill_color, const s_color_rgba32f outline_color, const t_f32 outline_thickness) {
        ZF_ASSERT(outline_thickness > 0.0f);

        DrawRectRot(rc, pos, {size.x + (outline_thickness * 2.0f), size.y + (outline_thickness * 2.0f)}, origin, rot, outline_color);
        DrawRectRot(rc, pos, size, origin, rot, fill_color);
    }

    void DrawLine(const s_rendering_context rc, const s_v2 a, const s_v2 b, const s_color_rgba32f blend, const t_f32 thickness) {
        ZF_ASSERT(thickness > 0.0f);

        const t_f32 len = CalcDist(a, b);
        const t_f32 dir = CalcDirInRads(a, b);
        DrawTexture(rc, *rc.basis->px_tex, a, {}, origins::g_centerleft, {len, thickness}, dir, blend);
    }

    t_b8 LoadStrChrDrawPositions(const s_str_rdonly str, const s_font_arrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, s_mem_arena &mem_arena, s_array<s_v2> &o_positions) {
        ZF_ASSERT(IsStrValidUTF8(str));
        ZF_ASSERT(IsAlignmentValid(alignment));

        // Calculate some useful string metadata.
        struct s_str_meta {
            t_len len = 0;
            t_len line_cnt = 0;
        };

        const auto str_meta = [str]() {
            s_str_meta meta = {.line_cnt = 1};

            ZF_WALK_STR(str, chr_info) {
                meta.len++;

                if (chr_info.code_pt == '\n') {
                    meta.line_cnt++;
                }
            }

            return meta;
        }();

        // Reserve memory for the character positions.
        if (!AllocArray(str_meta.len, mem_arena, o_positions)) {
            return false;
        }

        // From the line count we can determine the vertical alignment offset to apply.
        const t_f32 alignment_offs_y = static_cast<t_f32>(-(str_meta.line_cnt * font_arrangement.line_height)) * alignment.y;

        // Calculate the position of each character.
        t_len chr_index = 0;
        s_v2 chr_pos_pen = {}; // The position of the current character.
        t_len line_begin_chr_index = 0;
        t_len line_len = 0;
        t_code_pt code_pt_last;

        const auto apply_hor_alignment_offs = [&]() {
            if (line_len > 0) {
                const auto line_width = chr_pos_pen.x;

                for (t_len i = line_begin_chr_index; i < chr_index; i++) {
                    o_positions[i].x -= line_width * alignment.x;
                }
            }
        };

        ZF_WALK_STR(str, chr_info) {
            ZF_DEFER({
                chr_index++;
                code_pt_last = chr_info.code_pt;
            });

            if (line_len == 0) {
                line_begin_chr_index = chr_index;
            }

            if (chr_info.code_pt == '\n') {
                apply_hor_alignment_offs();

                chr_pos_pen.x = 0.0f;
                chr_pos_pen.y += static_cast<t_f32>(font_arrangement.line_height);

                line_len = 0;

                continue;
            }

            s_font_glyph_info glyph_info = {};

            if (!font_arrangement.code_pts_to_glyph_infos.Get(chr_info.code_pt, &glyph_info)) {
                ZF_ASSERT(false);
                return false;
            }

            if (chr_index > 0 && font_arrangement.has_kernings) {
                t_i32 kerning = 0;

                if (font_arrangement.code_pt_pairs_to_kernings.Get({code_pt_last, chr_info.code_pt}, &kerning)) {
                    chr_pos_pen.x += static_cast<t_f32>(kerning);
                }
            }

            o_positions[chr_index] = pos + chr_pos_pen + glyph_info.offs.ToV2();
            o_positions[chr_index].y += alignment_offs_y;

            chr_pos_pen.x += static_cast<t_f32>(glyph_info.adv);

            line_len++;
        }

        apply_hor_alignment_offs();

        return true;
    }

    t_b8 DrawStr(const s_rendering_context rc, const s_str_rdonly str, const s_gfx_resource &font, const s_v2 pos, const s_v2 alignment, const s_color_rgba32f blend, s_mem_arena &temp_mem_arena) {
        ZF_ASSERT(IsStrValidUTF8(str));
        ZF_ASSERT(IsAlignmentValid(alignment));

        if (str.IsEmpty()) {
            return true;
        }

        const auto &font_arrangement = font.Font().arrangement;
        const auto &font_atlas_gl_ids = font.Font().atlas_gl_ids;

        s_array<s_v2> chr_positions = {};

        if (!LoadStrChrDrawPositions(str, font_arrangement, pos, alignment, temp_mem_arena, chr_positions)) {
            return false;
        }

        t_len chr_index = 0;

        ZF_WALK_STR(str, chr_info) {
            if (chr_info.code_pt == ' ' || chr_info.code_pt == '\n') {
                chr_index++;
                continue;
            }

            s_font_glyph_info glyph_info = {};

            if (!font_arrangement.code_pts_to_glyph_infos.Get(chr_info.code_pt, &glyph_info)) {
                ZF_ASSERT(false);
                return false;
            }

            const auto chr_tex_coords = CalcTextureCoords(glyph_info.atlas_rect, g_font_atlas_size);

            Draw(rc, font_atlas_gl_ids[glyph_info.atlas_index], chr_tex_coords, chr_positions[chr_index], glyph_info.atlas_rect.Size().ToV2(), {}, 0.0f, blend);

            chr_index++;
        };

        return true;
    }
#endif

#if 0
    // ============================================================
    // @section: General
    // ============================================================
    t_b8 internal::InitGFX(const s_platform_layer_info &platform_layer_info) {
    #if 0
        //
        // BGFX Initialisation
        //
        bgfx::Init init = {};
        init.type = bgfx::RendererType::Count;

        init.resolution.reset = BGFX_RESET_VSYNC;

        const auto fb_size_cache = WindowFramebufferSizeCache(platform_layer_info);
        init.resolution.width = static_cast<uint32_t>(fb_size_cache.x);
        init.resolution.height = static_cast<uint32_t>(fb_size_cache.y);

        init.platformData.nwh = NativeWindowHandle(platform_layer_info);
        init.platformData.ndt = NativeDisplayHandle(platform_layer_info);
        init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(init)) {
            return false;
        }

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xABABABFF, 1.0f, 0);

        return true;
    #endif
    #if 0
        t_b8 clean_up = false;

        o_rendering_basis = Alloc<s_rendering_basis>(rendering_basis_mem_arena);

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
            s_array<t_u16> elems = {};

            if (!AllocArray(g_batch_slot_elem_cnt * g_batch_slot_cnt, temp_mem_arena, elems)) {
                ZF_REPORT_ERROR();
                clean_up = true;
                return false;
            }

            for (t_len i = 0; i < g_batch_slot_cnt; i++) {
                elems[(i * 6) + 0] = static_cast<t_u16>((i * 4) + 0);
                elems[(i * 6) + 1] = static_cast<t_u16>((i * 4) + 1);
                elems[(i * 6) + 2] = static_cast<t_u16>((i * 4) + 2);
                elems[(i * 6) + 3] = static_cast<t_u16>((i * 4) + 2);
                elems[(i * 6) + 4] = static_cast<t_u16>((i * 4) + 3);
                elems[(i * 6) + 5] = static_cast<t_u16>((i * 4) + 0);
            }

            constexpr t_len verts_len = g_batch_vert_component_cnt * g_batch_slot_vert_cnt * g_batch_slot_cnt;
            o_rendering_basis->batch_mesh_gl_ids = CreateGLMesh(nullptr, verts_len, elems, g_batch_vert_attr_lens);
        }

        ZF_DEFER({
            if (clean_up) {
                DestroyGLMesh(o_rendering_basis->batch_mesh_gl_ids);
            }
        });

        // Create the batch shader program.
        o_rendering_basis->batch_shader_prog_gl_id = CreateGLShaderProg(g_batch_vert_shader_src, g_batch_frag_shader_src, temp_mem_arena);

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

        // Set up resource arena.
        o_rendering_basis->res_arena = CreateGFXResourceArena(rendering_basis_mem_arena);

        ZF_DEFER({
            if (clean_up) {
                DestroyGFXResources(o_rendering_basis->res_arena);
            }
        });

        // Set up pixel texture.
        {
            const s_static_array<t_u8, 4> rgba = {255, 255, 255, 255};

            if (!CreateTexture({{1, 1}, rgba}, o_rendering_basis->res_arena, o_rendering_basis->px_tex)) {
                ZF_REPORT_ERROR();
                clean_up = true;
                return false;
            }
        }

        return true;
    #endif

        return true;
    }

    void internal::ShutdownGFX() {
    #if 0
        bgfx::shutdown();
    #endif

    #if 0
        DestroyGFXResources(rendering_basis.res_arena);
        glDeleteProgram(rendering_basis.batch_shader_prog_gl_id);
        DestroyGLMesh(rendering_basis.batch_mesh_gl_ids);

        rendering_basis = {};
    #endif
    }

    void internal::BeginFrame(const s_v2_i framebuffer_size_cache) {
    #if 0
        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(framebuffer_size_cache.x), static_cast<uint16_t>(framebuffer_size_cache.y));
        bgfx::touch(0);
    #endif
    }

    void internal::EndFrame() {
    #if 0
        bgfx::frame();
    #endif
    }
#endif
}
