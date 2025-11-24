#include "zap_packing.h"

#include <cJSON.h>

namespace zf {
    constexpr t_size g_mem_arena_size = Megabytes(8);

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

    t_b8 RunPacker(const s_str_rdonly instrs_json_file_path) {
        ZF_ASSERT(IsStrTerminated(instrs_json_file_path));

        zf::s_mem_arena mem_arena;

        if (!zf::AllocMemArena(g_mem_arena_size, mem_arena)) {
            ZF_LOG_ERROR("Failed to allocate memory arena!");
            return false;
        }

        ZF_DEFER({ zf::FreeMemArena(mem_arena); });

        zf::s_str instrs_json;

        if (!zf::LoadFileContentsAsStr(mem_arena, instrs_json_file_path, instrs_json)) {
            ZF_LOG_ERROR("Failed to load packing instructions JSON file \"%s\"!", StrRaw(instrs_json_file_path));
            return false;
        }

        cJSON* const cj = cJSON_Parse(StrRaw(instrs_json));

        if (!cj) {
            ZF_LOG_ERROR("Failed to parse packing instructions JSON file \"%s\"!", StrRaw(instrs_json_file_path));
            return false;
        }

        ZF_DEFER({ cJSON_Delete(cj); });

        if (!cJSON_IsObject(cj)) {
            ZF_LOG_ERROR("Packing instructions JSON root is not an object!");
            return false;
        }

        s_static_array<cJSON*, eks_tex_field_cnt> tex_field_cj_ptrs = {};
        s_static_array<cJSON*, eks_font_field_cnt> font_field_cj_ptrs = {};
        s_static_array<cJSON*, eks_snd_field_cnt> snd_field_cj_ptrs = {};

        for (t_size asset_type_index = 0; asset_type_index < eks_asset_type_cnt; asset_type_index++) {
            cJSON* const cj_assets = cJSON_GetObjectItemCaseSensitive(cj, StrRaw(g_asset_type_arr_names[asset_type_index]));

            if (!cJSON_IsArray(cj_assets)) {
                ZF_LOG_ERROR("Packing instructions JSON \"%s\" array does not exist or it is of the wrong type!", StrRaw(g_asset_type_arr_names[asset_type_index]));
                return false;
            }

            cJSON* cj_asset = nullptr;

            cJSON_ArrayForEach(cj_asset, cj_assets) {
                RewindMemArena(mem_arena, 0);

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
                        ZF_LOG_ERROR("Packing instructions JSON \"%s\" entry is missing required field \"%s\" or it is of the wrong type!", StrRaw(g_asset_type_arr_names[asset_type_index]), StrRaw(fields[fi].name));
                        return false;
                    }
                }

                switch (asset_type_index) {
                    case ek_asset_type_texture:
                        {
                            const auto dest_fp_raw = field_vals[ek_tex_field_dest_file_path]->valuestring;
                            const auto src_fp_raw = field_vals[ek_tex_field_src_file_path]->valuestring;

                            if (!PackTexture(StrFromRawTerminated(dest_fp_raw), StrFromRawTerminated(src_fp_raw), mem_arena)) {
                                ZF_LOG_ERROR("Failed to pack texture \"%s\" to \"%s\"!", StrRaw(StrFromRawTerminated(src_fp_raw)), StrRaw(StrFromRawTerminated(dest_fp_raw)));
                                return false;
                            }
                        }

                        break;

                    case ek_asset_type_font:
                        {
                            const auto dest_fp_raw = field_vals[ek_font_field_dest_file_path]->valuestring;
                            const auto src_fp_raw = field_vals[ek_font_field_src_file_path]->valuestring;
                            const auto height = field_vals[ek_font_field_height]->valueint;

                            const s_static_array<t_s32, 4> codepoints = {
                                {'a', 'b', 'c', 'd'}
                            };

                            if (!PackFont(StrFromRawTerminated(dest_fp_raw), StrFromRawTerminated(src_fp_raw), height, codepoints, mem_arena)) {
                                ZF_LOG_ERROR("Failed to pack font \"%s\" to \"%s\"!", StrRaw(StrFromRawTerminated(src_fp_raw)), StrRaw(StrFromRawTerminated(dest_fp_raw)));
                                return false;
                            }
                        }

                        break;

                    case ek_asset_type_sound:
                        {
                            const auto dest_fp_raw = field_vals[ek_snd_field_dest_file_path]->valuestring;
                            const auto src_fp_raw = field_vals[ek_snd_field_src_file_path]->valuestring;

                            if (!PackSound(StrFromRawTerminated(dest_fp_raw), StrFromRawTerminated(src_fp_raw), mem_arena)) {
                                ZF_LOG_ERROR("Failed to pack sound \"%s\" to \"%s\"!", StrRaw(StrFromRawTerminated(src_fp_raw)), StrRaw(StrFromRawTerminated(dest_fp_raw)));
                                return false;
                            }
                        }

                        break;
                }
            }
        }

        ZF_LOG_SUCCESS("Asset packing completed!");

        return true;
    }
}
