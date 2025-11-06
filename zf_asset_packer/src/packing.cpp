#include "packing.h"

#include <zc/debug.h>
#include <cJSON.h>

namespace zf {
    struct s_packed_header {
        int tex_cnt = 0;
        int font_cnt = 0;
        int shader_prog_cnt = 0;
    };

    static bool PackTexturesFromInstrs(cJSON* const cj, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(cj);

        cJSON* const cj_textures = cJSON_GetObjectItemCaseSensitive(cj, "textures");

        if (!cJSON_IsArray(cj_textures)) {
            ZF_LOG_ERROR("No \"textures\" array found in asset packing instructions!");
            return false;
        }

        cJSON* cj_tex = nullptr;

        cJSON_ArrayForEach(cj_tex, cj_textures) {
            if (!cJSON_IsObject(cj_tex)) {
                ZF_LOG_ERROR("Invalid texture entry in asset packing instructions!");
                continue;
            }

            const cJSON* const cj_src_file_path = cJSON_GetObjectItem(cj_tex, "src_file_path");

            if (!cJSON_IsString(cj_src_file_path)) {
                ZF_LOG_ERROR("\"src_file_path\" field missing or invalid in texture entry!");
                return false;
            }

            const cJSON* const cj_dest_file_path = cJSON_GetObjectItem(cj_tex, "dest_file_path");

            if (!cJSON_IsString(cj_dest_file_path)) {
                ZF_LOG_ERROR("\"dest_file_path\" field missing or invalid in texture entry!");
                return false;
            }

            const auto temp_mem_arena_offs_old = temp_mem_arena.Offs();

            s_rgba_texture rgba_tex;

            if (!LoadRGBATextureFromRaw(rgba_tex, temp_mem_arena, zf::s_str_view::FromRawTerminated(cj_src_file_path->valuestring))) {
                ZF_LOG_ERROR("Failed to load RGBA texture from file \"%s\"!", cj_src_file_path->valuestring);
                return false;
            }

            if (!PackTexture(zf::s_str_view::FromRawTerminated(cj_dest_file_path->valuestring), rgba_tex, temp_mem_arena)) {
                ZF_LOG_ERROR("Failed to pack texture from file \"%s\"!", cj_src_file_path->valuestring);
                return false;
            }

            ZF_LOG_SUCCESS("Packed texture from file \"%s\"!", cj_src_file_path->valuestring);

            temp_mem_arena.Rewind(temp_mem_arena_offs_old);
        }

        return true;
    }

    bool PackAssets(const s_str_view instrs_json, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(instrs_json.IsTerminated());

        cJSON* const cj = cJSON_Parse(instrs_json.Raw());

        if (!cj) {
            ZF_LOG_ERROR_SPECIAL("cJSON", "%s", cJSON_GetErrorPtr());
            return false;
        }

        const bool success = [cj, &temp_mem_arena]() {
            if (!cJSON_IsObject(cj)) {
                ZF_LOG_ERROR("Invalid JSON structure in asset packing instructions!");
                return false;
            }

            if (!PackTexturesFromInstrs(cj, temp_mem_arena)) {
                ZF_LOG_ERROR("Failed to pack textures!");
                return false;
            }

            return true;
        }();

        cJSON_Delete(cj);

        return success;
    }
}
