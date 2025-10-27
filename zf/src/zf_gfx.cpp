#include <zf_rendering.h>

namespace zf {
    bool c_texture_group::LoadRaws(const c_array<const c_string_view> file_paths, c_mem_arena& mem_arena, c_gfx_resource_lifetime& gfx_res_lifetime, c_mem_arena& temp_mem_arena) {
        assert(!m_loaded);

        m_bgfx_hdls = PushArrayToMemArena<bgfx::TextureHandle>(mem_arena, file_paths.Len());

        if (m_bgfx_hdls.IsEmpty()) {
            return false;
        }

        m_sizes = PushArrayToMemArena<s_v2_s32>(mem_arena, file_paths.Len());

        if (m_sizes.IsEmpty()) {
            return false;
        }

        for (int i = 0; i < file_paths.Len(); i++) {
            s_rgba_texture rgba;

            if (!LoadRGBATextureFromRawFile(rgba, temp_mem_arena, file_paths[i])) {
                return false;
            }

            m_bgfx_hdls[i] = bgfx::createTexture2D(static_cast<uint16_t>(rgba.dims.x), static_cast<uint16_t>(rgba.dims.y), false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(rgba.px_data.Raw(), rgba.px_data.Len()));

            if (!bgfx::isValid(m_bgfx_hdls[i])) {
                return false;
            }

            gfx_res_lifetime.AddBGFXResource(m_bgfx_hdls[i]);

            m_sizes[i] = rgba.dims;
        }

        m_loaded = true;

        return true;
    }
}
