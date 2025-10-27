namespace zf {
    class c_texture_group {
    public:
        bool LoadRaws(const c_array<const c_string_view> file_paths, c_mem_arena& mem_arena, c_gfx_resource_lifetime& gfx_res_lifetime, c_mem_arena& temp_mem_arena);
        void Unload();

        bool IsLoaded() const {
            return m_loaded;
        }

        int GetCnt() const {
            assert(m_loaded);
            return m_bgfx_hdls.Len();
        }

        bgfx::TextureHandle GetTextureBGFXHandle(const int index) const {
            assert(m_loaded);
            return m_bgfx_hdls[index];
        }

        s_v2_s32 GetTextureSize(const int index) const {
            assert(m_loaded);
            return m_sizes[index];
        }

    private:
        bool m_loaded;

        c_array<bgfx::TextureHandle> m_bgfx_hdls;
        c_array<s_v2_s32> m_sizes;
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
        c_array<const bgfx::ProgramHandle> m_bgfx_hdls = {};
    };

    /*class c_surface_group {
    public:
        bool LoadFromPacked(c_file_reader& fr, const int cnt, c_mem_arena& mem_arena);
        void Unload();

    private:
        c_array<const bgfx::FrameBufferHandle> m_frame_buf_bgfx_hdls = {};
        c_array<const bgfx::TextureHandle> m_tex_bgfx_hdls = {};
    };*/
}
