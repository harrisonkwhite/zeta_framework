#include "zap_packing.h"

int main(const int arg_cnt, const char *const *const args_raw) {
    const zf::s_array_rdonly<const char *> args = {args_raw, arg_cnt};

    if (args.Len() != 3) {
        zf::LogError(zf::s_cstr_literal("Invalid number of command-line arguments provided!"));
        // @todo: The error handling here sucks!
        return EXIT_FAILURE;
    }

    zf::s_mem_arena temp_mem_arena = {};

    if (!zf::CompileShader(zf::s_cstr_literal(""), zf::ConvertCstr(args[2]), false, temp_mem_arena)) {
        return EXIT_FAILURE;
    }

    return zf::RunPacker(zf::ConvertCstr(args[1])) ? EXIT_SUCCESS : EXIT_FAILURE;
}
