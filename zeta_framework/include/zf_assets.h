#pragma once

#include <bgfx/bgfx.h>
#include <zc.h>

namespace zf {
    class c_texture_group {
    public:
        bool LoadFromPacked(c_file_reader& fr, const int cnt, c_mem_arena& mem_arena);
        void Unload();

    private:
        c_array<const bgfx::TextureHandle> m_bgfx_hdls;
        c_array<const s_v2_s32> m_sizes;
    };

    class c_font_group {
    public:
        bool LoadFromPacked(c_file_reader& fr, const int cnt, c_mem_arena& mem_arena);
        void Unload();

    private:
        c_array<const bgfx::TextureHandle> m_bgfx_tex_hdls;
        c_array<const s_font_arrangement> m_arrangements;
        c_array<const s_font_texture_meta> m_tex_metas;
    };

    class c_shader_prog_group {
    public:
        bool LoadFromPacked(c_file_reader& fr, const int cnt, c_mem_arena& mem_arena);
        void Unload();

    private:
        c_array<const bgfx::ProgramHandle> m_bgfx_hdls;
    };

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
