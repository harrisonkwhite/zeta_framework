#include <zf/zf_renderer.h>

#include <glad/glad.h>

namespace zf::renderer {
    using t_gl_id = GLuint;

    struct s_mesh_gl_ids {
        t_gl_id vert_arr_gl_id;
        t_gl_id vert_buf_gl_id;
        t_gl_id elem_buf_gl_id;
    };

    static t_size CalcStride(const s_array_rdonly<t_s32> vert_attr_lens) {
        t_size stride = 0;

        for (t_size i = 0; i < vert_attr_lens.len; i++) {
            stride += ZF_SIZE_OF(t_f32) * static_cast<t_size>(vert_attr_lens[i]);
        }

        return stride;
    }

    static s_mesh_gl_ids MakeGLMesh(const t_f32* const verts_raw, const t_size verts_len, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_s32> vert_attr_lens) {
        s_mesh_gl_ids mesh = {};

        glGenVertexArrays(1, &mesh.vert_arr_gl_id);
        glBindVertexArray(mesh.vert_arr_gl_id);

        glGenBuffers(1, &mesh.vert_buf_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vert_buf_gl_id);
        glBufferData(GL_ARRAY_BUFFER, ZF_SIZE_OF(t_f32) * verts_len, verts_raw, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &mesh.elem_buf_gl_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.elem_buf_gl_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(ArraySizeInBytes(elems)), elems.buf_raw, GL_STATIC_DRAW);

        const t_size stride = CalcStride(vert_attr_lens);
        t_s32 offs = 0;

        for (t_size i = 0; i < vert_attr_lens.len; i++) {
            const t_s32 attr_len = vert_attr_lens[i];

            glVertexAttribPointer(static_cast<GLuint>(i), attr_len, GL_FLOAT, false, static_cast<GLsizei>(stride), reinterpret_cast<void*>(ZF_SIZE_OF(t_s32) * offs));
            glEnableVertexAttribArray(static_cast<GLuint>(i));

            offs += attr_len;
        }

        glBindVertexArray(0);

        return mesh;
    }

    static void ReleaseGLMesh(const s_mesh_gl_ids& mesh_gl_ids) {
        glDeleteBuffers(1, &mesh_gl_ids.elem_buf_gl_id);
        glDeleteBuffers(1, &mesh_gl_ids.vert_buf_gl_id);
        glDeleteVertexArrays(1, &mesh_gl_ids.vert_arr_gl_id);
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

                if (log_chr_cnt > 1) {
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

        return prog_gl_id;
    }

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

#ifndef ZF_TEXTURE_LIMIT
    #define ZF_TEXTURE_LIMIT 1024
#endif

#ifndef ZF_FONT_LIMIT
    #define ZF_FONT_LIMIT 128
#endif

    enum e_phase {
        ek_phase_uninitted,
        ek_phase_initting,
        ek_phase_initted,
        ek_phase_rendering
    };

    static struct {
        e_phase phase;

        s_mesh_gl_ids batch_mesh_gl_ids;
        t_gl_id batch_shader_prog_gl_id;

        struct {
            s_static_array<t_gl_id, ZF_TEXTURE_LIMIT> gl_ids;
            s_static_array<s_v2<t_s32>, ZF_TEXTURE_LIMIT> sizes;
            s_static_bit_vec<ZF_TEXTURE_LIMIT> activity;
        } textures;

        struct {
            s_static_array<s_mem_arena, ZF_FONT_LIMIT> mem_arenas;
            s_static_array<s_font_arrangement, ZF_FONT_LIMIT> arrangements;
            s_static_array<s_array<t_gl_id>, ZF_FONT_LIMIT> atlas_gl_id_arrs;
            s_static_bit_vec<ZF_FONT_LIMIT> activity;
        } fonts;

        s_resource_hdl px_tex_hdl;

        struct {
            s_static_array<t_batch_slot, g_batch_slot_cnt> batch_slots;
            t_size batch_slots_used_cnt;

            s_matrix_4x4 view_mat; // The view matrix to be used when flushing.
            s_resource_hdl tex_hdl; // The texture to be used when flushing.
        } rendering;
    } g_state;

    t_b8 Init(s_mem_arena& temp_mem_arena) {
        ZeroOut(g_state);

        t_b8 clean_up = false;

        g_state.phase = ek_phase_initting;

        ZF_DEFER({
            if (clean_up) {
                g_state.phase = ek_phase_uninitted;
            }
        });

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
            g_state.batch_mesh_gl_ids = MakeGLMesh(nullptr, verts_len, elems, g_batch_vert_attr_lens);
        }

        ZF_DEFER({
            if (clean_up) {
                ReleaseGLMesh(g_state.batch_mesh_gl_ids);
            }
        });

        // Create the batch shader program.
        g_state.batch_shader_prog_gl_id = MakeGLShaderProg(g_batch_vert_shader_src, g_batch_frag_shader_src, temp_mem_arena);

        if (!g_state.batch_shader_prog_gl_id) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        ZF_DEFER({
            if (clean_up) {
                glDeleteProgram(g_state.batch_shader_prog_gl_id);
                g_state.batch_shader_prog_gl_id = 0;
            }
        });

        // Create the pixel texture (for rectangles, lines, etc.)
        {
            const s_static_array<t_u8, 4> rgba = {{255, 255, 255, 255}};
            g_state.px_tex_hdl = CreateTexture({{1, 1}, rgba});

            if (!IsResourceValid(g_state.px_tex_hdl)) {
                ZF_REPORT_ERROR();
                clean_up = true;
                return false;
            }
        }

        g_state.phase = ek_phase_initted;

        return true;
    }

    void Shutdown() {
        ZF_ASSERT(g_state.phase == ek_phase_initted);

        ZF_FOR_EACH_SET_BIT(g_state.textures.activity, i) {
            glDeleteTextures(1, &g_state.textures.gl_ids[i]);
        }

        ReleaseGLMesh(g_state.batch_mesh_gl_ids);
        glDeleteProgram(g_state.batch_shader_prog_gl_id);
    }

    static inline s_v2<t_s32> GLTextureSizeLimit() {
        t_s32 size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
        return { size, size };
    }

    static t_gl_id CreateGLTexture(const s_rgba_texture_data_rdonly& tex_data) {
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

    s_resource_hdl CreateTexture(const s_rgba_texture_data_rdonly& tex_data) {
        ZF_ASSERT(g_state.phase == ek_phase_initting || g_state.phase == ek_phase_initted);

        const t_size index = IndexOfFirstUnsetBit(g_state.textures.activity);

        if (index == -1) {
            LogError("Out of room - no available slots for new texture!");
            return {};
        }

        SetBit(g_state.textures.activity, index);

        const t_gl_id gl_id = CreateGLTexture(tex_data);

        g_state.textures.gl_ids[index] = gl_id;
        g_state.textures.sizes[index] = tex_data.size_in_pxs;

        return {ek_resource_type_texture, index};
    }

    s_v2<t_s32> TextureSize(const s_resource_hdl hdl) {
        return g_state.textures.sizes[hdl.index];
    }

    [[nodiscard]] static t_b8 MakeFontAtlasGLTextures(const s_array_rdonly<t_font_atlas_rgba> atlas_rgbas, s_array<t_gl_id>& o_gl_ids) {
        o_gl_ids = {
            .buf_raw = static_cast<t_gl_id*>(malloc(static_cast<size_t>(ZF_SIZE_OF(t_gl_id) * atlas_rgbas.len))),
            .len = atlas_rgbas.len
        };

        if (!o_gl_ids.buf_raw) {
            return false;
        }

        for (t_size i = 0; i < atlas_rgbas.len; i++) {
            o_gl_ids[i] = CreateGLTexture({g_font_atlas_size, atlas_rgbas[i]});

            if (!o_gl_ids[i]) {
                return false;
            }
        }

        return true;
    }

    s_resource_hdl CreateFontFromRaw(const s_str_rdonly file_path, const t_s32 height, const t_unicode_code_pt_bit_vec& code_pts, s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena, e_font_load_from_raw_result* const o_load_from_raw_res, t_unicode_code_pt_bit_vec* const o_unsupported_code_pts) {
        const t_size index = IndexOfFirstUnsetBit(g_state.fonts.activity);

        if (index == -1) {
            LogError("Out of room - no available slots for new font!");
            return {};
        }

        SetBit(g_state.fonts.activity, index);

#if 0
        s_array<t_font_atlas_rgba> atlas_rgbas;

        const auto res = LoadFontFromRaw(file_path, height, code_pts, mem_arena, temp_mem_arena, temp_mem_arena, g_state.fonts.arrangements[index], atlas_rgbas, o_unsupported_code_pts);

        if (o_load_from_raw_res) {
            *o_load_from_raw_res = res;
        }

        if (res != ek_font_load_from_raw_result_success) {
            return {};
        }

        if (!MakeFontAtlasGLTextures(atlas_rgbas, g_state.fonts.atlas_gl_id_arrs[index])) {
            return {};
        }
#endif

        return {ek_resource_type_font, index};
    }

    // ============================================================
    // @section: Rendering
    // ============================================================
    static void Flush() {
        if (g_state.rendering.batch_slots_used_cnt == 0) {
            // Nothing to flush!
            return;
        }

        //
        // Submitting Vertex Data to GPU
        //
        glBindVertexArray(g_state.batch_mesh_gl_ids.vert_arr_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, g_state.batch_mesh_gl_ids.vert_buf_gl_id);

        {
            const t_size write_size = ZF_SIZE_OF(t_batch_slot) * g_state.rendering.batch_slots_used_cnt;
            glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, g_state.rendering.batch_slots.buf_raw);
        }

        //
        // Rendering the Batch
        //
        glUseProgram(g_state.batch_shader_prog_gl_id);

        const t_s32 view_uniform_loc = glGetUniformLocation(g_state.batch_shader_prog_gl_id, "u_view");
        glUniformMatrix4fv(view_uniform_loc, 1, false, reinterpret_cast<const t_f32*>(&g_state.rendering.view_mat));

        const s_rect<t_s32> viewport = []() {
            s_rect<t_s32> vp;
            glGetIntegerv(GL_VIEWPORT, reinterpret_cast<t_s32*>(&vp));
            return vp;
        }();

        const auto proj_mat = MakeOrthographicMatrix4x4(0.0f, static_cast<t_f32>(viewport.width), static_cast<t_f32>(viewport.height), 0.0f, -1.0f, 1.0f);
        const t_s32 proj_uniform_loc = glGetUniformLocation(g_state.batch_shader_prog_gl_id, "u_proj");
        glUniformMatrix4fv(proj_uniform_loc, 1, false, reinterpret_cast<const t_f32*>(&proj_mat));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_state.textures.gl_ids[g_state.rendering.tex_hdl.index]);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_state.batch_mesh_gl_ids.elem_buf_gl_id);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_batch_slot_elem_cnt * g_state.rendering.batch_slots_used_cnt), GL_UNSIGNED_SHORT, nullptr);

        glUseProgram(0);

        g_state.rendering.batch_slots_used_cnt = 0;
    }

    void BeginRenderingPhase() {
        ZF_ASSERT(g_state.phase == ek_phase_initted);

        ZeroOut(g_state.rendering);
        g_state.rendering.view_mat = MakeIdentityMatrix4x4();

        g_state.phase = ek_phase_rendering;

        Clear(s_color_rgba8(147, 207, 249, 255));
    }

    void EndRenderingPhase() {
        ZF_ASSERT(g_state.phase == ek_phase_rendering);

        Flush();
        g_state.phase = ek_phase_initted;
    }

    void Clear(const s_color_rgba32f col) {
        ZF_ASSERT(g_state.phase == ek_phase_rendering);

        glClearColor(col.r, col.g, col.b, col.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void SetViewMatrix(const s_matrix_4x4& mat) {
        ZF_ASSERT(g_state.phase == ek_phase_rendering);

        Flush();
        g_state.rendering.view_mat = mat;
    }

    static void Draw(const s_resource_hdl tex_hdl, const s_rect<t_f32> tex_coords, s_v2<t_f32> pos, s_v2<t_f32> size, s_v2<t_f32> origin, const t_f32 rot, const s_color_rgba32f blend) {
        ZF_ASSERT(g_state.phase == ek_phase_rendering);

        if (g_state.rendering.batch_slots_used_cnt == 0) {
            // This is the first draw to the batch, so set the texture associated with the batch to the one we're trying to render.
            g_state.rendering.tex_hdl = tex_hdl;
        } else if (g_state.rendering.batch_slots_used_cnt == g_batch_slot_cnt || !AreResourcesEqual(tex_hdl, g_state.rendering.tex_hdl)) {
            // Flush the batch and then try this same render operation again but on a fresh batch.
            Flush();
            Draw(tex_hdl, tex_coords, pos, size, origin, rot, blend);
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

        t_batch_slot& slot = g_state.rendering.batch_slots[g_state.rendering.batch_slots_used_cnt];

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
        g_state.rendering.batch_slots_used_cnt++;
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

    void DrawTexture(const s_resource_hdl hdl, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect, const s_v2<t_f32> origin, const s_v2<t_f32> scale, const t_f32 rot, const s_color_rgba32f blend) {
        ZF_ASSERT(g_state.phase == ek_phase_rendering);
        ZF_ASSERT(origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f); // @todo: Generic function for this check?
        // @todo: Add more assertions here!

        const auto tex_size = TextureSize(hdl);

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

        Draw(hdl, tex_coords, pos, size, origin, rot, blend);
    }

    void DrawRect(const s_rect<t_f32> rect, const s_color_rgba32f color) {
        ZF_ASSERT(g_state.phase == ek_phase_rendering);
        DrawTexture(g_state.px_tex_hdl, RectPos(rect), {}, {}, RectSize(rect), 0.0f, color);
    }

    void DrawLine(const s_v2<t_f32> a, const s_v2<t_f32> b, const s_color_rgba32f blend, const t_f32 width) {
        ZF_ASSERT(g_state.phase == ek_phase_rendering);
        ZF_ASSERT(width > 0.0f);

        const t_f32 len = CalcDist(a, b);
        const t_f32 dir = CalcDirInRads(a, b);
        DrawTexture(g_state.px_tex_hdl, a, {}, origins::g_centerleft, {len, width}, dir, blend);
    }
}
