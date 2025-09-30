#pragma once

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
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

    struct s_bgfx_resource_arena {
        c_array_list<bgfx::DynamicVertexBufferHandle> vert_bufs;
        c_array_list<bgfx::IndexBufferHandle> elem_bufs;
        c_array_list<bgfx::ProgramHandle> progs;
        c_array_list<bgfx::TextureHandle> textures;

        void Init(const int vert_buf_limit, const int elem_buf_limit, const int prog_limit, const int tex_limit);
        void Clean();
    };

    constexpr t_s32 g_batch_slot_cnt = 8192;
    constexpr t_s32 g_batch_slot_vert_cnt = 4;
    constexpr t_s32 g_batch_slot_elem_cnt = 6;
    static_assert(g_batch_slot_elem_cnt * g_batch_slot_cnt <= USHRT_MAX, "Batch slot count is too high!");

    struct s_quad_batch_vert {
        s_v2 vert_coord;
        s_v2 pos;
        s_v2 size;
        float rot = 0.0f;
        s_v4 blend;

        static bgfx::VertexLayout BuildLayout() {
            bgfx::VertexLayout layout;

            layout.begin()
                .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord2, 1, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
                .end();

            return layout;
        }
    };

    class c_renderer {
    public:
        c_renderer() = delete;
        c_renderer(const c_renderer&) = delete;
        c_renderer& operator=(const c_renderer&) = delete;

        static bool Init(c_mem_arena& temp_mem_arena);
        static void Shutdown();

        static void CompleteFrame();

        static void Clear(const s_v4 col);
        static void Draw(const s_v2 pos, const s_v2 size, const s_v2 origin = origins::g_origin_top_left, const float rot = 0.0f, const s_v4 blend = colors::g_white);
        static void Flush();

    private:
        using t_quad_batch_slot = s_static_array<s_quad_batch_vert, g_batch_slot_vert_cnt>;

        static inline s_bgfx_resource_arena sm_bgfx_res_arena;

        static inline bgfx::ProgramHandle sm_quad_batch_prog_bgfx_hdl;

        static inline bgfx::DynamicVertexBufferHandle sm_quad_batch_vb_bgfx_hdl;
        static inline bgfx::IndexBufferHandle sm_quad_batch_eb_bgfx_hdl;

        static inline s_static_array<t_quad_batch_slot, g_batch_slot_cnt> sm_quad_batch_slots;
        static inline t_s32 sm_quad_batch_slots_used_cnt = 0;
    };
}
