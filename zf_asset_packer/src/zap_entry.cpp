#include "zap_packing.h"

constexpr zf::t_size g_temp_mem_arena_size = zf::Megabytes(8);

int main(const int arg_cnt, const char* const* const args_raw) {
    const zf::s_array_rdonly<const char*> args = {args_raw, arg_cnt};

    if (args.len != 2) {
        ZF_LOG("Invalid number of command-line arguments provided! Expected a path to an packing instructions JSON file!");
        return EXIT_FAILURE;
    }

    const zf::s_str_rdonly instrs_json_file_path = zf::StrFromRawTerminated(args[1]);

    zf::s_mem_arena temp_mem_arena;

    if (!zf::AllocMemArena(g_temp_mem_arena_size, temp_mem_arena)) {
        ZF_LOG_ERROR("Failed to initialise temporary memory arena!");
        return EXIT_FAILURE;
    }

    ZF_DEFER({ zf::FreeMemArena(temp_mem_arena); });

    zf::s_str instrs_json;

    if (!zf::LoadFileContentsAsStr(temp_mem_arena, instrs_json_file_path, instrs_json)) {
        ZF_LOG_ERROR("Failed to load contents of asset packing instructions JSON file \"%s\"!", zf::StrRaw(instrs_json_file_path));
        return EXIT_FAILURE;
    }

    if (!zf::PackAssets(instrs_json, temp_mem_arena)) {
        ZF_LOG_ERROR("Failed to pack assets!");
        return EXIT_FAILURE;
    }

    ZF_LOG_SUCCESS("Asset packing completed!");

    return EXIT_SUCCESS;
}
