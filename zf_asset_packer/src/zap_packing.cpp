#include "zap_packing.h"

#include <cJSON.h>

namespace zf {
    struct s_packed_header {
        t_s32 tex_cnt = 0;
        t_s32 font_cnt = 0;
        t_s32 shader_prog_cnt = 0;
    };

    static t_b8 PackTexturesFromInstrs(cJSON* const cj, c_mem_arena& temp_mem_arena) {
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

            s_texture_data_mut tex_data;

            if (!LoadTextureFromRaw(StrFromRawTerminated(cj_src_file_path->valuestring), temp_mem_arena, tex_data)) {
                ZF_LOG_ERROR("Failed to load RGBA texture from file \"%s\"!", cj_src_file_path->valuestring);
                return false;
            }

            if (!PackTexture(tex_data, StrFromRawTerminated(cj_dest_file_path->valuestring), temp_mem_arena)) {
                ZF_LOG_ERROR("Failed to pack texture from file \"%s\"!", cj_src_file_path->valuestring);
                return false;
            }

            ZF_LOG_SUCCESS("Packed texture from file \"%s\"!", cj_src_file_path->valuestring);

            temp_mem_arena.Rewind(temp_mem_arena_offs_old);
        }

        return true;
    }

    static t_b8 PackSoundsFromInstrs(cJSON* const cj, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(cj);

        cJSON* const cj_snds = cJSON_GetObjectItemCaseSensitive(cj, "sounds");

        if (!cJSON_IsArray(cj_snds)) {
            ZF_LOG_ERROR("No \"sounds\" array found in asset packing instructions!");
            return false;
        }

        cJSON* cj_snd = nullptr;

        cJSON_ArrayForEach(cj_snd, cj_snds) {
            if (!cJSON_IsObject(cj_snd)) {
                ZF_LOG_ERROR("Invalid sound entry in asset packing instructions!");
                continue;
            }

            const cJSON* const cj_src_file_path = cJSON_GetObjectItem(cj_snd, "src_file_path");

            if (!cJSON_IsString(cj_src_file_path)) {
                ZF_LOG_ERROR("\"src_file_path\" field missing or invalid in sound entry!");
                return false;
            }

            const cJSON* const cj_dest_file_path = cJSON_GetObjectItem(cj_snd, "dest_file_path");

            if (!cJSON_IsString(cj_dest_file_path)) {
                ZF_LOG_ERROR("\"dest_file_path\" field missing or invalid in sound entry!");
                return false;
            }

            const auto temp_mem_arena_offs_old = temp_mem_arena.Offs();

            s_sound_data snd_data;

            if (!LoadSoundFromRaw(StrFromRawTerminated(cj_src_file_path->valuestring), temp_mem_arena, snd_data)) {
                ZF_LOG_ERROR("Failed to load sound from file \"%s\"!", cj_src_file_path->valuestring);
                return false;
            }

            if (!PackSound(snd_data, StrFromRawTerminated(cj_dest_file_path->valuestring), temp_mem_arena)) {
                ZF_LOG_ERROR("Failed to pack sound from file \"%s\"!", cj_src_file_path->valuestring);
                return false;
            }

            ZF_LOG_SUCCESS("Packed sound from file \"%s\"!", cj_src_file_path->valuestring);

            temp_mem_arena.Rewind(temp_mem_arena_offs_old);
        }

        return true;
    }

    t_b8 PackAssets(const s_str_ro instrs_json, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(instrs_json));

        cJSON* const cj = cJSON_Parse(instrs_json.Raw());

        if (!cj) {
            ZF_LOG_ERROR_SPECIAL("cJSON", "%s", cJSON_GetErrorPtr());
            return false;
        }

        const t_b8 success = [cj, &temp_mem_arena]() {
            if (!cJSON_IsObject(cj)) {
                ZF_LOG_ERROR("Invalid JSON structure in asset packing instructions!");
                return false;
            }

            if (!PackTexturesFromInstrs(cj, temp_mem_arena)) {
                ZF_LOG_ERROR("Failed to pack textures!");
                return false;
            }

            if (!PackSoundsFromInstrs(cj, temp_mem_arena)) {
                ZF_LOG_ERROR("Failed to pack sounds!");
                return false;
            }

            return true;
        }();

        cJSON_Delete(cj);

        return success;
    }
}
