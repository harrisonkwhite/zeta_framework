#include "zap.h"

int main(const int arg_cnt, const char *const *const args_raw) {
    const zf::s_array_rdonly<const char *> args = {args_raw, arg_cnt};

    if (args.Len() != 3) {
        zf::LogError(zf::s_cstr_literal("Invalid number of command-line arguments provided!"));
        // @todo: The error handling here sucks!
        return EXIT_FAILURE;
    }

    return zf::PackAssets(zf::ConvertCstr(args[1]), zf::ConvertCstr(args[2])) ? EXIT_SUCCESS : EXIT_FAILURE;
}
