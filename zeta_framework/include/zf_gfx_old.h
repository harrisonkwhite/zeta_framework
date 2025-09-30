#pragma once

#include <bgfx/bgfx.h>
#include "zc_gfx.h"
#include "zc_math.h"
#include <zc.h>

namespace zf {
    enum e_gfx_resource_type {
        ek_quad_buf,
        ek_shader_prog,
        ek_texture,

        eks_gfx_resource_type_cnt
    };

    struct s_gfx_resource_hdl {
        e_gfx_resource_type type;
        t_s32 index;
    };

    struct s_renderable {
        bgfx::DynamicVertexBufferHandle dvb_bgfx_hdl;
        bgfx::IndexBufferHandle index_bgfx_hdl;
    };

    class c_gfx_resource_arena {
    public:
        bool Init(const s_static_array<int, eks_gfx_resource_type_cnt>& hdl_limits, c_mem_arena& mem_arena);
        void Clean();

        s_gfx_resource_hdl CreateVertBuf(const c_array<const float> verts);
        s_gfx_resource_hdl CreateVertBuf(const int vert_cnt);
        void UpdateVertBuf(const s_gfx_resource_hdl hdl, const c_array<const float> verts, const int begin_index = 0);

        s_gfx_resource_hdl CreateIndexBuf(const c_array<const t_u16> indices);

        s_gfx_resource_hdl CreateTexture(const s_rgba_texture rgba);

        s_gfx_resource_hdl CreateShaderProg(const c_array<const t_u8> vs_bin, const c_array<const t_u8> fs_bin);

    private:
        c_array_list<bgfx::DynamicVertexBufferHandle> m_vert_buf_bgfx_hdls;
        c_array_list<bgfx::DynamicVertexBufferHandle> m_index_buf_bgfx_hdls;
    };

    namespace renderer {
        struct s_texture {
            s_gfx_resource_hdl hdl;
            s_v2_s32 size;
        };

        bool Init();
        void Shutdown();

        void Clear(const int view_index, const s_v4 col);
        void DrawQuad(const int view_index, const s_gfx_resource_hdl vert_buf_hdl, const s_gfx_resource_hdl index_buf_hdl);
        void SetViewTransform(const int view_index, const s_matrix_4x4& view_mat, const s_matrix_4x4& proj_mat);
        void CompleteFrame();
    }
}
