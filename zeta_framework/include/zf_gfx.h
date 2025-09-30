#pragma once

#include "zc_gfx.h"
#include <zc.h>

namespace zf {
    enum class ec_gfx_resource_type {
        invalid,
        vert_buf,
        index_buf,
        shader_prog,
        texture
    };

    struct s_gfx_resource_hdl {
        t_u16 bgfx;
        ec_gfx_resource_type type;

        bool IsValid() {
            return type != ec_gfx_resource_type::invalid;
        }
    };

    class c_gfx_resource_arena {
    public:
        void Init(const int hdl_limit);
        void Clean();

        s_gfx_resource_hdl CreateVertBuf(const c_array<const float> verts);
        s_gfx_resource_hdl CreateVertBuf(const int vert_cnt);

        s_gfx_resource_hdl CreateIndexBuf(const c_array<const t_u16> indices);

        s_gfx_resource_hdl CreateTexture(const s_rgba_texture rgba);

        s_gfx_resource_hdl CreateShaderProg(const c_array<const t_u8> vs_bin, const c_array<const t_u8> fs_bin);

    private:
        c_array_list<s_gfx_resource_hdl> m_hdls;
    };

    class c_renderer {
    public:
        bool Init();
        void Shutdown();

        void Clear(const int view_index);
        void Draw(const int view_index);
        void Frame();

    private:
    };
}
