#include "zap.h"

#include <reproc/run.h>

namespace zf {
    static bool Fart(c_file_writer& fw, const c_string_view file_path, const bool is_fs, const c_string_view varying_def_file_path) {
        const char* args[] = {
            "shaderc",
            "-f", file_path.Raw(),
            "--type", is_fs ? "fragment" : "vertex",
            "--platform", "windows", // @todo: Update!
            "--profile", "s_5_0",
            "--varyingdef", varying_def_file_path.Raw(),
            nullptr
        };

        reproc_options options = {};
        options.redirect.out.type = REPROC_REDIRECT_FILE;
        options.redirect.out.file = fw.Raw();

        const int res = reproc_run(args, options);

        if (res < 0) {
            ZF_LOG_ERROR_SPECIAL("shaderc", "%d", res);
            return false;
        }

        return true;
    }

    bool PackShaderProgFromRawFiles(c_file_writer& fw, const c_string_view vs_file_path, const c_string_view fs_file_path, const c_string_view varying_def_file_path) {
        if (!Fart(fw, vs_file_path, false, varying_def_file_path)) {
            return false;
        }

        if (!Fart(fw, fs_file_path, true, varying_def_file_path)) {
            return false;
        }

        return true;
    }
}
