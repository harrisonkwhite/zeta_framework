#include "zfwap.h"

#include <stdlib.h>
#include <cJSON.h>

#define MEM_ARENA_SIZE MEGABYTES(20)

static const c_array<const char> g_json_file_path = ARRAY_FROM_STATIC("asset_packing_instrs.json"); // TODO: Maybe pass this in through command-line arguments instead?

static bool PackAssets(cJSON* const cj, c_mem_arena* const temp_mem_arena) {
    if (!cJSON_IsArray(cj)) {
        return false;
    }

    const t_s32 item_cnt = cJSON_GetArraySize(cj);

    for (t_s32 i = 0; i < item_cnt; i++) {
        const cJSON* const cj_item = cJSON_GetArrayItem(cj, i);

        const cJSON* const cj_type = cJSON_GetObjectItem(cj_item, "type");
        const cJSON* const cj_output_file_path = cJSON_GetObjectItem(cj_item, "output_file_path");

        if (!cJSON_IsString(cj_type) || !cJSON_IsString(cj_output_file_path)) {
            LOG_ERROR("Invalid JSON item at index %d; \"type\" and \"output_file_path\" should be strings!", i);
            return false;
        }

        const c_array<const char> output_file_path = StrViewFromRawTerminated(cj_output_file_path->valuestring);

        if (strcmp(cj_type->valuestring, "texture") == 0) {
            const cJSON* const cj_file_path = cJSON_GetObjectItem(cj_item, "file_path");

            if (!cJSON_IsString(cj_file_path)) {
                LOG_ERROR("Invalid JSON item at index %d; \"file_path\" should be a string for texture type!", i);
                return false;
            }

            const c_array<const char> file_path = StrViewFromRawTerminated(cj_file_path->valuestring);

            if (!PackTexture(file_path, output_file_path, temp_mem_arena)) {
                LOG_ERROR("Failed to pack texture with file path \"%s\"!", file_path.buf_raw);
                return false;
            }
        } else if (strcmp(cj_type->valuestring, "font") == 0) {
            const cJSON* const cj_file_path = cJSON_GetObjectItem(cj_item, "file_path");
            const cJSON* const cj_height = cJSON_GetObjectItem(cj_item, "height");

            if (!cJSON_IsString(cj_file_path) || !cJSON_IsNumber(cj_height)) {
                LOG_ERROR("Invalid JSON item at index %d; \"file_path\" should be a string and \"height\" should be a number for font type!", i);
                return false;
            }

            const c_array<const char> file_path = StrViewFromRawTerminated(cj_file_path->valuestring);

            if (!PackFont(file_path, cj_height->valueint, output_file_path, temp_mem_arena)) {
                LOG_ERROR("Failed to pack font with file path \"%s\" and height %d!", file_path.buf_raw, cj_height->valueint);
                return false;
            }
        } else if (strcmp(cj_type->valuestring, "shader_prog") == 0) {
            const cJSON* const cj_vert_file_path = cJSON_GetObjectItem(cj_item, "vert_file_path");
            const cJSON* const cj_frag_file_path = cJSON_GetObjectItem(cj_item, "frag_file_path");

            if (!cJSON_IsString(cj_vert_file_path) || !cJSON_IsString(cj_frag_file_path)) {
                LOG_ERROR("Invalid JSON item at index %d; \"vert_file_path\" and \"frag_file_path\" should be strings for shader program type!", i);
                return false;
            }

            const c_array<const char> vert_file_path = StrViewFromRawTerminated(cj_vert_file_path->valuestring);

            const c_array<const char> frag_file_path = StrViewFromRawTerminated(cj_frag_file_path->valuestring);

            if (!PackShaderProg(vert_file_path, frag_file_path, output_file_path, temp_mem_arena)) {
                LOG_ERROR("Failed to pack shader program with vertex shader file path \"%s\" and fragment shader file path \"%s\"!", vert_file_path.buf_raw, frag_file_path.buf_raw);
                return false;
            }
        }

        RewindMemArena(temp_mem_arena, 0);
    }

    return true;
}

int main() {
    c_mem_arena mem_arena;

    if (!mem_arena.Init(MEM_ARENA_SIZE)) {
        LOG_ERROR("Failed to initialise the asset packer memory arena!");
        return EXIT_FAILURE;
    }

    const c_array<const char> json_file_contents = LoadFileContentsAsStr(g_json_file_path, &mem_arena);

    if (json_file_contents.IsEmpty()) {
        LOG_ERROR("Failed to load contents of asset packing JSON file \"%s\"!", g_json_file_path.buf_raw);
        CleanMemArena(&mem_arena);
        return EXIT_FAILURE;
    }

    cJSON* const cj = cJSON_Parse(json_file_contents.buf_raw);

    if (!cj) {
        LOG_ERROR_SPECIAL("cJSON", "%s", cJSON_GetErrorPtr());
        CleanMemArena(&mem_arena);
        return EXIT_FAILURE;
    }

    if (!PackAssets(cj, &mem_arena)) {
        LOG_ERROR("Failed to pack assets from JSON file \"%s\"!", g_json_file_path.buf_raw);
        cJSON_Delete(cj);
        CleanMemArena(&mem_arena);
        return EXIT_FAILURE;
    }

    cJSON_Delete(cj);
    CleanMemArena(&mem_arena);

    return EXIT_SUCCESS;
}
