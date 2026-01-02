#include "zap.h"

int main(const int arg_cnt, const char *const *const args_raw) {
    const zf::s_array_rdonly<const char *> args = {args_raw, arg_cnt};

    if (args.len != 2) {
        zf::LogError("Invalid number of command-line arguments provided! Expected a path to a packing instructions JSON file.");
        return EXIT_FAILURE;
    }

    return zf::PackAssets(zf::ConvertCstr(args[1])) ? EXIT_SUCCESS : EXIT_FAILURE;
}
