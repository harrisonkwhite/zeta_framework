#pragma once

#include <zc.h>
#include <bgfx/bgfx.h>

namespace zf {
    enum class ec_bgfx_resource_type : t_bgfx_resource_hdl {
        prog,
        uniform,
        dynamic_vert_buf,
        index_buf,
        texture
    };

    struct s_bgfx_resource_hdl_wrapper {
        t_bgfx_resource_hdl hdl = BGFX_INVALID_HANDLE;
        ec_bgfx_resource_type type;

        s_bgfx_resource_hdl_wrapper() = default;
        s_bgfx_resource_hdl_wrapper(const bgfx::ProgramHandle prog_hdl) : hdl(prog_hdl.idx), type(ec_bgfx_resource_type::prog) {}
        s_bgfx_resource_hdl_wrapper(const bgfx::UniformHandle uni_hdl) : hdl(uni_hdl.idx), type(ec_bgfx_resource_type::uniform) {}
        s_bgfx_resource_hdl_wrapper(const bgfx::DynamicVertexBufferHandle dvb_hdl) : hdl(dvb_hdl.idx), type(ec_bgfx_resource_type::dynamic_vert_buf) {}
        s_bgfx_resource_hdl_wrapper(const bgfx::IndexBufferHandle ib_hdl) : hdl(ib_hdl.idx), type(ec_bgfx_resource_type::index_buf) {}
        s_bgfx_resource_hdl_wrapper(const bgfx::TextureHandle tex_hdl) : hdl(tex_hdl.idx), type(ec_bgfx_resource_type::texture) {}

        bool IsValid() const {
            const t_bgfx_resource_hdl invalid = BGFX_INVALID_HANDLE;
            return hdl != invalid;
        }
    };

    struct s_texture {
        // @idea: OPTIONALLY keep pixel data? nullptr otherwise?
        s_v2_s32 size;
        bgfx::TextureHandle bgfx_hdl = BGFX_INVALID_HANDLE;

        //bool LoadFromRaw(const c_string_view file_path, c_gfx_resource_lifetime& gfx_res_lifetime);
        //void LoadFromPacked(c_file_reader& fr, c_gfx_resource_lifetime& gfx_res_lifetime); // @todo: Just ask for the buffer for this specific packing. You can easily pull that from a file, store temporarily in a buffer, then pass into here. Much more flexible than a file explicitly.
    };

    struct s_font {
    };


}
