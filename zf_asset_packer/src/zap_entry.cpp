#include "zap_packing.h"

int main(const int arg_cnt, const char* const* const args_raw) {
    const zf::s_array<const char* const> args = {args_raw, arg_cnt};

    if (args.len != 2) {
        ZF_LOG("Invalid number of command-line arguments provided! Expected a path to an packing instructions JSON file!");
        return EXIT_FAILURE;
    }

    const zf::s_str_rdonly instrs_json_file_path = zf::StrFromRawTerminated(args[1]);

    zf::s_mem_arena temp_mem_arena;

    if (!zf::MakeMemArena(temp_mem_arena)) {
        ZF_LOG_ERROR("Failed to initialise temporary memory arena!");
        return EXIT_FAILURE;
    }

    const zf::t_b8 success = [instrs_json_file_path, &temp_mem_arena]() {
        zf::s_str instrs_json;

        if (!zf::LoadFileContentsAsStr(temp_mem_arena, instrs_json_file_path, instrs_json)) {
            ZF_LOG_ERROR("Failed to load contents of asset packing instructions JSON file \"%s\"!", zf::StrRaw(instrs_json_file_path));
            return false;
        }

        if (!zf::PackAssets(instrs_json, temp_mem_arena)) {
            ZF_LOG_ERROR("Failed to pack assets!");
            return false;
        }

        return true;
    }();

    zf::ReleaseMemArena(temp_mem_arena);

    if (success) {
        ZF_LOG_SUCCESS("Asset packing completed!");
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
