#include "zf_gfx.h"

#include <bgfx/bgfx.h>
#include "zc_mem.h"
#include "zf_window.h"

namespace zf {
    bool c_gfx_resource_arena::Init(const s_static_array<int, eks_gfx_resource_type_cnt>& hdl_limits, c_mem_arena& mem_arena) {
        for (int i = 0; i < eks_gfx_resource_type_cnt; i++) {
            if (hdl_limits[i] == 0) {
                continue;
            }

            m_hdls[i] = PushArrayToMemArena<s_gfx_resource_hdl>(mem_arena, hdl_limits[i]);

            if (m_hdls[i].IsEmpty()) {
                return false;
            }
        }

        return true;
    }

    s_gfx_resource_hdl c_gfx_resource_arena::CreateVertBuf(const int vert_cnt) {
        //const bgfx::VertexLayout layout = GenVertLayout();
        return bgfx::createDynamicVertexBuffer(vert_cnt, layout);
    }

    namespace renderer {
        bool Init() {
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

            return true;
        }

        void Shutdown() {
            bgfx::shutdown();
        }

        void UpdateVertBuf(const s_gfx_resource_hdl hdl, const c_array<const float> verts, const int begin_index) {
            const auto mem = bgfx::makeRef(verts.Raw(), );
            bgfx::update(hdl, 0, mem);
        }

        void Clear(const int view_index, const s_v4 col) {
            const s_int_rgba int_rgba = ToIntRGBA(col); // @todo: Update.
            bgfx::setViewClear(view_index, BGFX_CLEAR_COLOR, 0x303030ff);
        }

        void Draw(const int view_index, const s_gfx_resource_hdl prog_hdl, const s_gfx_resource_hdl vert_buf_hdl, const int vert_cnt, const s_gfx_resource_hdl index_buf_hdl) {
            bgfx::setVertexBuffer(0, vert_buf_hdl.bgfx, 0, vert_cnt);
            bgfx::setIndexBuffer(index_buf_hdl.bgfx);

            bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

            bgfx::submit(0, prog_hdl.bgfx);
        }

        void SetViewTransform(const int view_index, const s_matrix_4x4& view_mat, const s_matrix_4x4& proj_mat) {
            bgfx::setViewTransform(0, view_mat.elems.buf_raw, proj_mat.elems.buf_raw);

            // @todo: Remove the below?
            const s_v2_s32 fb_size = c_window::GetFramebufferSize();
            bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(fb_size.x), static_cast<uint16_t>(fb_size.y));
        }

        void CompleteFrame() {
            bgfx::frame();
        }
    }

#if 0
    bool Init(c_mem_arena& temp_mem_arena) {
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

    void Shutdown() {
        // @todo: Clean up.
        bgfx::shutdown();
    }

    void CompleteFrame() {
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

    void Clear(const s_v4 col) {
        const s_int_rgba int_rgba = ToIntRGBA(col);
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR, 0x303030ff);
    }

    void Draw(const s_v2 pos, const s_v2 size, const s_v2 origin, const float rot, const s_v4 blend) {
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

    void Flush() {
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
#endif
}
