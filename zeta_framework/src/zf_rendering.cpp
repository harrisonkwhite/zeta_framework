#include "zf_rendering.h"
#include "bgfx/bgfx.h"

namespace zf {
    static bgfx::ProgramHandle CreateShaderProg(const c_array<const t_u8> vs_bin, const c_array<const t_u8> fs_bin) {
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

    static bgfx::ProgramHandle LoadShaderProgFromRawFiles(const c_string_view vs_bin_file_path, const c_string_view fs_bin_file_path, c_mem_arena& temp_mem_arena) {
        const auto vs_bin = LoadFileContents(vs_bin_file_path, temp_mem_arena);

        if (vs_bin.IsEmpty()) {
            return BGFX_INVALID_HANDLE;
        }

        const auto fs_bin = LoadFileContents(fs_bin_file_path, temp_mem_arena);

        if (fs_bin.IsEmpty()) {
            return BGFX_INVALID_HANDLE;
        }

        return CreateShaderProg(vs_bin, fs_bin);
    }

    static bgfx::VertexLayout GenQuadBatchVertLayout() {
        bgfx::VertexLayout layout;

        layout.begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord2, 1, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord3, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .end();

        return layout;
    }

    static bgfx::DynamicVertexBufferHandle GenQuadBatchVertBuf() {
        const bgfx::VertexLayout layout = GenQuadBatchVertLayout();
        return bgfx::createDynamicVertexBuffer(g_quad_batch_slot_vert_cnt * g_quad_batch_slot_cnt, layout);
    }

    static bgfx::IndexBufferHandle GenQuadBatchIndexBuf() {
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

    static bool InitQuadBatchRenderable(s_renderable& renderable, c_gfx_resource_lifetime& gfx_res_lifetime, c_mem_arena& temp_mem_arena) {
        renderable.prog_bgfx_hdl = LoadShaderProgFromRawFiles("zeta_framework/shaders/quad/vs.bin", "zeta_framework/shaders/quad/fs.bin", temp_mem_arena);

        if (!bgfx::isValid(renderable.prog_bgfx_hdl)) {
            return false;
        }

        gfx_res_lifetime.AddBGFXResource(renderable.prog_bgfx_hdl);

        renderable.tex_uni_bgfx_hdl = bgfx::createUniform("u_tex", bgfx::UniformType::Sampler);

        if (!bgfx::isValid(renderable.tex_uni_bgfx_hdl)) {
            return false;
        }

        gfx_res_lifetime.AddBGFXResource(renderable.tex_uni_bgfx_hdl);

        renderable.dvb_bgfx_hdl = GenQuadBatchVertBuf();

        if (!bgfx::isValid(renderable.dvb_bgfx_hdl)) {
            return false;
        }

        gfx_res_lifetime.AddBGFXResource(renderable.dvb_bgfx_hdl);

        renderable.index_bgfx_hdl = GenQuadBatchIndexBuf();

        if (!bgfx::isValid(renderable.index_bgfx_hdl)) {
            return false;
        }

        gfx_res_lifetime.AddBGFXResource(renderable.index_bgfx_hdl);

        return true;
    }

    bool c_renderer::Init(c_mem_arena& mem_arena, c_mem_arena& temp_mem_arena) {
        assert(sm_core.state == ec_renderer_state::not_initted);

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

        if (!sm_core.res_lifetime.Init(mem_arena)) {
            ZF_LOG_ERROR("Failed to initialise the renderer GFX resource lifetime!");
            bgfx::shutdown();
            return false;
        }

        if (!InitQuadBatchRenderable(sm_core.quad_batch_renderable, sm_core.res_lifetime, temp_mem_arena)) {
            ZF_LOG_ERROR("Failed to initialise the quad batch renderable!");
            sm_core.res_lifetime.Clean();
            bgfx::shutdown();
            return false;
        }

        for (int i = 0; i < g_view_limit; i++) {
            sm_core.view_mats[i] = s_matrix_4x4::Identity();
        }

        sm_core.state = ec_renderer_state::initted;

        return true;
    }

    void c_renderer::Shutdown() {
        assert(sm_core.state == ec_renderer_state::initted);

        sm_core.res_lifetime.Clean();
        bgfx::shutdown();

        sm_core = {};
    }

    void c_renderer::BeginFrame() {
        assert(sm_core.state == ec_renderer_state::initted);
        sm_core.state = ec_renderer_state::rendering;
        SetView(0);
    }

    void c_renderer::EndFrame() {
        assert(sm_core.state == ec_renderer_state::rendering);

        if (sm_core.quad_batch_slots_used_cnt > 0) {
            Flush();
        }

        bgfx::frame();

        sm_core.state = ec_renderer_state::initted;
    }

    void c_renderer::SetView(const t_s32 view_index, const s_v4 clear_col) {
        assert(sm_core.state == ec_renderer_state::rendering);
        assert(view_index >= 0 && view_index < g_view_limit);

        if (sm_core.quad_batch_slots_used_cnt > 0) {
            Flush();
        }

        sm_core.active_view_index = view_index;

        bgfx::setViewMode(view_index, bgfx::ViewMode::Sequential);

        const s_v2_s32 fb_size = c_window::GetFramebufferSize();

        const auto& view_mat = sm_core.view_mats[view_index];
        const auto proj_mat = s_matrix_4x4::Orthographic(0.0f, fb_size.x, fb_size.y, 0.0f, -1.0f, 1.0f); // @todo: Cache this.
        bgfx::setViewTransform(view_index, view_mat.elems.buf_raw, proj_mat.elems.buf_raw);

        bgfx::setViewRect(view_index, 0, 0, static_cast<uint16_t>(fb_size.x), static_cast<uint16_t>(fb_size.y));

        bgfx::touch(view_index);

        Clear(clear_col);
    }

    void c_renderer::Clear(const s_v4 col) {
        assert(sm_core.state == ec_renderer_state::rendering);

        const s_int_rgba int_rgba = ToIntRGBA(col); // @todo: Update.
        bgfx::setViewClear(sm_core.active_view_index, BGFX_CLEAR_COLOR, 0x303030ff);
    }

    void c_renderer::Draw(const int tex_index, const c_texture_group& tex_group, const s_v2 pos, const s_v2 size, const s_v2 origin, const float rot, const s_v4 blend) {
        assert(sm_core.state == ec_renderer_state::rendering);

        const auto tex_bgfx_hdl = tex_group.GetTextureBGFXHandle(tex_index);

        if (sm_core.quad_batch_slots_used_cnt == g_quad_batch_slot_cnt
            || (sm_core.quad_batch_slots_used_cnt > 0 && tex_bgfx_hdl.idx != sm_core.quad_batch_tex_bgfx_hdl.idx)) {
            Flush();
            Draw(tex_index, tex_group, pos, size, origin, rot, blend);
            return;
        }

        sm_core.quad_batch_tex_bgfx_hdl = tex_bgfx_hdl;

        auto& slot = sm_core.quad_batch_slots[sm_core.quad_batch_slots_used_cnt];

        const s_static_array<s_v2, 4> vert_coords = {{
            {0.0f - origin.x, 0.0f - origin.y},
            {1.0f - origin.x, 0.0f - origin.y},
            {1.0f - origin.x, 1.0f - origin.y},
            {0.0f - origin.x, 1.0f - origin.y}
        }};

        const s_static_array<s_v2, 4> uvs = {{
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        }};

        // @todo: src_rect
        /*const s_static_array<s_v2, 4> tex_coords = {{
            {write_info.tex_coords.left, write_info.tex_coords.top},
            {write_info.tex_coords.right, write_info.tex_coords.top},
            {write_info.tex_coords.right, write_info.tex_coords.bottom},
            {write_info.tex_coords.left, write_info.tex_coords.bottom}
        }};*/

        for (int i = 0; i < slot.Len(); i++) {
            slot[i] = {
                .vert_coord = vert_coords[i],
                .pos = pos,
                .size = size,
                .rot = rot,
                .uv = uvs[i],
                .blend = blend
            };
        }

        sm_core.quad_batch_slots_used_cnt++;
    }

    void c_renderer::Flush() {
        assert(sm_core.state == ec_renderer_state::rendering);
        assert(sm_core.quad_batch_slots_used_cnt > 0);

        // Upload vertex data to GPU.
        const uint32_t vert_cnt = g_quad_batch_slot_vert_cnt * sm_core.quad_batch_slots_used_cnt;
        const auto mem = bgfx::makeRef(sm_core.quad_batch_slots.buf_raw, sizeof(s_quad_batch_vert) * vert_cnt);
        bgfx::update(sm_core.quad_batch_renderable.dvb_bgfx_hdl, 0, mem);

        // Render the batch.
        bgfx::setVertexBuffer(0, sm_core.quad_batch_renderable.dvb_bgfx_hdl, 0, vert_cnt);
        bgfx::setIndexBuffer(sm_core.quad_batch_renderable.index_bgfx_hdl);
        bgfx::setTexture(0, sm_core.quad_batch_renderable.tex_uni_bgfx_hdl, sm_core.quad_batch_tex_bgfx_hdl);
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
        bgfx::submit(0, sm_core.quad_batch_renderable.prog_bgfx_hdl);

        // Reset to batch beginning.
        sm_core.quad_batch_slots_used_cnt = 0;
    }
}
