#include "zap_packing.h"
#include "zc_io.h"
#include "zc_mem.h"

static bool ProcCmdlineArgs(const zf::c_array<const char* const> args, zf::c_string_view& instrs_json_file_path, zf::c_string_view& output_file_path) {
    if ((args.Len() - 1) % 2 != 0) {
        ZF_LOG_ERROR("Invalid command-line argument count!");
        return false;
    }

    for (int i = 1; i < args.Len(); i++) {
        if (strcmp(args[i], "-ij") == 0) {
            if (!instrs_json_file_path.IsEmpty()) {
                ZF_LOG_ERROR("Command-line argument \"-ij\" specified twice!");
                return false;
            }

            instrs_json_file_path = {args[i + 1]};
            i++;
        } else if (strcmp(args[i], "-o") == 0) {
            if (!output_file_path.IsEmpty()) {
                ZF_LOG_ERROR("Command-line argument \"-o\" specified twice!");
                return false;
            }

            output_file_path = {args[i + 1]};
            i++;
        } else {
            ZF_LOG_ERROR("Unsupported command-line argument \"%s\"!", args[i]);
            return false;
        }
    }

    return true;
}

int main(const int arg_cnt, const char* const* const args_raw) {
    const zf::c_array<const char* const> args = {args_raw, arg_cnt};

    zf::c_string_view instrs_json_file_path;
    zf::c_string_view output_file_path;

    if (!ProcCmdlineArgs(args, instrs_json_file_path, output_file_path)) {
        ZF_LOG_ERROR("Error while processing command-line arguments!");
        return EXIT_FAILURE;
    }

    if (instrs_json_file_path.IsEmpty()) {
        instrs_json_file_path = "zf_asset_packing_instrs.json";
    }

    if (output_file_path.IsEmpty()) {
        output_file_path = "assets.zfdat";
    }

    zf::c_mem_arena temp_mem_arena;

    if (!temp_mem_arena.Init(zf::Megabytes(4))) {
        ZF_LOG_ERROR("Failed to initialise temporary memory arena!");
        return EXIT_FAILURE;
    }

    const auto instrs_json = zf::LoadFileContentsAsStr(instrs_json_file_path, temp_mem_arena);

    if (instrs_json.IsEmpty()) {
        ZF_LOG_ERROR("Failed to load contents of asset packing instructions JSON file \"%s\"!", instrs_json_file_path.Raw());
        temp_mem_arena.Clean();
        return EXIT_FAILURE;
    }

    if (!PackAssets(instrs_json, output_file_path, temp_mem_arena)) {
        ZF_LOG_ERROR("Failed to pack assets!");
        temp_mem_arena.Clean();
        return EXIT_FAILURE;
    }

    temp_mem_arena.Clean();

    ZF_LOG_SUCCESS("Asset packing completed!");

    return EXIT_SUCCESS;
}
