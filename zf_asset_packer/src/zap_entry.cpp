#include "zap_packing.h"
#include "zc_mem.h"

static bool ProcCmdlineArgs(const zf::c_array<const char* const> args, zf::c_string_view& instrs_json_file_path, zf::c_string_view& output_file_path) {
    if ((args.Len() - 1) % 2 != 0) {
        return false;
    }

    for (int i = 1; i < args.Len(); i++) {
        if (strcmp(args[i], "-ij") == 0) {
            if (!instrs_json_file_path.IsEmpty()) {
                return false;
            }

            instrs_json_file_path = {args[i + 1]};
            i++;
        } else if (strcmp(args[i], "-o") == 0) {
            if (!output_file_path.IsEmpty()) {
                return false;
            }

            output_file_path = {args[i + 1]};
            i++;
        } else {
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
        return EXIT_FAILURE;
    }

    if (instrs_json_file_path.IsEmpty()) {
        instrs_json_file_path = "zf_asset_packing_instrs.json";
    }

    if (output_file_path.IsEmpty()) {
        output_file_path = "assets.zfdat";
    }

    zf::c_mem_arena temp_mem_arena;

    if (!temp_mem_arena.Init(zf::Megabytes(1))) {
        return EXIT_FAILURE;
    }

    if (!PackAssets(instrs_json_file_path, output_file_path, temp_mem_arena)) {
        temp_mem_arena.Clean();
        return EXIT_FAILURE;
    }

    temp_mem_arena.Clean();

    return EXIT_SUCCESS;
}
