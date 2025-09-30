#include "zap_packing.h"

#include <cJSON.h>
#include "zc_gfx.h"

namespace zf {
    struct s_packed_header {
        t_s32 tex_cnt;
        t_s32 font_cnt;
        t_s32 shader_prog_cnt;
    };

    static bool PackTexturesFromInstrs(c_file_writer& fw, cJSON* const cj, c_mem_arena& temp_mem_arena) {
        cJSON* const cj_textures = cJSON_GetObjectItemCaseSensitive(cj, "textures");

        if (!cJSON_IsArray(cj_textures)) {
            return false;
        }

        cJSON* cj_tex = nullptr;

        cJSON_ArrayForEach(cj_tex, cj_textures) {
            if (!cJSON_IsObject(cj_tex)) {
                continue;
            }

            const cJSON* const cj_file_path = cJSON_GetObjectItem(cj_tex, "file_path");

            if (!cJSON_IsString(cj_file_path)) {
                return false;
            }

            const auto temp_mem_arena_offs_old = temp_mem_arena.Offs();

            s_rgba_texture rgba_tex;

            if (!LoadRGBATextureFromRawFile(rgba_tex, temp_mem_arena, cj_file_path->valuestring)) {
                return false;
            }

            if (!PackTexture(fw, rgba_tex)) {
                return false;
            }

            temp_mem_arena.Rewind(temp_mem_arena_offs_old);
        }

        return true;
    }

    static bool PackFontsFromInstrs(c_file_writer& fw, cJSON* const cj, c_mem_arena& temp_mem_arena) {
        cJSON* const cj_fonts = cJSON_GetObjectItemCaseSensitive(cj, "fonts");

        if (!cJSON_IsArray(cj_fonts)) {
            return false;
        }

        cJSON* cj_font = nullptr;

        cJSON_ArrayForEach(cj_font, cj_fonts) {
            if (!cJSON_IsObject(cj_font)) {
                continue;
            }

            const cJSON* const cj_file_path = cJSON_GetObjectItem(cj_font, "file_path");
            const cJSON* const cj_height = cJSON_GetObjectItem(cj_font, "height");

            if (!cJSON_IsString(cj_file_path) || !cJSON_IsNumber(cj_height)) {
                return false;
            }

            const auto temp_mem_arena_offs_old = temp_mem_arena.Offs();

            s_font_arrangement arrangement;
            s_font_texture_meta tex_meta;
            c_array<const t_u8> tex_rbga_px_data;

            if (!LoadFontFromRawFile(arrangement, tex_meta, tex_rbga_px_data, cj_file_path->valuestring, cj_height->valueint, temp_mem_arena)) {
                return false;
            }

            temp_mem_arena.Rewind(temp_mem_arena_offs_old);
        }

        return true;
    }

    bool PackAssets(const c_string_view instrs_json, const c_string_view output_file_path, c_mem_arena& temp_mem_arena) {
        bool success = true;

        cJSON* const cj = cJSON_Parse(instrs_json.Raw());

        c_file_writer fw;

        if (!cj) {
            ZF_LOG_ERROR_SPECIAL("cJSON", "%s", cJSON_GetErrorPtr());
            success = false;
            goto out_a;
        }

        if (!cJSON_IsObject(cj)) {
            success = false;
            goto out_b;
        }

        if (!fw.Open(output_file_path)) {
            success = false;
            goto out_b;
        }

        if (!PackTexturesFromInstrs(fw, cj, temp_mem_arena)) {
            success = false;
            goto out_c;
        }

        if (!PackFontsFromInstrs(fw, cj, temp_mem_arena)) {
            success = false;
            goto out_c;
        }

out_c:
        fw.Close();

out_b:
        cJSON_Delete(cj);

out_a:
        return success;
    }
}
