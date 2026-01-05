#include "zap.h"

int main(const int arg_cnt, const char *const *const args_raw) {
    const zf::t_array_rdonly<const char *> args = {args_raw, arg_cnt};

    if (args.len != 2) {
        zf::io::log_error(ZF_STR_LITERAL("Invalid number of command-line arguments provided! Expected a path to a packing instructions JSON file."));
        return EXIT_FAILURE;
    }

    return zf::pack_assets(zf::strs::cstr_convert(args[1])) ? EXIT_SUCCESS : EXIT_FAILURE;
}
