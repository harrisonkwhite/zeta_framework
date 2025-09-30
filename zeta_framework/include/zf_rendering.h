#pragma once

#include <bgfx/bgfx.h>
#include <zc.h>
#include "zf_assets.h"

namespace zf {
    constexpr t_s32 g_quad_batch_slot_cnt = 8192;
    constexpr t_s32 g_quad_batch_slot_vert_cnt = 4;
    constexpr t_s32 g_quad_batch_slot_elem_cnt = 6;
    static_assert(g_quad_batch_slot_elem_cnt * g_quad_batch_slot_cnt <= USHRT_MAX, "Quad batch slot count is too high!");

    enum class ec_gfx_resource_type {
        invalid,
        vert_buf,
        index_buf,
        shader_prog,
        texture
    };

    struct s_gfx_resource_hdl {
        t_u16 raw_bgfx;
        ec_gfx_resource_type type;
    };

    class c_gfx_resource_arena {
    public:
        s_gfx_resource_hdl CreateVertBuf(const int vert_cnt) {
        }

        void Clean() {
        }

    private:
        c_array_list<s_gfx_resource_hdl> m_hdls;
    };

    class c_quad_batch_bgfx_resources {
    public:
        bool Init() {
            vb_hdl = GenVertBuf();

            if (!bgfx::isValid(vb_hdl)) {
                return false;
            }

            ib_hdl = GenIndexBuf();

            if (!bgfx::isValid(vb_hdl)) {
                return false;
            }

            return true;
        }

        void Clean() {
            bgfx::destroy(prog_hdl);
            prog_hdl = BGFX_INVALID_HANDLE;

            bgfx::destroy(ib_hdl);
            ib_hdl = BGFX_INVALID_HANDLE;

            bgfx::destroy(vb_hdl);
            vb_hdl = BGFX_INVALID_HANDLE;
        }

    private:
        bgfx::ProgramHandle prog_hdl = BGFX_INVALID_HANDLE;
        bgfx::DynamicVertexBufferHandle vb_hdl = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle ib_hdl = BGFX_INVALID_HANDLE;

        static bgfx::VertexLayout GenVertLayout() {
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

        static bgfx::DynamicVertexBufferHandle GenVertBuf() {
            const bgfx::VertexLayout layout = GenVertLayout();
            return bgfx::createDynamicVertexBuffer(g_quad_batch_slot_vert_cnt * g_quad_batch_slot_cnt, layout);
        }

        static bgfx::IndexBufferHandle GenIndexBuf() {
            const int index_cnt = g_quad_batch_slot_elem_cnt * g_quad_batch_slot_cnt;

            const auto indices_mem = bgfx::alloc(sizeof(t_u16) * index_cnt);

            if (!indices_mem) {
                ZF_LOG_ERROR("Failed to allocate memory for quad batch indices!");
                return BGFX_INVALID_HANDLE;
            }

            const c_array<t_u16> indices = {reinterpret_cast<t_u16*>(indices_mem->data), index_cnt};

            for (int i = 0; i < g_quad_batch_slot_cnt; i++) {
                indices[(i * 6) + 0] = (i * 4) + 0;
                indices[(i * 6) + 1] = (i * 4) + 1;
                indices[(i * 6) + 2] = (i * 4) + 2;
                indices[(i * 6) + 3] = (i * 4) + 2;
                indices[(i * 6) + 4] = (i * 4) + 3;
                indices[(i * 6) + 5] = (i * 4) + 0;
            }

            return bgfx::createIndexBuffer(indices_mem);
        }
    };

    struct s_quad_batch_vert {
        s_v2 vert_coord;
        s_v2 pos;
        s_v2 size;
        float rot = 0.0f;
        s_v4 blend;
    };

    using t_quad_batch_slot = s_static_array<s_quad_batch_vert, g_quad_batch_slot_vert_cnt>;

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
        static inline c_quad_batch_bgfx_resources sm_quad_batch_bgfx_resources;

        static inline s_static_array<t_quad_batch_slot, g_quad_batch_slot_cnt> sm_quad_batch_slots;
        static inline t_s32 sm_quad_batch_slots_used_cnt = 0;
    };
}
