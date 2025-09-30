#pragma once

#include <bgfx/bgfx.h>
#include <zc.h>
#include "zf_window.h"

namespace zf {
    constexpr t_s32 g_quad_batch_slot_cnt = 8192;
    constexpr t_s32 g_quad_batch_slot_vert_cnt = 4;
    constexpr t_s32 g_quad_batch_slot_elem_cnt = 6;
    static_assert(g_quad_batch_slot_elem_cnt * g_quad_batch_slot_cnt <= USHRT_MAX, "Quad batch slot count is too high!");

    constexpr t_s32 g_view_limit = 128;
    constexpr t_s32 g_surf_limit = 128;
    static_assert(g_view_limit + g_surf_limit <= 256, "Needed view count exceeds BGFX limit!");

    class c_texture_group {
    public:
        bool LoadFromPacked(c_file_reader& fr, const int cnt, c_mem_arena& mem_arena);
        void Unload();

    private:
        c_array<const bgfx::TextureHandle> m_bgfx_hdls = {};
        c_array<const s_v2_s32> m_sizes = {};
    };

    class c_font_group {
    public:
        bool LoadFromPacked(c_file_reader& fr, const int cnt, c_mem_arena& mem_arena);
        void Unload();

    private:
        c_array<const bgfx::TextureHandle> m_bgfx_tex_hdls = {};
        c_array<const s_font_arrangement> m_arrangements = {};
        c_array<const s_font_texture_meta> m_tex_metas = {};
    };

    class c_shader_prog_group {
    public:
        bool LoadFromPacked(c_file_reader& fr, const int cnt, c_mem_arena& mem_arena);
        void Unload();

    private:
        c_array<const bgfx::ProgramHandle> m_bgfx_hdls = {};
    };

    class c_surface_group {
    public:
        bool LoadFromPacked(c_file_reader& fr, const int cnt, c_mem_arena& mem_arena);
        void Unload();

    private:
        c_array<const bgfx::FrameBufferHandle> m_frame_buf_bgfx_hdls = {};
        c_array<const bgfx::TextureHandle> m_tex_bgfx_hdls = {};
    };

    struct s_quad_batch_vert {
        s_v2 vert_coord = {};
        s_v2 pos = {};
        s_v2 size = {};
        float rot = 0.0f;
        s_v4 blend = {};
    };

    using t_quad_batch_slot = s_static_array<s_quad_batch_vert, g_quad_batch_slot_vert_cnt>;

    struct s_renderable {
        bgfx::ProgramHandle prog_bgfx_hdl = BGFX_INVALID_HANDLE;
        bgfx::DynamicVertexBufferHandle dvb_bgfx_hdl = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle index_bgfx_hdl = BGFX_INVALID_HANDLE;

        void Clean() {
            if (bgfx::isValid(index_bgfx_hdl)) bgfx::destroy(index_bgfx_hdl);
            if (bgfx::isValid(dvb_bgfx_hdl))   bgfx::destroy(dvb_bgfx_hdl);
            if (bgfx::isValid(prog_bgfx_hdl))  bgfx::destroy(prog_bgfx_hdl);

            *this = {};
        }
    };

    enum class ec_renderer_state {
        not_initted,
        initted,
        rendering
    };

    struct s_renderer_core {
        ec_renderer_state state = ec_renderer_state::not_initted;

        s_renderable quad_batch_renderable = {};

        t_s32 active_view_index = 0;
        s_static_array<s_matrix_4x4, g_view_limit> view_mats = {};

        s_static_array<t_quad_batch_slot, g_quad_batch_slot_cnt> quad_batch_slots = {};
        t_s32 quad_batch_slots_used_cnt = 0;
    };

    class c_renderer {
    public:
        c_renderer() = delete;
        c_renderer(const c_renderer&) = delete;
        c_renderer& operator=(const c_renderer&) = delete;

        static bool Init(c_mem_arena& temp_mem_arena);
        static void Shutdown();

        static void RefreshSize() {
            s_v2_s32 fb_size = c_window::GetFramebufferSize();
            bgfx::reset(static_cast<uint32_t>(fb_size.x), static_cast<uint32_t>(fb_size.y), BGFX_RESET_VSYNC);
        }

        static void UpdateViewMatrix(const t_s32 view_index, const s_matrix_4x4& mat) {
            assert(sm_core.state == ec_renderer_state::initted);
            sm_core.view_mats[view_index] = mat;
        }

        static void BeginFrame();
        static void EndFrame();
        static void SetView(const t_s32 view_index, const s_v4 clear_col = {});
        static void Clear(const s_v4 col);
        static void Draw(const s_v2 pos, const s_v2 size, const s_v2 origin = origins::g_origin_top_left, const float rot = 0.0f, const s_v4 blend = colors::g_white);

    private:
        static inline s_renderer_core sm_core = {};

        static void Flush();
    };
}
