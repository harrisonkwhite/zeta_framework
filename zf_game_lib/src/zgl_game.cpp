#include <zgl/zgl_game.h>

#include <zgl/zgl_audio.h>
#include <zgl/zgl_gfx.h>
#include <zgl/zgl_input.h>
#include <zgl/zgl_platform.h>

namespace zf {
    t_b8 RunGame() {
#ifndef ZF_DEBUG
        // Redirect stderr to crash log file.
        freopen("error.log", "w", stderr);
#endif

        const t_b8 success = []() {
            //
            // Initialisation
            //
            s_mem_arena mem_arena;

            if (!mem_arena.Init(zf::Megabytes(80))) {
                ZF_REPORT_ERROR();
                return false;
            }

            ZF_DEFER({ mem_arena.Release(); });

            s_mem_arena temp_mem_arena;

            if (!temp_mem_arena.InitAsChild(zf::Megabytes(10), &mem_arena)) {
                ZF_REPORT_ERROR();
                return false;
            }

            return true;
        }();

#ifndef ZF_DEBUG
        if (!success) {
            ShowErrorBox("Error", "A fatal error occurred! Please check \"error.log\" for details.");
        }
#endif

        return success;
    }
}
