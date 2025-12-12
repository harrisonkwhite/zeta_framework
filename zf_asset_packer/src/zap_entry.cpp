#include "zap_packing.h"

int main(const int arg_cnt, const char *const *const args_raw) {
#if 0
    const zf::s_array_rdonly<const char *> args = {args_raw, arg_cnt};

    if (args.Len() != 2) {
        zf::LogError("Invalid number of command-line arguments provided! Expected a path to a packing instructions JSON file!");
        return EXIT_FAILURE;
    }

    return zf::RunPacker(args[1]) ? EXIT_SUCCESS : EXIT_FAILURE;
#endif
}
