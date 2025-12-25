#include "zap_packing.h"

#include <reproc/reproc.h>

namespace zf {
    t_b8 CompileShader(const s_str_rdonly shader_file_path, const s_str_rdonly shaderc_file_path, const t_b8 is_frag, s_mem_arena &temp_mem_arena) {
        const s_str_rdonly shader_file_path_terminated = AllocStrCloneButAddTerminator(shader_file_path, temp_mem_arena);
        const s_str_rdonly shaderc_file_path_terminated = AllocStrCloneButAddTerminator(shaderc_file_path, temp_mem_arena);

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

        const s_static_array<const char *, 2> args = {
            shaderc_file_path_terminated.AsCstr(),
            nullptr,
        };

        r = reproc_start(proc, args.raw, {});

        if (r < 0) {
            return false;
        }

        r = reproc_close(proc, REPROC_STREAM_IN);

        if (r < 0) {
            return false;
        }

        do {
            s_static_array<t_u8, 4096> buf;
            r = reproc_read(proc, REPROC_STREAM_OUT, buf.raw, ZF_SIZE_OF(buf));

            if (r >= 0) {
                Log(s_cstr_literal("Read % bytes from shaderc output!"), r);
            }
        } while (r >= 0);

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
