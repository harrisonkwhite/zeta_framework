#include "zap.h"

#include <reproc/reproc.h>

namespace zf {
    t_b8 compile_shader(const t_str_rdonly shader_file_path, const t_str_rdonly varying_def_file_path, const t_b8 is_frag, t_arena *const bin_arena, t_arena *const temp_arena, t_array_mut<t_u8> *const o_bin) {
        const t_str_rdonly shader_file_path_terminated = f_strs_clone_but_add_terminator(shader_file_path, temp_arena);
        const t_str_rdonly varying_def_file_path_terminated = f_strs_clone_but_add_terminator(varying_def_file_path, temp_arena);

        t_i32 r = 0;

        ZF_DEFER({
            if (r < 0) {
                const auto err = f_strs_convert_cstr(reproc_strerror(r));
                f_io_log_error(ZF_STR_LITERAL("%"), err);
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
        const t_str_rdonly shaderc_file_path_rel = ZF_STR_LITERAL("tools/bgfx/shaderc_windows.exe");
        const char platform_cstr[] = "windows";
        const char profile_cstr[] = "s_5_0";
#elif defined(ZF_PLATFORM_MACOS)
    #error "Platform support not complete!" // @todo
        const StrRdonly shaderc_file_path_rel = ZF_STR_LITERAL("tools/bgfx/shaderc_macos");
        const char platform_cstr[] = "osx";
        const char profile_cstr[] = "metal";
#elif defined(ZF_PLATFORM_LINUX)
    #error "Platform support not complete!" // @todo
        const StrRdonly shaderc_file_path_rel = ZF_STR_LITERAL("tools/bgfx/shaderc_linux");
        const char platform_cstr[] = "linux";
        const char profile_cstr[] = "glsl";
#endif

        const t_str_rdonly exe_dir = f_io_get_executable_directory(temp_arena);
        ZF_ASSERT(exe_dir.bytes[exe_dir.bytes.len - 1] == '/' || exe_dir.bytes[exe_dir.bytes.len - 1] == '\\'); // Assuming this.

        const t_str_mut shaderc_file_path_terminated = {f_mem_arena_push_array<t_u8>(temp_arena, exe_dir.bytes.len + shaderc_file_path_rel.bytes.len + 1)};
        t_stream shaderc_file_path_terminated_byte_stream = f_io_create_mem_stream(shaderc_file_path_terminated.bytes, ec_stream_mode_write);
        f_io_print_fmt(&shaderc_file_path_terminated_byte_stream, ZF_STR_LITERAL("%%\0"), exe_dir, shaderc_file_path_rel);
        ZF_ASSERT(f_strs_are_bytes_terminated_only_at_end(shaderc_file_path_terminated.bytes));

        const t_str_rdonly shaderc_include_dir_rel = ZF_STR_LITERAL("tools/bgfx/shaderc_include");
        const t_str_mut shaderc_include_dir_terminated = {f_mem_arena_push_array<t_u8>(temp_arena, exe_dir.bytes.len + shaderc_include_dir_rel.bytes.len + 1)};
        t_stream shaderc_include_dir_terminated_byte_stream = f_io_create_mem_stream(shaderc_include_dir_terminated.bytes, ec_stream_mode_write);
        f_io_print_fmt(&shaderc_include_dir_terminated_byte_stream, ZF_STR_LITERAL("%%\0"), exe_dir, shaderc_include_dir_rel);
        ZF_ASSERT(f_strs_are_bytes_terminated_only_at_end(shaderc_include_dir_terminated.bytes));

        const t_static_array<const char *, 15> args = {{
            f_strs_get_as_cstr(shaderc_file_path_terminated),
            "-f",
            f_strs_get_as_cstr(shader_file_path_terminated),
            "--type",
            is_frag ? "fragment" : "vertex",
            "--platform",
            platform_cstr,
            "--profile",
            profile_cstr,
            "--varyingdef",
            f_strs_get_as_cstr(varying_def_file_path_terminated),
            "-i",
            f_strs_get_as_cstr(shaderc_include_dir_terminated),
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

            ListAppendManyDynamic(&bin_list, f_array_slice(f_array_get_as_nonstatic(buf), 0, r), bin_arena);
        }

        if (r != REPROC_EPIPE) {
            return false;
        }

        r = reproc_wait(proc, REPROC_INFINITE);

        if (r < 0) {
            return false;
        }

        if (r > 0) {
            t_stream std_err = f_io_get_std_error();
            const auto err = t_str_rdonly(bin_list.AsArray());
            f_io_print_fmt(&std_err, ZF_STR_LITERAL("==================== BGFX SHADERC ERROR ====================\n%============================================================\n"), err);
            return false;
        }

        *o_bin = bin_list.AsArray();

        return true;
    }
}
