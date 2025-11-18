#include "zap_packing.h"

#include <cJSON.h>

namespace zf {
    enum e_asset_type {
        ek_asset_type_texture,
        ek_asset_type_sound,

        eks_asset_type_cnt
    };

    constexpr s_static_array<s_str_rdonly, eks_asset_type_cnt> g_asset_type_arr_names = {
        "textures",
        "sounds"
    };

    enum class ec_asset_field_type {
        str,
        num
    };

    struct s_asset_field {
        s_str_rdonly name;
        ec_asset_field_type type;
    };

    enum e_tex_field {
        ek_tex_field_src_file_path,
        ek_tex_field_dest_file_path,
        eks_tex_field_cnt
    };

    constexpr s_static_array<s_asset_field, eks_tex_field_cnt> g_tex_fields = {
        {"src_file_path", ec_asset_field_type::str},
        {"dest_file_path", ec_asset_field_type::str}
    };

    enum e_snd_field {
        ek_snd_field_src_file_path,
        ek_snd_field_dest_file_path,
        eks_snd_field_cnt
    };

    constexpr s_static_array<s_asset_field, eks_snd_field_cnt> g_snd_fields = {{
        {"src_file_path", ec_asset_field_type::str},
        {"dest_file_path", ec_asset_field_type::str}
    }};

    static t_b8 PackAssetsFromInstrs(cJSON* const cj, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(cj);

        s_static_array<cJSON*, eks_tex_field_cnt> tex_field_cj_ptrs = {};
        s_static_array<cJSON*, eks_snd_field_cnt> snd_field_cj_ptrs = {};

        for (t_size asset_type_index = 0; asset_type_index < eks_asset_type_cnt; asset_type_index++) {
            cJSON* const cj_assets = cJSON_GetObjectItemCaseSensitive(cj, StrRaw(g_asset_type_arr_names[asset_type_index]));

            if (!cJSON_IsArray(cj_assets)) {
                ZF_LOG_ERROR("No \"%s\" array found!", StrRaw(g_asset_type_arr_names[asset_type_index]));
                return false;
            }

            cJSON* cj_asset = nullptr;

            cJSON_ArrayForEach(cj_asset, cj_assets) {
                if (!cJSON_IsObject(cj_asset)) {
                    ZF_LOG_ERROR("Invalid entry!");
                    continue;
                }

                const auto fields = [asset_type_index]() -> s_array<const s_asset_field> {
                    switch (asset_type_index) {
                        case ek_asset_type_texture:
                            return g_tex_fields;

                        case ek_asset_type_sound:
                            return g_snd_fields;
                    }

                    return {};
                }();

                const auto field_vals = [asset_type_index, &tex_field_cj_ptrs, &snd_field_cj_ptrs]() -> s_array<cJSON*> {
                    switch (asset_type_index) {
                        case ek_asset_type_texture:
                            return tex_field_cj_ptrs;

                        case ek_asset_type_sound:
                            return snd_field_cj_ptrs;
                    }

                    return {};
                }();

                // Get each field.
                for (t_size fi = 0; fi < fields.len; fi++) {
                    field_vals[fi] = cJSON_GetObjectItem(cj_asset, StrRaw(fields[fi].name));

                    const auto is_valid = [fi, fields, field_vals]() -> t_b8 {
                        switch (fields[fi].type) {
                            case ec_asset_field_type::str:
                                return cJSON_IsString(field_vals[fi]);

                            case ec_asset_field_type::num:
                                return cJSON_IsNumber(field_vals[fi]);
                        }

                        return false;
                    }();

                    if (!is_valid) {
                        ZF_LOG_ERROR("Found a \"%s\" field missing or invalid in \"%s\"!", StrRaw(fields[fi].name), StrRaw(g_asset_type_arr_names[asset_type_index]));
                        return false;
                    }
                }

                // Perform different packing for each asset type.
                const auto temp_mem_arena_offs_old = temp_mem_arena.offs;

                switch (asset_type_index) {
                    case ek_asset_type_texture:
                        {
                            s_rgba_texture_data tex_data;

                            const auto src_fp_raw = field_vals[ek_tex_field_src_file_path]->valuestring;
                            const auto dest_fp_raw = field_vals[ek_tex_field_dest_file_path]->valuestring;

                            if (!LoadRGBATextureDataFromRaw(StrFromRawTerminated(src_fp_raw), temp_mem_arena, tex_data)) {
                                ZF_LOG_ERROR("Failed to load RGBA texture from file \"%s\"!", src_fp_raw);
                                return false;
                            }

                            if (!PackTexture(tex_data, StrFromRawTerminated(dest_fp_raw), temp_mem_arena)) {
                                ZF_LOG_ERROR("Failed to pack texture from file \"%s\"!", src_fp_raw);
                                return false;
                            }

                            ZF_LOG_SUCCESS("Packed texture from file \"%s\"!", src_fp_raw);
                        }

                        break;

                    case ek_asset_type_sound:
                        {
                            s_sound_data snd_data;

                            const auto src_fp_raw = field_vals[ek_snd_field_src_file_path]->valuestring;
                            const auto dest_fp_raw = field_vals[ek_snd_field_dest_file_path]->valuestring;

                            if (!LoadSoundFromRaw(StrFromRawTerminated(src_fp_raw), temp_mem_arena, snd_data)) {
                                ZF_LOG_ERROR("Failed to load sound from file \"%s\"!", src_fp_raw);
                                return false;
                            }

                            if (!PackSound(snd_data, StrFromRawTerminated(dest_fp_raw), temp_mem_arena)) {
                                ZF_LOG_ERROR("Failed to pack sound from file \"%s\"!", src_fp_raw);
                                return false;
                            }

                            ZF_LOG_SUCCESS("Packed sound from file \"%s\"!", src_fp_raw);
                        }

                        break;
                }

                RewindMemArena(temp_mem_arena, temp_mem_arena_offs_old);
            }
        }

        return true;
    }

    t_b8 PackAssets(const s_str_rdonly instrs_json, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(instrs_json));

        cJSON* const cj = cJSON_Parse(StrRaw(instrs_json));

        if (!cj) {
            ZF_LOG_ERROR_SPECIAL("cJSON", "%s", cJSON_GetErrorPtr());
            return false;
        }

        const t_b8 success = [cj, &temp_mem_arena]() {
            if (!cJSON_IsObject(cj)) {
                ZF_LOG_ERROR("Invalid JSON structure in asset packing instructions!");
                return false;
            }

            if (!PackAssetsFromInstrs(cj, temp_mem_arena)) {
                ZF_LOG_ERROR("Failed to pack assets!");
                return false;
            }

            return true;
        }();

        cJSON_Delete(cj);

        return success;
    }
}
