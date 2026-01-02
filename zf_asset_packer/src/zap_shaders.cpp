#include "zap.h"

#include <reproc/reproc.h>

namespace zf {
    t_b8 CompileShader(const s_str_rdonly shader_file_path, const s_str_rdonly varying_def_file_path, const t_b8 is_frag, s_arena *const bin_arena, s_arena *const temp_arena, s_array_mut<t_u8> *const o_bin) {
        const s_str_rdonly shader_file_path_terminated = CloneStrButAddTerminator(shader_file_path, temp_arena);
        const s_str_rdonly varying_def_file_path_terminated = CloneStrButAddTerminator(varying_def_file_path, temp_arena);

        t_i32 r = 0;

        ZF_DEFER({
            if (r < 0) {
                const auto err = ConvertCstr(reproc_strerror(r));
                LogError(ZF_STR_LITERAL("%"), err);
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
        const s_str_rdonly shaderc_file_path_rel = ZF_STR_LITERAL("tools/bgfx/shaderc_windows.exe");
        const char platform_cstr[] = "windows";
        const char profile_cstr[] = "s_5_0";
#elif defined(ZF_PLATFORM_MACOS)
    #error "Platform support not complete!" // @todo
        const char shaderc_file_path_rel_cstr[] = "tools/bgfx/shaderc_macos";
        const char platform_cstr[] = "osx";
        const char profile_cstr[] = "metal";
#elif defined(ZF_PLATFORM_LINUX)
    #error "Platform support not complete!" // @todo
        const char shaderc_file_path_rel_cstr[] = "tools/bgfx/shaderc_linux";
        const char platform_cstr[] = "linux";
        const char profile_cstr[] = "glsl";
#endif

        const s_str_rdonly exe_dir = LoadExecutableDirectory(temp_arena);
        ZF_ASSERT(exe_dir.bytes[exe_dir.bytes.len - 1] == '/' || exe_dir.bytes[exe_dir.bytes.len - 1] == '\\'); // Assuming this.

        const auto shaderc_file_path_terminated = s_str_mut(PushArray<t_u8>(temp_arena, exe_dir.bytes.len + shaderc_file_path_rel.bytes.len + 1));
        c_stream shaderc_file_path_terminated_byte_stream = {shaderc_file_path_terminated.bytes, ek_stream_mode_write};
        PrintFormat(&shaderc_file_path_terminated_byte_stream, ZF_STR_LITERAL("%%\0"), exe_dir, shaderc_file_path_rel);
        ZF_ASSERT(AreBytesTerminatedOnlyAtEnd(shaderc_file_path_terminated.bytes));

        const s_str_rdonly shaderc_include_dir_rel = ZF_STR_LITERAL("tools/bgfx/shaderc_include");
        const auto shaderc_include_dir_terminated = s_str_mut(PushArray<t_u8>(temp_arena, exe_dir.bytes.len + shaderc_include_dir_rel.bytes.len + 1));
        c_stream shaderc_include_dir_terminated_byte_stream = {shaderc_include_dir_terminated.bytes, ek_stream_mode_write};
        PrintFormat(&shaderc_include_dir_terminated_byte_stream, ZF_STR_LITERAL("%%\0"), exe_dir, shaderc_include_dir_rel);
        ZF_ASSERT(AreBytesTerminatedOnlyAtEnd(shaderc_include_dir_terminated.bytes));

        const s_static_array<const char *, 15> args = {{
            AsCstr(shaderc_file_path_terminated),
            "-f",
            AsCstr(shader_file_path_terminated),
            "--type",
            is_frag ? "fragment" : "vertex",
            "--platform",
            platform_cstr,
            "--profile",
            profile_cstr,
            "--varyingdef",
            AsCstr(varying_def_file_path_terminated),
            "-i",
            AsCstr(shaderc_include_dir_terminated),
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
            c_stream std_err = StdError();
            const auto err = s_str_rdonly(bin_list.AsArray());
            PrintFormat(&std_err, ZF_STR_LITERAL("==================== BGFX SHADERC ERROR ====================\n%============================================================\n"), err);
            return false;
        }

        *o_bin = bin_list.AsArray();

        return true;
    }
}
