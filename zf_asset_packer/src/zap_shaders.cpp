#include "zap.h"

#include <reproc/reproc.h>

namespace zf {
    t_b8 compile_shader(const strs::StrRdonly shader_file_path, const strs::StrRdonly varying_def_file_path, const t_b8 is_frag, t_arena *const bin_arena, t_arena *const temp_arena, t_array_mut<t_u8> *const o_bin) {
        const strs::StrRdonly shader_file_path_terminated = clone_str_but_add_terminator(shader_file_path, temp_arena);
        const strs::StrRdonly varying_def_file_path_terminated = clone_str_but_add_terminator(varying_def_file_path, temp_arena);

        t_i32 r = 0;

        ZF_DEFER({
            if (r < 0) {
                const auto err = strs::convert_cstr(reproc_strerror(r));
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
        const strs::StrRdonly shaderc_file_path_rel = ZF_STR_LITERAL("tools/bgfx/shaderc_windows.exe");
        const char platform_cstr[] = "windows";
        const char profile_cstr[] = "s_5_0";
#elif defined(ZF_PLATFORM_MACOS)
    #error "Platform support not complete!" // @todo
        const strs::StrRdonly shaderc_file_path_rel = ZF_STR_LITERAL("tools/bgfx/shaderc_macos");
        const char platform_cstr[] = "osx";
        const char profile_cstr[] = "metal";
#elif defined(ZF_PLATFORM_LINUX)
    #error "Platform support not complete!" // @todo
        const strs::StrRdonly shaderc_file_path_rel = ZF_STR_LITERAL("tools/bgfx/shaderc_linux");
        const char platform_cstr[] = "linux";
        const char profile_cstr[] = "glsl";
#endif

        const strs::StrRdonly exe_dir = LoadExecutableDirectory(temp_arena);
        ZF_ASSERT(exe_dir.bytes[exe_dir.bytes.len - 1] == '/' || exe_dir.bytes[exe_dir.bytes.len - 1] == '\\'); // Assuming this.

        const strs::StrMut shaderc_file_path_terminated = {f_mem_push_array<t_u8>(temp_arena, exe_dir.bytes.len + shaderc_file_path_rel.bytes.len + 1)};
        s_stream shaderc_file_path_terminated_byte_stream = CreateMemStream(shaderc_file_path_terminated.bytes, ek_stream_mode_write);
        PrintFormat(&shaderc_file_path_terminated_byte_stream, ZF_STR_LITERAL("%%\0"), exe_dir, shaderc_file_path_rel);
        ZF_ASSERT(strs::determine_are_bytes_terminated_only_at_end(shaderc_file_path_terminated.bytes));

        const strs::StrRdonly shaderc_include_dir_rel = ZF_STR_LITERAL("tools/bgfx/shaderc_include");
        const strs::StrMut shaderc_include_dir_terminated = {f_mem_push_array<t_u8>(temp_arena, exe_dir.bytes.len + shaderc_include_dir_rel.bytes.len + 1)};
        s_stream shaderc_include_dir_terminated_byte_stream = CreateMemStream(shaderc_include_dir_terminated.bytes, ek_stream_mode_write);
        PrintFormat(&shaderc_include_dir_terminated_byte_stream, ZF_STR_LITERAL("%%\0"), exe_dir, shaderc_include_dir_rel);
        ZF_ASSERT(strs::determine_are_bytes_terminated_only_at_end(shaderc_include_dir_terminated.bytes));

        const t_static_array<const char *, 15> args = {{
            get_as_cstr(shaderc_file_path_terminated),
            "-f",
            get_as_cstr(shader_file_path_terminated),
            "--type",
            is_frag ? "fragment" : "vertex",
            "--platform",
            platform_cstr,
            "--profile",
            profile_cstr,
            "--varyingdef",
            get_as_cstr(varying_def_file_path_terminated),
            "-i",
            get_as_cstr(shaderc_include_dir_terminated),
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
            t_static_array<t_u8, 4096> buf;
            r = reproc_read(proc, REPROC_STREAM_OUT, buf.raw, ZF_SIZE_OF(buf));

            if (r < 0) {
                break;
            }

            ListAppendManyDynamic(&bin_list, f_mem_slice_array(f_mem_as_nonstatic_array(buf), 0, r), bin_arena);
        }

        if (r != REPROC_EPIPE) {
            return false;
        }

        r = reproc_wait(proc, REPROC_INFINITE);

        if (r < 0) {
            return false;
        }

        if (r > 0) {
            s_stream std_err = StdError();
            const auto err = strs::StrRdonly(bin_list.AsArray());
            PrintFormat(&std_err, ZF_STR_LITERAL("==================== BGFX SHADERC ERROR ====================\n%============================================================\n"), err);
            return false;
        }

        *o_bin = bin_list.AsArray();

        return true;
    }
}
