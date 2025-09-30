#pragma once

#include <bgfx/bgfx.h>
#include <zc.h>
#include "zc_gfx.h"
#include "zc_math.h"
#include "zc_mem.h"
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
        bool LoadRaws(const c_array<const c_string_view> file_paths, c_mem_arena& mem_arena, c_mem_arena& temp_mem_arena) {
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

                m_sizes[i] = rgba.dims;
            }

            m_loaded = true;

            return true;
        }

        void Unload() {
            assert(m_loaded);
        }

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

        c_array<bgfx::TextureHandle> m_bgfx_hdls = {};
        c_array<s_v2_s32> m_sizes = {};
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
        s_v2 vert_coord;
        s_v2 pos;
        s_v2 size;
        float rot = 0.0f;
        s_v2 uv;
        s_v4 blend;
    };

    using t_quad_batch_slot = s_static_array<s_quad_batch_vert, g_quad_batch_slot_vert_cnt>;

    struct s_renderable {
        bgfx::ProgramHandle prog_bgfx_hdl = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle tex_uni_bgfx_hdl = BGFX_INVALID_HANDLE;
        bgfx::DynamicVertexBufferHandle dvb_bgfx_hdl = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle index_bgfx_hdl = BGFX_INVALID_HANDLE;

        void Clean() {
            bgfx::destroy(index_bgfx_hdl);
            bgfx::destroy(dvb_bgfx_hdl);
            bgfx::destroy(prog_bgfx_hdl);

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

        s_renderable quad_batch_renderable;

        t_s32 active_view_index = 0;
        s_static_array<s_matrix_4x4, g_view_limit> view_mats;

        s_static_array<t_quad_batch_slot, g_quad_batch_slot_cnt> quad_batch_slots;
        t_s32 quad_batch_slots_used_cnt = 0;
        bgfx::TextureHandle quad_batch_tex_bgfx_hdl = BGFX_INVALID_HANDLE;
    };

    class c_renderer {
    public:
        c_renderer() = delete;
        c_renderer(const c_renderer&) = delete;
        c_renderer& operator=(const c_renderer&) = delete;

        static bool Init(c_mem_arena& temp_mem_arena);
        static void Shutdown();

        static void RefreshSize() {
            assert(sm_core.state == ec_renderer_state::initted);

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
        static void Draw(const int tex_index, const c_texture_group& tex_group, const s_v2 pos, const s_v2 size, const s_v2 origin = origins::g_origin_top_left, const float rot = 0.0f, const s_v4 blend = colors::g_white);

    private:
        static inline s_renderer_core sm_core;

        static void Flush();
    };
}
