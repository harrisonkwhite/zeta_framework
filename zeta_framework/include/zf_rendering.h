#pragma once

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <zc.h>

namespace zf {
    // @todo: We need a module for the generic graphics abstraction, another for the renderer.

    struct s_int_rgba {
        t_u8 r = 0;
        t_u8 g = 0;
        t_u8 b = 0;
        t_u8 a = 0;

        t_u32 RGBA() const {
            return *reinterpret_cast<const t_u32*>(this);
        }
    };

    static inline s_int_rgba ToIntRGBA(const s_v4 flt) {
        return {
            static_cast<t_u8>(roundf(flt.x * 255.0f)),
            static_cast<t_u8>(roundf(flt.y * 255.0f)),
            static_cast<t_u8>(roundf(flt.z * 255.0f)),
            static_cast<t_u8>(roundf(flt.w * 255.0f))
        };
    }

    constexpr t_s32 g_batch_slot_cnt = 8192;
    constexpr t_s32 g_batch_slot_vert_cnt = 4;
    constexpr t_s32 g_batch_slot_elem_cnt = 6;
    static_assert(g_batch_slot_elem_cnt * g_batch_slot_cnt <= USHRT_MAX, "Batch slot count is too high!");

    struct s_batch_vert {
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

    using t_batch_slot = s_static_array<s_batch_vert, g_batch_slot_vert_cnt>;

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
        static inline bgfx::ProgramHandle sm_quad_batch_ph;

        static inline bgfx::DynamicVertexBufferHandle sm_quad_batch_vbh;
        static inline bgfx::IndexBufferHandle sm_quad_batch_ibh;

        static inline s_static_array<t_batch_slot, g_batch_slot_cnt> sm_batch_slots;
        static inline t_s32 sm_batch_slots_used_cnt = 0;
    };
}
