#include "zap_packing.h"

#include <cJSON.h>

namespace zf {
    constexpr t_size g_per_asset_mem_arena_size = Megabytes(8);

    enum e_asset_type : t_s32 {
        ek_asset_type_texture,
        ek_asset_type_font,
        ek_asset_type_sound,

        eks_asset_type_cnt
    };

    constexpr s_static_array<s_str_rdonly, eks_asset_type_cnt> g_asset_type_arr_names = {{
        "textures",
        "fonts",
        "sounds"
    }};

    enum e_asset_field_type : t_s32 {
        ek_asset_field_type_str,
        ek_asset_field_type_num
    };

    struct s_asset_field {
        s_str_rdonly name;
        e_asset_field_type type;
    };

    enum e_tex_field : t_s32 {
        ek_tex_field_src_file_path,
        ek_tex_field_dest_file_path,

        eks_tex_field_cnt
    };

    constexpr s_static_array<s_asset_field, eks_tex_field_cnt> g_tex_fields = {{
        {"src_file_path", ek_asset_field_type_str},
        {"dest_file_path", ek_asset_field_type_str}
    }};

    enum e_font_field : t_s32 {
        ek_font_field_src_file_path,
        ek_font_field_height,
        ek_font_field_dest_file_path,

        eks_font_field_cnt
    };

    constexpr s_static_array<s_asset_field, eks_font_field_cnt> g_font_fields = {{
        {"src_file_path", ek_asset_field_type_str},
        {"height", ek_asset_field_type_num},
        {"dest_file_path", ek_asset_field_type_str}
    }};

    enum e_snd_field : t_s32 {
        ek_snd_field_src_file_path,
        ek_snd_field_dest_file_path,

        eks_snd_field_cnt
    };

    constexpr s_static_array<s_asset_field, eks_snd_field_cnt> g_snd_fields = {{
        {"src_file_path", ek_asset_field_type_str},
        {"dest_file_path", ek_asset_field_type_str}
    }};

    t_b8 PackAssets(const s_str_rdonly instrs_json, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(instrs_json));

        ZF_DEFER_MEM_ARENA_REWIND(temp_mem_arena);

        cJSON* const cj = cJSON_Parse(StrRaw(instrs_json));

        if (!cj) {
            ZF_LOG_ERROR_SPECIAL("cJSON", "%s", cJSON_GetErrorPtr());
            return false;
        }

        ZF_DEFER({ cJSON_Delete(cj); });

        if (!cJSON_IsObject(cj)) {
            return false;
        }

        s_mem_arena per_asset_mem_arena;

        if (!AllocMemArena(g_per_asset_mem_arena_size, per_asset_mem_arena)) {
            return false;
        }

        ZF_DEFER({ FreeMemArena(per_asset_mem_arena); });

        s_static_array<cJSON*, eks_tex_field_cnt> tex_field_cj_ptrs = {};
        s_static_array<cJSON*, eks_font_field_cnt> font_field_cj_ptrs = {};
        s_static_array<cJSON*, eks_snd_field_cnt> snd_field_cj_ptrs = {};

        for (t_size asset_type_index = 0; asset_type_index < eks_asset_type_cnt; asset_type_index++) {
            ZF_DEFER_MEM_ARENA_REWIND(per_asset_mem_arena);

            cJSON* const cj_assets = cJSON_GetObjectItemCaseSensitive(cj, StrRaw(g_asset_type_arr_names[asset_type_index]));

            if (!cJSON_IsArray(cj_assets)) {
                return false;
            }

            cJSON* cj_asset = nullptr;

            cJSON_ArrayForEach(cj_asset, cj_assets) {
                if (!cJSON_IsObject(cj_asset)) {
                    continue;
                }

                const auto fields = [asset_type_index]() -> s_array_rdonly<s_asset_field> {
                    switch (asset_type_index) {
                        case ek_asset_type_texture:
                            return g_tex_fields;

                        case ek_asset_type_font:
                            return g_font_fields;

                        case ek_asset_type_sound:
                            return g_snd_fields;
                    }

                    return {};
                }();

                const auto field_vals = [asset_type_index, &tex_field_cj_ptrs, &font_field_cj_ptrs, &snd_field_cj_ptrs]() -> s_array<cJSON*> {
                    switch (asset_type_index) {
                        case ek_asset_type_texture:
                            return tex_field_cj_ptrs;

                        case ek_asset_type_font:
                            return font_field_cj_ptrs;

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
                            case ek_asset_field_type_str:
                                return cJSON_IsString(field_vals[fi]);

                            case ek_asset_field_type_num:
                                return cJSON_IsNumber(field_vals[fi]);
                        }

                        return false;
                    }();

                    if (!is_valid) {
                        return false;
                    }
                }

                // Perform different packing for each asset type.
                switch (asset_type_index) {
                    case ek_asset_type_texture:
                        {
                            const auto src_fp_raw = field_vals[ek_tex_field_src_file_path]->valuestring;
                            const auto dest_fp_raw = field_vals[ek_tex_field_dest_file_path]->valuestring;

                            s_rgba_texture_data tex_data;

                            if (!LoadRGBATextureDataFromRaw(StrFromRawTerminated(src_fp_raw), temp_mem_arena, tex_data)) {
                                return false;
                            }

                            if (!PackTexture(tex_data, StrFromRawTerminated(dest_fp_raw), temp_mem_arena)) {
                                return false;
                            }
                        }

                        break;

                    case ek_asset_type_font:
                        {
                            const auto src_fp_raw = field_vals[ek_font_field_src_file_path]->valuestring;
                            const auto height = field_vals[ek_font_field_height]->valueint;
                            const auto dest_fp_raw = field_vals[ek_font_field_dest_file_path]->valuestring;

                            s_font font;

                            const s_static_array<t_s32, 4> codepoints = {
                                {'a', 'b', 'c', 'd'}
                            };

                            if (!LoadFontFromRaw(per_asset_mem_arena, StrFromRawTerminated(src_fp_raw), height, codepoints, temp_mem_arena, font)) {
                                return false;
                            }

                            if (!PackFont(font, StrFromRawTerminated(dest_fp_raw), temp_mem_arena)) {
                                return false;
                            }
                        }

                        break;

                    case ek_asset_type_sound:
                        {
                            const auto src_fp_raw = field_vals[ek_snd_field_src_file_path]->valuestring;
                            const auto dest_fp_raw = field_vals[ek_snd_field_dest_file_path]->valuestring;

                            s_sound_data snd_data;

                            if (!LoadSoundFromRaw(StrFromRawTerminated(src_fp_raw), temp_mem_arena, snd_data.meta, snd_data.pcm)) {
                                return false;
                            }

                            if (!PackSound(snd_data, StrFromRawTerminated(dest_fp_raw), temp_mem_arena)) {
                                return false;
                            }
                        }

                        break;
                }
            }
        }

        return true;
    }
}
