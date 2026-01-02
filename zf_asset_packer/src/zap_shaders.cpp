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
                LogError("%", err);
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
        constexpr char shaderc_file_path_rel_cstr[] = "tools/bgfx/shaderc_windows.exe";
        constexpr char platform_cstr[] = "windows";
        constexpr char profile_cstr[] = "s_5_0";
#elif defined(ZF_PLATFORM_MACOS)
    #error "Platform support not complete!" // @todo
        constexpr char shaderc_file_path_rel_cstr[] = "tools/bgfx/shaderc_macos";
        constexpr char platform_cstr[] = "osx";
        constexpr char profile_cstr[] = "metal";
#elif defined(ZF_PLATFORM_LINUX)
    #error "Platform support not complete!" // @todo
        constexpr char shaderc_file_path_rel_cstr[] = "tools/bgfx/shaderc_linux";
        constexpr char platform_cstr[] = "linux";
        constexpr char profile_cstr[] = "glsl";
#endif

        const s_str_rdonly exe_dir = LoadExecutableDirectory(temp_arena);
        ZF_ASSERT(exe_dir.bytes[exe_dir.bytes.len - 1] == '/' || exe_dir.bytes[exe_dir.bytes.len - 1] == '\\'); // Assuming this.

        const s_str_mut shaderc_file_path_terminated = {PushArray<char>(temp_arena, exe_dir.bytes.len + ZF_SIZE_OF(shaderc_file_path_rel_cstr))};
        CopyAll(exe_dir.bytes, shaderc_file_path_terminated.bytes);
        CopyAll(s_array_rdonly<char>{shaderc_file_path_rel_cstr, ZF_SIZE_OF(shaderc_file_path_rel_cstr)}, SliceFrom(shaderc_file_path_terminated.bytes, exe_dir.bytes.len));
        ZF_ASSERT(!shaderc_file_path_terminated.bytes[shaderc_file_path_terminated.bytes.len - 1]);

        const char shaderc_include_dir_rel_cstr[] = "tools/bgfx/shaderc_include";
        const s_str_mut shaderc_include_dir_terminated = {PushArray<char>(temp_arena, exe_dir.bytes.len + ZF_SIZE_OF(shaderc_include_dir_rel_cstr))};
        CopyAll(exe_dir.bytes, shaderc_include_dir_terminated.bytes);
        CopyAll(s_array_rdonly<char>{shaderc_include_dir_rel_cstr, ZF_SIZE_OF(shaderc_include_dir_rel_cstr)}, SliceFrom(shaderc_include_dir_terminated.bytes, exe_dir.bytes.len));
        ZF_ASSERT(!shaderc_include_dir_terminated.bytes[shaderc_include_dir_terminated.bytes.len - 1]);

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
            const s_str_rdonly err = {{reinterpret_cast<const char *>(bin_list.backing_arr.raw), bin_list.len}};
            PrintFormat(&std_err, "==================== BGFX SHADERC ERROR ====================\n%============================================================\n", err);
            return false;
        }

        *o_bin = bin_list.AsArray();

        return true;
    }
}
