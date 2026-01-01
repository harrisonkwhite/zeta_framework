#include "zap.h"

#include <reproc/reproc.h>

namespace zf {
    t_b8 CompileShader(const s_str_rdonly shader_file_path, const s_str_rdonly varying_def_file_path, const t_b8 is_frag, s_arena *const bin_arena, s_arena *const temp_arena, s_array_mut<t_u8> *const o_bin) {
        const s_str_rdonly shader_file_path_terminated = AllocStrCloneButAddTerminator(shader_file_path, temp_arena);
        const s_str_rdonly varying_def_file_path_terminated = AllocStrCloneButAddTerminator(varying_def_file_path, temp_arena);

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
        const s_cstr_literal shaderc_file_path_rel = "tools/bgfx/shaderc_windows.exe";
        const s_cstr_literal platform = "windows";
        const s_cstr_literal profile = "s_5_0";
#elif defined(ZF_PLATFORM_MACOS)
    #error "Platform support not complete!" // @todo
        const s_cstr_literal shaderc_file_path_rel = "tools/bgfx/shaderc_macos";
        const s_cstr_literal platform = "osx";
        const s_cstr_literal profile = "metal";
#elif defined(ZF_PLATFORM_LINUX)
    #error "Platform support not complete!" // @todo
        const s_cstr_literal shaderc_file_path_rel = "tools/bgfx/shaderc_linux";
        const s_cstr_literal platform = "linux";
        const s_cstr_literal profile = "glsl";
#endif

        const auto exe_dir = LoadExecutableDirectory(temp_arena);
        ZF_ASSERT(exe_dir.bytes[exe_dir.bytes.len - 1] == '/' || exe_dir.bytes[exe_dir.bytes.len - 1] == '\\'); // Assuming this.

        const s_str shaderc_file_path = {PushArray<t_u8>(temp_arena, exe_dir.bytes.len + shaderc_file_path_rel.buf.len + 1)};
        CopyAll(exe_dir.bytes, shaderc_file_path.bytes);
        CopyAll(shaderc_file_path_rel.buf.AsByteArray(), SliceFrom(shaderc_file_path.bytes, exe_dir.bytes.len));

        const s_cstr_literal shaderc_include_dir_rel = "tools/bgfx/shaderc_include";
        const s_str shaderc_include_dir = {PushArray<t_u8>(temp_arena, exe_dir.bytes.len + shaderc_include_dir_rel.buf.len + 1)};
        CopyAll(exe_dir.bytes, shaderc_include_dir.bytes);
        CopyAll(shaderc_include_dir_rel.buf.AsByteArray(), SliceFrom(shaderc_include_dir.bytes, exe_dir.bytes.len));

        const s_static_array<const char *, 15> args = {{
            AsCstr(shaderc_file_path),
            "-f",
            AsCstr(shader_file_path_terminated),
            "--type",
            is_frag ? "fragment" : "vertex",
            "--platform",
            platform.buf.raw,
            "--profile",
            profile.buf.raw,
            "--varyingdef",
            AsCstr(varying_def_file_path_terminated),
            "-i",
            AsCstr(shaderc_include_dir),
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

        s_list_mut<t_u8> bin_list = {};

        while (true) {
            s_static_array<t_u8, 4096> buf;
            r = reproc_read(proc, REPROC_STREAM_OUT, buf.raw, ZF_SIZE_OF(buf));

            if (r < 0) {
                break;
            }

            AppendManyDynamic(&bin_list, Slice(buf.AsNonstatic(), 0, r), bin_arena);
        }

        if (r != REPROC_EPIPE) {
            return false;
        }

        r = reproc_wait(proc, REPROC_INFINITE);

        if (r < 0) {
            return false;
        }

        if (r > 0) {
            auto std_err = StdError();
            PrintFormat(&std_err, s_cstr_literal("==================== BGFX SHADERC ERROR ====================\n%============================================================\n"), s_str_rdonly(bin_list.AsArray()));
            return false;
        }

        *o_bin = bin_list.AsArray();

        return true;
    }
}
