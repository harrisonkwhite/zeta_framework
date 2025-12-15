#include <zgl/zgl_rendering.h>

#include <zgl/zgl_platform.h>

namespace zf {
    constexpr s_color_rgb8 g_bg_color_default = {109, 187, 255};

    struct s_batch_vert {
        s_v2 pos;
        s_color_rgba32f blend;
        s_v2 uv;

        s_batch_vert() = default;
        s_batch_vert(const s_v2 pos, const s_color_rgba32f blend, const s_v2 uv) : pos(pos), blend(blend), uv(uv) {}
    };

    constexpr s_static_array<t_i32, 3> g_batch_vert_attr_component_cnts = {2, 4, 2}; // This has to match the number of components per attribute above.

    constexpr t_len g_batch_vert_component_cnt = ZF_SIZE_OF(s_batch_vert) / ZF_SIZE_OF(t_f32);

    static_assert([]() {
        t_len sum = 0;

        for (t_len i = 0; i < g_batch_vert_attr_component_cnts.g_len; i++) {
            sum += g_batch_vert_attr_component_cnts[i];
        }

        return sum == g_batch_vert_component_cnt;
    }());

    constexpr t_len g_batch_vert_limit = 1024;

    constexpr s_cstr_literal g_batch_vert_shader_src = R"(#version 330 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec4 a_blend;
layout (location = 2) in vec2 a_uv;

out vec4 v_blend;
out vec2 v_uv;

uniform mat4 u_proj;

void main() {
    gl_Position = u_proj * vec4(a_vert, 0.0, 1.0);
    v_blend = a_blend;
    v_uv = a_uv;
}
)";

    constexpr s_cstr_literal g_batch_frag_shader_src = R"(#version 330 core

in vec4 v_blend;
in vec2 v_uv;

out vec4 o_frag_color;

void main() {
    o_frag_color = v_blend;
}
)";

    s_rendering_basis CreateRenderingBasis(s_mem_arena &mem_arena, s_mem_arena &temp_mem_arena) {
        s_rendering_basis basis = {
            .gfx_res_arena = CreateGFXResourceArena(mem_arena),
        };

        basis.batch_mesh = &CreateMesh(nullptr, g_batch_vert_component_cnt * g_batch_vert_limit, true, g_batch_vert_attr_component_cnts, basis.gfx_res_arena);

        if (!CreateShaderProg(g_batch_vert_shader_src, g_batch_frag_shader_src, basis.gfx_res_arena, temp_mem_arena, basis.batch_shader_prog)) {
            ZF_FATAL();
        }

        constexpr s_static_array<t_u8, 4> white_px_tex_rgba = {255, 255, 255, 255};

        if (!CreateTexture({{1, 1}, white_px_tex_rgba}, basis.gfx_res_arena, basis.white_px_texture)) {
            ZF_FATAL();
        }

        return basis;
    }

    struct s_rendering_state {
        s_ptr<const s_rendering_basis> basis;

        s_ptr<s_mem_arena> mem_arena;

        s_render_instr_seq instr_seq;

        s_static_list<s_batch_vert, g_batch_vert_limit> batch_verts;
        s_ptr<const s_gfx_resource> batch_texture;
    };

    s_rendering_state &internal::BeginRendering(const s_rendering_basis &basis, s_mem_arena &mem_arena) {
        auto &state = Alloc<s_rendering_state>(mem_arena);

        state.basis = &basis;
        state.mem_arena = &mem_arena;
        state.instr_seq = {mem_arena};

        DrawClear(state, g_bg_color_default);

        return state;
    }

    static void Flush(s_rendering_state &rs) {
        const auto verts = rs.batch_verts.ToArray();
        const s_array<t_f32> verts_f32 = {reinterpret_cast<t_f32 *>(verts.Ptr().Raw()), verts.SizeInBytes() / ZF_SIZE_OF(t_f32)};
        rs.instr_seq.SubmitMeshUpdate(*rs.basis->batch_mesh, verts_f32);

        rs.instr_seq.SubmitShaderProgSet(*rs.basis->batch_shader_prog);

        const auto fb_size_cache = WindowFramebufferSizeCache();

        auto proj_mat = CreateIdentityMatrix();
        proj_mat.elems[0][0] = 1.0f / (static_cast<t_f32>(fb_size_cache.x) / 2.0f);
        proj_mat.elems[1][1] = -1.0f / (static_cast<t_f32>(fb_size_cache.y) / 2.0f);
        proj_mat.elems[3][0] = -1.0f;
        proj_mat.elems[3][1] = 1.0f;

        rs.instr_seq.SubmitShaderProgUniformSet(s_cstr_literal("u_proj"), proj_mat);

        rs.instr_seq.SubmitMeshDraw(*rs.basis->batch_mesh, rs.batch_texture ? rs.batch_texture : rs.basis->white_px_texture);

        rs.batch_texture = nullptr;
    }

    void internal::EndRendering(s_rendering_state &rs, s_mem_arena &temp_mem_arena) {
        Flush(rs);
        rs.instr_seq.Exec(temp_mem_arena);
        internal::SwapWindowBuffers();
    }

    void DrawClear(s_rendering_state &rs, const s_color_rgb24f col) {
        rs.instr_seq.SubmitClear(col);
    }

    void DrawTriangle(s_rendering_state &rs, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors) {
        if (rs.batch_verts.Len() + pts.g_len > rs.batch_verts.Cap()) {
            Flush(rs);
        }

        rs.batch_verts.Append({pts[0], pt_colors[0], {}});
        rs.batch_verts.Append({pts[1], pt_colors[1], {}});
        rs.batch_verts.Append({pts[2], pt_colors[2], {}});
    }

    void DrawRect(s_rendering_state &rs, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft) {
        if (rs.batch_verts.Len() + 6 > rs.batch_verts.Cap() || rs.batch_texture) {
            Flush(rs);
        }

        rs.batch_verts.Append({rect.TopLeft(), color_topleft, {}});
        rs.batch_verts.Append({rect.TopRight(), color_topright, {}});
        rs.batch_verts.Append({rect.BottomRight(), color_bottomright, {}});

        rs.batch_verts.Append({rect.BottomRight(), color_bottomright, {}});
        rs.batch_verts.Append({rect.BottomLeft(), color_bottomleft, {}});
        rs.batch_verts.Append({rect.TopLeft(), color_topleft, {}});
    }

    void DrawTexture(s_rendering_state &rs, const s_gfx_resource &tex, const s_v2 pos, const s_color_rgba32f blend) {
        if (rs.batch_verts.Len() + 6 > rs.batch_verts.Cap()
            || (!rs.batch_verts.IsEmpty() && rs.batch_texture != s_ptr<const s_gfx_resource>(&tex))) {
            Flush(rs);
        }

        const s_rect_f rect = {pos, TextureSize(tex).ToV2()};

        rs.batch_verts.Append({rect.TopLeft(), blend, {0.0f, 0.0f}});
        rs.batch_verts.Append({rect.TopRight(), blend, {1.0f, 0.0f}});
        rs.batch_verts.Append({rect.BottomRight(), blend, {1.0f, 1.0f}});

        rs.batch_verts.Append({rect.BottomRight(), blend, {1.0f, 1.0f}});
        rs.batch_verts.Append({rect.BottomLeft(), blend, {0.0f, 1.0f}});
        rs.batch_verts.Append({rect.TopLeft(), blend, {0.0f, 0.0f}});
    }
}
