#pragma once

#include <bgfx/bgfx.h>
#include <zc.h>

// @todo:
// - Ensure that everything is possible WITHOUT using the asset packer.
// - Move most string utility functions to the generic core library.

// @todo: It might actually be simpler to NOT do this grouping. Textures are already grouped by arena/lifetime. Let the developer set up their own organisational systems (e.g. a single array containing textures from BOTH raw and packed sources). Only downside might be caching of texture sizes...

namespace zf {
    constexpr t_s32 g_quad_batch_slot_cnt = 8192;
    constexpr t_s32 g_quad_batch_slot_vert_cnt = 4;
    constexpr t_s32 g_quad_batch_slot_elem_cnt = 6;
    static_assert(g_quad_batch_slot_elem_cnt * g_quad_batch_slot_cnt <= USHRT_MAX, "Quad batch slot count is too high!");

    constexpr t_s32 g_view_limit = 128;
    constexpr t_s32 g_surf_limit = 128;
    static_assert(g_view_limit + g_surf_limit <= 256, "Needed view count exceeds BGFX limit!");

    using t_bgfx_resource_hdl = t_u16;
    static_assert(std::is_same_v<decltype(bgfx::ProgramHandle::idx), t_bgfx_resource_hdl>, "bgfx::ProgramHandle::idx is not t_u16!");

    class c_gfx_resource_lifetime {
    public:
        bool Init(c_mem_arena& mem_arena, const int hdl_limit = 1024) {
            assert(hdl_limit > 0);

            m_hdls = PushArrayToMemArena<s_bgfx_resource_hdl_wrapper>(mem_arena, hdl_limit);

            return !m_hdls.IsEmpty();
        }

        void Clean() {
            for (int i = 0; i < m_hdls_used_cnt; i++) {
                switch (m_hdls[i].type) {
                    case ec_bgfx_resource_type::prog:
                        bgfx::destroy(bgfx::ProgramHandle{m_hdls[i].hdl});
                        break;

                    case ec_bgfx_resource_type::uniform:
                        bgfx::destroy(bgfx::UniformHandle{m_hdls[i].hdl});
                        break;

                    case ec_bgfx_resource_type::dynamic_vert_buf:
                        bgfx::destroy(bgfx::DynamicVertexBufferHandle{m_hdls[i].hdl});
                        break;

                    case ec_bgfx_resource_type::index_buf:
                        bgfx::destroy(bgfx::IndexBufferHandle{m_hdls[i].hdl});
                        break;

                    case ec_bgfx_resource_type::texture:
                        bgfx::destroy(bgfx::TextureHandle{m_hdls[i].hdl});
                        break;
                }
            }

            *this = {};
        }

        void AddBGFXResource(const s_bgfx_resource_hdl_wrapper hdl_wrapper) {
            assert(m_hdls_used_cnt < m_hdls.Len());
            assert(hdl_wrapper.IsValid());

            m_hdls[m_hdls_used_cnt] = hdl_wrapper;
            m_hdls_used_cnt++;
        }

    private:
        c_array<s_bgfx_resource_hdl_wrapper> m_hdls;
        t_s32 m_hdls_used_cnt = 0;
    };

    struct s_texture {
        // @idea: OPTIONALLY keep pixel data? nullptr otherwise?
        s_v2_s32 size;
        bgfx::TextureHandle bgfx_hdl = BGFX_INVALID_HANDLE;

        bool LoadFromRaw(const c_string_view file_path, c_gfx_resource_lifetime& gfx_res_lifetime);
        void LoadFromPacked(c_file_reader& fr, c_gfx_resource_lifetime& gfx_res_lifetime); // @todo: Just ask for the buffer for this specific packing. You can easily pull that from a file, store temporarily in a buffer, then pass into here. Much more flexible than a file explicitly.
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
    };

    enum class ec_renderer_state {
        not_initted,
        initted,
        rendering
    };

    struct s_renderer_core {
        ec_renderer_state state = ec_renderer_state::not_initted;

        c_gfx_resource_lifetime res_lifetime;

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

        static bool Init(c_mem_arena& mem_arena, c_mem_arena& temp_mem_arena);
        static void Shutdown();

        static c_gfx_resource_lifetime& GFXResourceLifetime() {
            assert(sm_core.state == ec_renderer_state::initted);
            return sm_core.res_lifetime;
        }

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
        static void Draw(const s_texture& tex, const s_v2 pos, const s_v2 size, const s_rect_s32 src_rect = {}, const s_v2 origin = origins::g_origin_top_left, const float rot = 0.0f, const s_v4 blend = colors::g_white);
        //[[nodiscard]] bool DrawStr(const c_string_view str, const int font_index, const s_font_group& fonts, const s_v2 pos, const s_v2 alignment, const s_v4 color, c_mem_arena& temp_mem_arena);

    private:
        static inline s_renderer_core sm_core;

        static void Flush();
    };
}
