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

        sm_quad_batch_ph = LoadProgFromShaderFiles("quad_vs.bin", "quad_fs.bin", temp_mem_arena);

        if (!bgfx::isValid(sm_quad_batch_ph)) {
            bgfx::shutdown();
            return false;
        }

        ZF_LOG("up to here");

        /*BuildQuadRenderable(sm_quad_renderable);

        sm_tex_color_uni = bgfx::createUniform("s_tex_color_uni", bgfx::UniformType::Sampler);*/

        /*const bgfx::Memory* mem = bgfx::copy(data, w * h * 4);
        sm_texture = bgfx::createTexture2D((uint16_t)w, (uint16_t)h, false, 1, bgfx::TextureFormat::RGBA8, 0, mem);*/

        return true;
    }

    void c_renderer::Shutdown() {
        //bgfx::destroy(sm_quad_batch_ibh);
        //bgfx::destroy(sm_quad_batch_vbh);
        bgfx::destroy(sm_quad_batch_ph);
        bgfx::shutdown();
    }

    void c_renderer::Clear(const s_v4 col) {
        const s_int_rgba int_rgba = ToIntRGBA(col);
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff);
    }

    void c_renderer::Draw(const s_v2 pos, const s_v2 size, const s_v2 origin, const float rot, const s_v4 blend) {
        return;

#if 0
        if (sm_batch_slots_used_cnt == g_batch_slot_cnt) {
            Flush();
            Draw(pos, size, origin, rot, blend);
            return;
        }

        auto& slot = sm_batch_slots[sm_batch_slots_used_cnt];

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

        sm_batch_slots_used_cnt++;
#endif
    }

    void c_renderer::CompleteFrame() {
        const s_v2_s32 fb_size = c_window::GetFramebufferSize();

        s_static_array<float, 16> ortho_mat;
        bx::mtxOrtho(ortho_mat.buf_raw, 0.0f, static_cast<float>(fb_size.x), static_cast<float>(fb_size.y), 0.0f, 0.0f, 100.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);

        s_static_array<float, 16> view_mat;
        bx::mtxIdentity(view_mat.buf_raw);

        bgfx::setViewTransform(0, view_mat.buf_raw, ortho_mat.buf_raw);
        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(fb_size.x), static_cast<uint16_t>(fb_size.y));
        bgfx::touch(0);

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, sm_quad_batch_ph);
        bgfx::frame();
    }

#if 0
    void c_renderer::Flush() {
        return;

        const s_v2_s32 fb_size = c_window::GetFramebufferSize();

        s_static_array<float, 16> ortho_mat;
        bx::mtxOrtho(ortho_mat.buf_raw, 0.0f, static_cast<float>(fb_size.x), static_cast<float>(fb_size.y), 0.0f, 0.0f, 100.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);

        s_static_array<float, 16> view_mat;
        bx::mtxIdentity(view_mat.buf_raw);

        bgfx::setViewTransform(0, view_mat.buf_raw, ortho_mat.buf_raw);
        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(fb_size.x), static_cast<uint16_t>(fb_size.y));
        bgfx::touch(0);

        bgfx::setVertexBuffer(0, sm_quad_batch_vbh);
        bgfx::setIndexBuffer(sm_quad_batch_ibh);

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, sm_quad_batch_ph);
        bgfx::frame();
    }
#endif
}

#if 0
    bgfx::ShaderHandle LoadShaderFromFile(const c_string_view file_path, c_mem_arena& temp_mem_arena) {
        const c_array<t_u8> file_contents = LoadFileContents(file_path, temp_mem_arena, false);

        if (file_contents.IsEmpty()) {
            return BGFX_INVALID_HANDLE;
        }

        const bgfx::Memory* mem = bgfx::makeRef(file_contents.Raw(), file_contents.Len());

        return bgfx::createShader(mem);
    }

    bgfx::ProgramHandle LoadProgFromShaderFiles(const c_string_view vs_file_path, const c_string_view fs_file_path, c_mem_arena& temp_mem_arena) {
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

    struct s_quad_vertex {
        float pos_x;
        float pos_y;
        float tex_u;
        float tex_v;
    };

    void BuildQuadRenderable(s_quad_renderable& renderable) {
        const bgfx::VertexLayout layout = s_batch_vert::BuildLayout();
        renderable.dvbh = bgfx::createDynamicVertexBuffer(g_batch_slot_vert_cnt * g_batch_slot_cnt, layout);

        static const s_static_array<t_u16, 6> indices = {{
            0, 1, 2,
            1, 3, 2
        }};

        renderable.ibh = bgfx::createIndexBuffer(
            bgfx::makeRef(indices.buf_raw, sizeof(indices))
        );
    }

    static bgfx::UniformHandle sm_tex_color_uni;
    static bgfx::TextureHandle sm_texture;

    bool c_renderer::Init(c_mem_arena& temp_mem_arena) {
        sm_size = c_window::GetFramebufferSize();

        bgfx::Init init;
        init.type = bgfx::RendererType::Count;

        init.resolution.reset = BGFX_RESET_VSYNC;
        init.resolution.width = static_cast<uint32_t>(sm_size.x);
        init.resolution.height = static_cast<uint32_t>(sm_size.y);

        init.platformData.nwh = c_window::GetNativeWindowHandle();
        init.platformData.ndt = c_window::GetNativeDisplayHandle();
        init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(init)) {
            ZF_LOG_ERROR("Failed to initialise bgfx!");
            return false;
        }

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

        sm_prog = LoadProgFromShaderFiles("quad_vs.bin", "quad_fs.bin", temp_mem_arena);

        if (!bgfx::isValid(sm_prog)) {
            bgfx::shutdown();
            return false;
        }

        BuildQuadRenderable(sm_quad_renderable);

        sm_tex_color_uni = bgfx::createUniform("s_tex_color_uni", bgfx::UniformType::Sampler);

        /*const bgfx::Memory* mem = bgfx::copy(data, w * h * 4);
        sm_texture = bgfx::createTexture2D((uint16_t)w, (uint16_t)h, false, 1, bgfx::TextureFormat::RGBA8, 0, mem);*/

        return true;
    }

    void c_renderer::Shutdown() {
        bgfx::destroy(sm_quad_renderable.ibh);
        bgfx::destroy(sm_quad_renderable.vbh);
        bgfx::destroy(sm_prog);
        bgfx::destroy(sm_tex_color_uni);
        bgfx::destroy(sm_texture);
        bgfx::shutdown();
    }

    void c_renderer::Flush() {
        sm_size = c_window::GetFramebufferSize();

        s_static_array<float, 16> ortho_mat;
        bx::mtxOrtho(ortho_mat.buf_raw, 0.0f, float(sm_size.x), float(sm_size.y), 0.0f, 0.0f, 100.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);

        s_static_array<float, 16> view_mat;
        bx::mtxIdentity(view_mat.buf_raw);

        bgfx::setViewTransform(0, view_mat.buf_raw, ortho_mat.buf_raw);
        bgfx::setViewRect(0, 0, 0, uint16_t(sm_size.x), uint16_t(sm_size.y));
        bgfx::touch(0);

        bgfx::setVertexBuffer(0, sm_quad_renderable.vbh);
        bgfx::setIndexBuffer(sm_quad_renderable.ibh);

        bgfx::setTexture(0, sm_tex_color_uni, sm_texture);

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, sm_prog);
        bgfx::frame();
    }
#endif
