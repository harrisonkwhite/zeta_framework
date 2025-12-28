#include "zap.h"

#include <reproc/reproc.h>

namespace zf {
    t_b8 CompileShader(const s_str_rdonly shader_file_path, const s_str_rdonly varying_def_file_path, const t_b8 is_frag, s_mem_arena &temp_mem_arena) {
        const s_str_rdonly shader_file_path_terminated = AllocStrCloneButAddTerminator(shader_file_path, temp_mem_arena);
        const s_str_rdonly varying_def_file_path_terminated = AllocStrCloneButAddTerminator(varying_def_file_path, temp_mem_arena);

        t_i32 r = 0;

        ZF_DEFER({
            if (r < 0) {
                const auto err = ConvertCstr(reproc_strerror(r));
                LogError(s_cstr_literal("%"), err);
            }
        });

        reproc_t *const proc = reproc_new();

        if (!proc) {
            return false;
        }

        ZF_DEFER({
            reproc_destroy(proc);
        });

#if defined(ZF_PLATFORM_WINDOWS)
        const s_cstr_literal shaderc_file_path = "tools/bgfx/shaderc_windows.exe";
        const s_cstr_literal platform = "windows";
        const s_cstr_literal profile = "s_5_0";
#elif defined(ZF_PLATFORM_MACOS)
        const s_cstr_literal shaderc_file_path = "tools/bgfx/shaderc_macos";
        const s_cstr_literal platform = "osx";
        const s_cstr_literal profile = "metal";
#elif defined(ZF_PLATFORM_LINUX)
        const s_cstr_literal shaderc_file_path = "tools/bgfx/shaderc_linux";
        const s_cstr_literal platform = "linux";
        const s_cstr_literal profile = "glsl";
#endif

        const s_static_array<const char *, 15> args = {{
            shaderc_file_path.BufPtr(),
            "-f",
            shader_file_path_terminated.AsCstr(),
            "--type",
            is_frag ? "fragment" : "vertex",
            "--platform",
            platform.BufPtr(),
            "--profile",
            profile.BufPtr(),
            "--varyingdef",
            varying_def_file_path_terminated.AsCstr(),
            "-i",
            "tools/bgfx/shaderc_include",
            "--stdout",
            nullptr,
        }};

        r = reproc_start(proc, args.raw, {});

        if (r < 0) {
            return false;
        }

        r = reproc_close(proc, REPROC_STREAM_IN);

        if (r < 0) {
            return false;
        }

        s_list<t_u8> blob = {};

        while (true) {
            s_static_array<t_u8, 4096> buf = {};
            r = reproc_read(proc, REPROC_STREAM_OUT, buf.raw, ZF_SIZE_OF(buf));

            if (r < 0) {
                break;
            }

            ListAppendManyDynamic(blob, buf.ToNonstatic().Slice(0, r), temp_mem_arena);
        }

        if (r != REPROC_EPIPE) {
            return false;
        }

        r = reproc_wait(proc, REPROC_INFINITE);

        if (r < 0) {
            return false;
        }

        return true;
    }
}
