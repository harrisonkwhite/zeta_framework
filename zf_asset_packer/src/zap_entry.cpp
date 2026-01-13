#include "zap.h"

int main(const int arg_cnt, const char *const *const args_raw) {
    const zcl::t_array_rdonly<const char *> args = {args_raw, arg_cnt};

    if (args.len != 2) {
        zcl::log_error(ZCL_STR_LITERAL("Invalid number of command-line arguments provided! Expected a path to a packing instructions JSON file."));
        return EXIT_FAILURE;
    }

    return pack_assets(zcl::cstr_to_str(args[1])) ? EXIT_SUCCESS : EXIT_FAILURE;
}
