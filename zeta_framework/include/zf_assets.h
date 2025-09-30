#pragma once

#include <zc.h>
#include "zf_rendering.h"

namespace zf {
    class c_asset_arena {
    public:
        bool LoadFromPacked(const c_string_view packed_file_path, c_mem_arena& mem_arena) {
            if (!m_textures.LoadFromPacked(, , mem_arena)) {
                return false;
            }
        }

        void Unload() {
            m_shader_progs.Unload();
            m_fonts.Unload();
            m_textures.Unload();
        }

    private:
        c_texture_group m_textures;
        c_font_group m_fonts;
        c_shader_prog_group m_shader_progs;
    };
}
