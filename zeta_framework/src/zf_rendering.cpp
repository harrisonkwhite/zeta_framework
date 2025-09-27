#include "zf_rendering.h"

#include <bx/math.h>
#include "bgfx/bgfx.h"
#include "zf_window.h"

namespace zf {
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
        t_u32 abgr;
    };

    static void BuildQuadVertexLayout(bgfx::VertexLayout& layout) {
        layout.begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    };

    void BuildQuadRenderable(s_renderable& renderable) {
        static const s_static_array<s_quad_vertex, 4> verts = {{
            {100.0f, 100.0f, 0xff0000ff},
            {200.0f, 100.0f, 0xff00ff00},
            {100.0f, 200.0f, 0xffff0000},
            {200.0f, 200.0f, 0xffffffff}
        }};

        bgfx::VertexLayout vert_layout;
        BuildQuadVertexLayout(vert_layout);

        renderable.vbh = bgfx::createVertexBuffer(
            bgfx::makeRef(verts.buf_raw, sizeof(verts)),
            vert_layout
        );

        static const s_static_array<t_u16, 6> indices = {{
            0, 1, 2,
            1, 3, 2
        }};

        renderable.ibh = bgfx::createIndexBuffer(
            bgfx::makeRef(indices.buf_raw, sizeof(indices))
        );
    }

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

        //
        //
        //
        sm_prog = LoadProgFromShaderFiles("quad_vs.bin", "quad_fs.bin", temp_mem_arena);

        if (!bgfx::isValid(sm_prog)) {
            bgfx::shutdown();
            return false;
        }

        BuildQuadRenderable(sm_quad_renderable);

        return true;
    }

    void c_renderer::Clean() {
        bgfx::destroy(sm_quad_renderable.ibh);
        bgfx::destroy(sm_quad_renderable.vbh);
        bgfx::destroy(sm_prog);
        bgfx::shutdown();
    }

    void c_renderer::Render() {
        sm_size = c_window::GetFramebufferSize(); // @todo: Perform resize on window resize callback.

        s_static_array<float, 16> ortho_mat;
        bx::mtxOrtho(ortho_mat.buf_raw, 0.0f, float(sm_size.x), float(sm_size.y), 0.0f, 0.0f, 100.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);

        s_static_array<float, 16> view_mat;
        bx::mtxIdentity(view_mat.buf_raw);

        bgfx::setViewTransform(0, view_mat.buf_raw, ortho_mat.buf_raw);
        bgfx::setViewRect(0, 0, 0, uint16_t(sm_size.x), uint16_t(sm_size.y));
        bgfx::touch(0);

        bgfx::setVertexBuffer(0, sm_quad_renderable.vbh);
        bgfx::setIndexBuffer(sm_quad_renderable.ibh);

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, sm_prog);
        bgfx::frame();
    }
}
