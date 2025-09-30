#include "zf_rendering.h"

#include "zf_window.h"

namespace zf {
    static bgfx::ShaderHandle LoadShaderFromFile(const c_string_view file_path, c_mem_arena& temp_mem_arena) {
        const c_array<t_u8> file_contents = LoadFileContents(file_path, temp_mem_arena, false);

        if (file_contents.IsEmpty()) {
            return BGFX_INVALID_HANDLE;
        }

        const bgfx::Memory* mem = bgfx::makeRef(file_contents.Raw(), file_contents.Len());
        return bgfx::createShader(mem);
    }

    static bgfx::ProgramHandle CreateShaderProg(const c_array<const t_u8> vs_bin, const c_array<const t_u8> fs_bin, c_mem_arena& temp_mem_arena) {
        const bgfx::Memory* vs_mem = bgfx::makeRef(vs_bin.Raw(), vs_bin.Len());
        const bgfx::ShaderHandle vs_hdl = bgfx::createShader(vs_mem);

        if (!bgfx::isValid(vs_hdl)) {
            return BGFX_INVALID_HANDLE;
        }

        const bgfx::Memory* fs_mem = bgfx::makeRef(fs_bin.Raw(), fs_bin.Len());
        const bgfx::ShaderHandle fs_hdl = bgfx::createShader(fs_mem);

        if (!bgfx::isValid(fs_hdl)) {
            return BGFX_INVALID_HANDLE;
        }

        return bgfx::createProgram(vs_hdl, fs_hdl, true);
    }

    bool c_renderer::Init(c_mem_arena& temp_mem_arena) {
        assert(sm_state == ec_renderer_state::not_initted);

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

        sm_state = ec_renderer_state::initted;

        return true;
    }

    void c_renderer::Shutdown() {
        assert(sm_state == ec_renderer_state::initted);

        sm_surface_renderable.Clean();
        sm_quad_batch_renderable.Clean();
        bgfx::frame();
        bgfx::shutdown();

        sm_state = ec_renderer_state::not_initted;
    }

    void c_renderer::BeginFrame() {
        assert(sm_state == ec_renderer_state::initted);
        sm_state = ec_renderer_state::rendering;
        SetView(0);
    }

    void c_renderer::EndFrame() {
        assert(sm_state == ec_renderer_state::rendering);

        if (sm_quad_batch_slots_used_cnt > 0) {
            Flush();
        }

        bgfx::frame();

        sm_state = ec_renderer_state::initted;
    }

    void c_renderer::SetView(const t_s32 view_index, const s_v4 clear_col) {
        assert(sm_state == ec_renderer_state::rendering);
        assert(view_index >= 0 && view_index < g_view_limit);

        if (sm_quad_batch_slots_used_cnt > 0) {
            Flush();
        }

        sm_active_view_index = view_index;

        const auto& mats = sm_view_mats[view_index];
        bgfx::setViewTransform(view_index, mats.view.elems.buf_raw, mats.proj.elems.buf_raw);

        const s_v2_s32 fb_size = c_window::GetFramebufferSize();
        bgfx::setViewRect(view_index, 0, 0, static_cast<uint16_t>(fb_size.x), static_cast<uint16_t>(fb_size.y));

        Clear(clear_col);
    }

    void c_renderer::Clear(const s_v4 col) {
        assert(sm_state == ec_renderer_state::rendering);

        const s_int_rgba int_rgba = ToIntRGBA(col); // @todo: Update.
        bgfx::setViewClear(sm_active_view_index, BGFX_CLEAR_COLOR, 0x303030ff);
    }

    void c_renderer::Draw(const s_v2 pos, const s_v2 size, const s_v2 origin, const float rot, const s_v4 blend) {
        assert(sm_state == ec_renderer_state::rendering);

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
        assert(sm_state == ec_renderer_state::rendering);
        assert(sm_quad_batch_slots_used_cnt > 0);

        // Upload vertex data to GPU.
        const uint32_t vert_cnt = g_quad_batch_slot_vert_cnt * sm_quad_batch_slots_used_cnt;
        const auto mem = bgfx::makeRef(sm_quad_batch_slots.buf_raw, sizeof(s_quad_batch_vert) * vert_cnt);
        bgfx::update(sm_quad_batch_renderable.dvb_bgfx_hdl, 0, mem);

        // Render the batch.
        bgfx::setVertexBuffer(0, sm_quad_batch_renderable.dvb_bgfx_hdl, 0, vert_cnt);
        bgfx::setIndexBuffer(sm_quad_batch_renderable.index_bgfx_hdl);
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
        bgfx::submit(0, sm_quad_batch_renderable.prog_bgfx_hdl);

        // Reset to batch beginning.
        sm_quad_batch_slots_used_cnt = 0;
    }
}
