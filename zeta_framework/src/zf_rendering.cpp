#include "zf_rendering.h"

#include <bx/math.h>
#include "bgfx/bgfx.h"
#include "zf_window.h"

namespace zf {
    bool c_renderer::Init(c_mem_arena& temp_mem_arena) {
        const s_v2_s32 fb_size = c_window::GetFramebufferSize();

        bgfx::Init init;
        init.type = bgfx::RendererType::Count;

        init.resolution.reset = BGFX_RESET_VSYNC;
        init.resolution.width = static_cast<uint32_t>(fb_size.x);
        init.resolution.height = static_cast<uint32_t>(fb_size.y);

        init.platformData.nwh = c_window::GetNativeWindowHandle();
        init.platformData.ndt = c_window::GetNativeDisplayHandle();
        init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(init)) {
            ZF_LOG_ERROR("Failed to initialise bgfx!");
            return false;
        }

        sm_quad_batch_vb_bgfx_hdl = BuildQuadBatchVertBuf();

        if (!bgfx::isValid(sm_quad_batch_vb_bgfx_hdl)) {
            bgfx::shutdown();
            return false;
        }

        sm_quad_batch_eb_bgfx_hdl = BuildQuadBatchIndexBuf();

        if (!bgfx::isValid(sm_quad_batch_eb_bgfx_hdl)) {
            bgfx::shutdown();
            return false;
        }

        Clear(colors::g_cyan);

        return true;
    }

    void c_renderer::Shutdown() {
        // @todo: Clean up.
        bgfx::shutdown();
    }

    void c_renderer::CompleteFrame() {
        if (sm_quad_batch_slots_used_cnt > 0) {
            Flush();
        }

        const s_v2_s32 fb_size = c_window::GetFramebufferSize();

        s_static_array<float, 16> ortho_mat;
        bx::mtxOrtho(ortho_mat.buf_raw, 0.0f, (float)fb_size.x, (float)fb_size.y, 0.0f, 0.0f, 1.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);

        s_static_array<float, 16> view_mat;
        bx::mtxIdentity(view_mat.buf_raw);

        bgfx::setViewTransform(0, view_mat.buf_raw, ortho_mat.buf_raw);
        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(fb_size.x), static_cast<uint16_t>(fb_size.y));

        bgfx::touch(0);

        bgfx::frame();
    }

    void c_renderer::Clear(const s_v4 col) {
        const s_int_rgba int_rgba = ToIntRGBA(col);
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR, 0x303030ff);
    }

    void c_renderer::Draw(const s_v2 pos, const s_v2 size, const s_v2 origin, const float rot, const s_v4 blend) {
        if (sm_quad_batch_slots_used_cnt == g_quad_batch_slot_cnt) {
            Flush();
            Draw(pos, size, origin, rot, blend);
            return;
        }

        auto& slot = sm_quad_batch_slots[sm_quad_batch_slots_used_cnt];

        const s_static_array<s_v2, 4> vert_coords = {{
            {0.0f - origin.x, 0.0f - origin.y},
            {1.0f - origin.x, 0.0f - origin.y},
            {1.0f - origin.x, 1.0f - origin.y},
            {0.0f - origin.x, 1.0f - origin.y}
        }};

        for (int i = 0; i < slot.Len(); i++) {
            slot[i] = {
                .vert_coord = vert_coords[i],
                .pos = pos,
                .size = size,
                .rot = rot,
                .blend = blend
            };
        }

        sm_quad_batch_slots_used_cnt++;
    }

    void c_renderer::Flush() {
        if (sm_quad_batch_slots_used_cnt == 0) {
            return;
        }

        const uint32_t vert_cnt = g_quad_batch_slot_vert_cnt * sm_quad_batch_slots_used_cnt;

        const auto mem = bgfx::makeRef(sm_quad_batch_slots.buf_raw, sizeof(s_quad_batch_vert) * vert_cnt);
        bgfx::update(sm_quad_batch_vb_bgfx_hdl, 0, mem);

        bgfx::setVertexBuffer(0, sm_quad_batch_vb_bgfx_hdl, 0, vert_cnt);
        bgfx::setIndexBuffer(sm_quad_batch_eb_bgfx_hdl);

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, sm_quad_batch_prog_bgfx_hdl);

        sm_quad_batch_slots_used_cnt = 0;
    }
}
