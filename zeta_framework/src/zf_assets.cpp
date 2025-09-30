#include "zf_assets.h"

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
}
