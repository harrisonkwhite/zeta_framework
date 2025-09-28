#include "zf_rendering.h"

#include <bx/math.h>
#include "bgfx/bgfx.h"
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

    static bgfx::ProgramHandle LoadProgFromShaderFiles(const c_string_view vs_file_path, const c_string_view fs_file_path, c_mem_arena& temp_mem_arena) {
        const auto vs = LoadShaderFromFile(vs_file_path.Raw(), temp_mem_arena);

        if (!bgfx::isValid(vs)) {
            return BGFX_INVALID_HANDLE;
        }

        const auto fs = LoadShaderFromFile(fs_file_path.Raw(), temp_mem_arena);

        if (!bgfx::isValid(fs)) {
            bgfx::destroy(vs);
            return BGFX_INVALID_HANDLE;
        }

        return bgfx::createProgram(vs, fs, true);
    }

    static bgfx::DynamicVertexBufferHandle BuildQuadBatchVertBuf() {
        const bgfx::VertexLayout layout = s_batch_vert::BuildLayout();
        return bgfx::createDynamicVertexBuffer(g_batch_slot_vert_cnt * g_batch_slot_cnt, layout);
    }

    static bgfx::IndexBufferHandle BuildQuadBatchIndexBuf() {
        const int index_cnt = g_batch_slot_elem_cnt * g_batch_slot_cnt;

        const auto indices_mem = bgfx::alloc(sizeof(t_u16) * index_cnt);

        if (!indices_mem) {
            ZF_LOG_ERROR("Failed to allocate memory for quad batch indices!");
            return BGFX_INVALID_HANDLE;
        }

        const c_array<t_u16> indices = {reinterpret_cast<t_u16*>(indices_mem->data), index_cnt};

        for (int i = 0; i < g_batch_slot_cnt; i++) {
            indices[(i * 6) + 0] = (i * 4) + 0;
            indices[(i * 6) + 1] = (i * 4) + 1;
            indices[(i * 6) + 2] = (i * 4) + 2;
            indices[(i * 6) + 3] = (i * 4) + 2;
            indices[(i * 6) + 4] = (i * 4) + 3;
            indices[(i * 6) + 5] = (i * 4) + 0;
        }

        return bgfx::createIndexBuffer(indices_mem);
    }

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

        Clear(colors::g_cyan);

        sm_quad_batch_prog = LoadProgFromShaderFiles("quad_vs.bin", "quad_fs.bin", temp_mem_arena);

        if (!bgfx::isValid(sm_quad_batch_prog)) {
            bgfx::shutdown();
            return false;
        }

        sm_quad_batch_vb = BuildQuadBatchVertBuf();

        if (!bgfx::isValid(sm_quad_batch_vb)) {
            bgfx::destroy(sm_quad_batch_prog);
            bgfx::shutdown();
            return false;
        }

        sm_quad_batch_eb = BuildQuadBatchIndexBuf();

        if (!bgfx::isValid(sm_quad_batch_eb)) {
            bgfx::destroy(sm_quad_batch_vb);
            bgfx::destroy(sm_quad_batch_prog);
            bgfx::shutdown();
            return false;
        }

        return true;
    }

    void c_renderer::Shutdown() {
        bgfx::destroy(sm_quad_batch_eb);
        bgfx::destroy(sm_quad_batch_vb);
        bgfx::destroy(sm_quad_batch_prog);
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
        if (sm_quad_batch_slots_used_cnt == g_batch_slot_cnt) {
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

        const uint32_t vertCount = g_batch_slot_vert_cnt * sm_quad_batch_slots_used_cnt;

        const auto mem = bgfx::makeRef(sm_quad_batch_slots.buf_raw, sizeof(s_quad_batch_vert) * vertCount);
        bgfx::update(sm_quad_batch_vb, 0, mem);

        bgfx::setVertexBuffer(0, sm_quad_batch_vb, 0, vertCount);
        bgfx::setIndexBuffer(sm_quad_batch_eb);

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, sm_quad_batch_prog);

        sm_quad_batch_slots_used_cnt = 0;
    }
}
