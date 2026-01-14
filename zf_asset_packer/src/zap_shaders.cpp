#include "zap.h"

#include <reproc/reproc.h>

zcl::t_b8 compile_shader(const zcl::t_str_rdonly shader_file_path, const zcl::t_str_rdonly varying_def_file_path, const zcl::t_b8 is_frag, zcl::t_arena *const bin_arena, zcl::t_arena *const temp_arena, zcl::t_array_mut<zcl::t_u8> *const o_bin) {
    const zcl::t_str_rdonly shader_file_path_terminated = zcl::str_clone_but_add_terminator(shader_file_path, temp_arena);
    const zcl::t_str_rdonly varying_def_file_path_terminated = zcl::str_clone_but_add_terminator(varying_def_file_path, temp_arena);

    zcl::t_i32 r = 0;

    ZCL_DEFER({
        if (r < 0) {
            const auto err = zcl::cstr_to_str(reproc_strerror(r));
            zcl::LogError(ZCL_STR_LITERAL("%"), err);
        }
    });

    reproc_t *const proc = reproc_new();

    if (!proc) {
        return false;
    }

    ZCL_DEFER({
        reproc_destroy(proc);
    });

#if defined(ZCL_PLATFORM_WINDOWS)
    const zcl::t_str_rdonly shaderc_file_path_rel = ZCL_STR_LITERAL("tools/bgfx/shaderc_windows.exe");
    const char platform_cstr[] = "windows";
    const char profile_cstr[] = "s_5_0";
#elif defined(ZCL_PLATFORM_MACOS)
    #error "Platform support not complete!" // @todo
    const zcl::t_str_rdonly shaderc_file_path_rel = ZCL_STR_LITERAL("tools/bgfx/shaderc_macos");
    const char platform_cstr[] = "osx";
    const char profile_cstr[] = "metal";
#elif defined(ZCL_PLATFORM_LINUX)
    #error "Platform support not complete!" // @todo
    const zcl::t_str_rdonly shaderc_file_path_rel = ZCL_STR_LITERAL("tools/bgfx/shaderc_linux");
    const char platform_cstr[] = "linux";
    const char profile_cstr[] = "glsl";
#endif

    const zcl::t_str_rdonly exe_dir = zcl::get_executable_directory(temp_arena);
    ZCL_ASSERT(exe_dir.bytes[exe_dir.bytes.len - 1] == '/' || exe_dir.bytes[exe_dir.bytes.len - 1] == '\\'); // Assuming this.

    const zcl::t_str_mut shaderc_file_path_terminated = {zcl::arena_push_array<zcl::t_u8>(temp_arena, exe_dir.bytes.len + shaderc_file_path_rel.bytes.len + 1)};
    zcl::t_mem_stream shaderc_file_path_terminated_byte_stream = zcl::mem_stream_create(shaderc_file_path_terminated.bytes, zcl::ek_stream_mode_write);
    zcl::PrintFormat(shaderc_file_path_terminated_byte_stream, ZCL_STR_LITERAL("%%\0"), exe_dir, shaderc_file_path_rel);
    ZCL_ASSERT(zcl::str_bytes_check_terminated_only_at_end(shaderc_file_path_terminated.bytes));

    const zcl::t_str_rdonly shaderc_include_dir_rel = ZCL_STR_LITERAL("tools/bgfx/shaderc_include");
    const zcl::t_str_mut shaderc_include_dir_terminated = {zcl::arena_push_array<zcl::t_u8>(temp_arena, exe_dir.bytes.len + shaderc_include_dir_rel.bytes.len + 1)};
    zcl::t_mem_stream shaderc_include_dir_terminated_byte_stream = zcl::mem_stream_create(shaderc_include_dir_terminated.bytes, zcl::ek_stream_mode_write);
    zcl::PrintFormat(shaderc_include_dir_terminated_byte_stream, ZCL_STR_LITERAL("%%\0"), exe_dir, shaderc_include_dir_rel);
    ZCL_ASSERT(zcl::str_bytes_check_terminated_only_at_end(shaderc_include_dir_terminated.bytes));

    const zcl::t_static_array<const char *, 15> args = {{
        zcl::str_to_cstr(shaderc_file_path_terminated),
        "-f",
        zcl::str_to_cstr(shader_file_path_terminated),
        "--type",
        is_frag ? "fragment" : "vertex",
        "--platform",
        platform_cstr,
        "--profile",
        profile_cstr,
        "--varyingdef",
        zcl::str_to_cstr(varying_def_file_path_terminated),
        "-i",
        zcl::str_to_cstr(shaderc_include_dir_terminated),
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

    zcl::t_list<zcl::t_u8> bin_list = {};

    while (true) {
        zcl::t_static_array<zcl::t_u8, 4096> buf;
        r = reproc_read(proc, REPROC_STREAM_OUT, buf.raw, ZCL_SIZE_OF(buf));

        if (r < 0) {
            break;
        }

        zcl::list_append_many_dynamic(&bin_list, zcl::array_slice(zcl::array_to_nonstatic(&buf), 0, r), bin_arena);
    }

    if (r != REPROC_EPIPE) {
        return false;
    }

    r = reproc_wait(proc, REPROC_INFINITE);

    if (r < 0) {
        return false;
    }

    if (r > 0) {
        zcl::t_file_stream std_err = zcl::file_stream_create_std_error();
        const auto err = zcl::t_str_rdonly{zcl::list_to_array(&bin_list)};
        zcl::PrintFormat(std_err, ZCL_STR_LITERAL("==================== BGFX SHADERC ERROR ====================\n%============================================================\n"), err);
        return false;
    }

    *o_bin = zcl::list_to_array(&bin_list);

    return true;
}
