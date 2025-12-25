#include "zap_packing.h"

#include <cJSON.h>

namespace zf {
    enum e_asset_type : t_i32 {
        ek_asset_type_texture,
        ek_asset_type_font,
        ek_asset_type_sound,

        eks_asset_type_cnt
    };

    constexpr s_static_array<s_cstr_literal, eks_asset_type_cnt> g_asset_type_arr_names = {
        "textures",
        "fonts",
        "sounds",
    };

    enum e_asset_field_type : t_i32 {
        ek_asset_field_type_str,
        ek_asset_field_type_num,

        eks_asset_field_type_cnt
    };

    constexpr s_static_array<s_cstr_literal, eks_asset_field_type_cnt> g_asset_field_type_names = {
        "string",
        "number",
    };

    struct s_asset_field {
        s_cstr_literal name;
        e_asset_field_type type = {};
        t_b8 optional = false;
    };

    enum e_tex_field : t_i32 {
        ek_tex_field_src_file_path,
        ek_tex_field_dest_file_path,

        eks_tex_field_cnt
    };

    constexpr s_static_array<s_asset_field, eks_tex_field_cnt> g_tex_fields = {
        {"src_file_path", ek_asset_field_type_str},
        {"dest_file_path", ek_asset_field_type_str},
    };

    enum e_font_field : t_i32 {
        ek_font_field_src_file_path,
        ek_font_field_height,
        ek_font_field_extra_chrs_file_path,
        ek_font_field_dest_file_path,

        eks_font_field_cnt
    };

    constexpr s_static_array<s_asset_field, eks_font_field_cnt> g_font_fields = {
        {"src_file_path", ek_asset_field_type_str},
        {"height", ek_asset_field_type_num},
        {"extra_chrs_file_path", ek_asset_field_type_str, true},
        {"dest_file_path", ek_asset_field_type_str},
    };

    enum e_snd_field : t_i32 {
        ek_snd_field_src_file_path,
        ek_snd_field_dest_file_path,

        eks_snd_field_cnt
    };

    constexpr static s_static_array<s_asset_field, eks_snd_field_cnt> g_snd_fields = {
        {"src_file_path", ek_asset_field_type_str},
        {"dest_file_path", ek_asset_field_type_str},
    };

    t_b8 RunPacker(const s_str_rdonly instrs_json_file_path) {
        if (!Test()) {
            return false;
        }

        s_mem_arena mem_arena = {};
        ZF_DEFER({ mem_arena.Release(); });

        cJSON *cj = nullptr;

        {
            s_array<t_u8> instrs_json_file_contents; // Not needed beyond this scope.

            if (!LoadFileContents(instrs_json_file_path, mem_arena, mem_arena, instrs_json_file_contents, true)) {
                LogError(s_cstr_literal("Failed to load packing instructions JSON file \"%\"!"), instrs_json_file_path);
                return false;
            }

            cj = cJSON_Parse(s_str(instrs_json_file_contents).AsCstr());

            if (!cj) {
                LogError(s_cstr_literal("Failed to parse packing instructions JSON file!"));
                return false;
            }
        }

        ZF_DEFER({ cJSON_Delete(cj); });

        if (!cJSON_IsObject(cj)) {
            LogError(s_cstr_literal("Packing instructions JSON root is not an object!"));
            return false;
        }

        s_static_array<cJSON *, eks_tex_field_cnt> tex_field_cj_ptrs;
        s_static_array<cJSON *, eks_font_field_cnt> font_field_cj_ptrs;
        s_static_array<cJSON *, eks_snd_field_cnt> snd_field_cj_ptrs;

        for (t_i32 asset_type_index = 0; asset_type_index < eks_asset_type_cnt; asset_type_index++) {
            const auto asset_type_arr_name = g_asset_type_arr_names[asset_type_index];

            cJSON *const cj_assets = cJSON_GetObjectItemCaseSensitive(cj, asset_type_arr_name.BufPtr());

            if (!cJSON_IsArray(cj_assets)) {
                LogError(s_cstr_literal("Packing instructions JSON \"%\" array does not exist or it is of the wrong type!"), asset_type_arr_name);
                return false;
            }

            cJSON *cj_asset = nullptr;

            cJSON_ArrayForEach(cj_asset, cj_assets) {
                mem_arena.Reset();

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

                const auto field_vals = [asset_type_index, &tex_field_cj_ptrs, &font_field_cj_ptrs, &snd_field_cj_ptrs]() -> s_array<cJSON *> {
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

                for (t_i32 fi = 0; fi < fields.Len(); fi++) {
                    const auto field_name = fields[fi].name;

                    field_vals[fi] = cJSON_GetObjectItem(cj_asset, field_name.BufPtr());

                    if (!field_vals[fi]) {
                        if (fields[fi].optional) {
                            continue;
                        }

                        LogError(s_cstr_literal("A packing instructions JSON \"%\" entry is missing required field \"%\"!"), asset_type_arr_name, field_name);
                        return false;
                    }

                    const auto is_valid = [fi, fields, field_vals]() -> t_b8 {
                        switch (fields[fi].type) {
                        case ek_asset_field_type_str:
                            return cJSON_IsString(field_vals[fi]);

                        case ek_asset_field_type_num:
                            return cJSON_IsNumber(field_vals[fi]);

                        case eks_asset_field_type_cnt:
                            ZF_FATAL();
                        }

                        ZF_FATAL();
                    }();

                    if (!is_valid) {
                        LogError(s_cstr_literal("A packing instructions JSON \"%\" entry has field \"%\" as the wrong type! Expected a %."), asset_type_arr_name, field_name, g_asset_field_type_names[fields[fi].type]);
                        return false;
                    }
                }

                switch (asset_type_index) {
                case ek_asset_type_texture: {
                    const auto dest_fp_cstr = field_vals[ek_tex_field_dest_file_path]->valuestring;
                    const auto dest_fp = ConvertCstr(dest_fp_cstr);

                    const auto src_fp_cstr = field_vals[ek_tex_field_src_file_path]->valuestring;
                    const auto src_fp = ConvertCstr(src_fp_cstr);

                    s_texture_data tex_data;

                    if (!LoadTextureFromRaw(src_fp, mem_arena, mem_arena, tex_data)) {
                        LogError(s_cstr_literal("Failed to load texture from raw file \"%\"!"), src_fp);
                        return false;
                    }

                    if (!PackTexture(dest_fp, tex_data, mem_arena)) {
                        LogError(s_cstr_literal("Failed to pack texture to file \"%\"!"), dest_fp);
                        return false;
                    }

                    break;
                }

                case ek_asset_type_font: {
                    const auto dest_fp = ConvertCstr(field_vals[ek_font_field_dest_file_path]->valuestring);
                    const auto src_fp = ConvertCstr(field_vals[ek_font_field_src_file_path]->valuestring);
                    const auto height = field_vals[ek_font_field_height]->valueint;

                    auto &code_pt_bv = Alloc<t_code_pt_bit_vec>(mem_arena);

                    SetBitsInRange(code_pt_bv, g_printable_ascii_range_begin, g_printable_ascii_range_end); // Add the printable ASCII range as a default.

                    if (field_vals[ek_font_field_extra_chrs_file_path]) {
                        const auto extra_chrs_fp = ConvertCstr(field_vals[ek_font_field_extra_chrs_file_path]->valuestring);

                        s_array<t_u8> extra_chrs_file_contents;

                        if (!LoadFileContents(extra_chrs_fp, mem_arena, mem_arena, extra_chrs_file_contents)) {
                            LogError(s_cstr_literal("Failed to load extra characters file \"%\"!"), extra_chrs_fp);
                            return false;
                        }

                        MarkStrCodePoints({extra_chrs_file_contents}, code_pt_bv);
                    }

                    if (!PackFont(dest_fp, src_fp, height, code_pt_bv, mem_arena)) {
                        LogError(s_cstr_literal("Failed to pack font to file \"%\"!"), dest_fp);
                        return false;
                    }

                    break;
                }

                case ek_asset_type_sound: {
                    const auto dest_fp = ConvertCstr(field_vals[ek_snd_field_dest_file_path]->valuestring);
                    const auto src_fp = ConvertCstr(field_vals[ek_snd_field_src_file_path]->valuestring);

                    s_sound_data snd_data;

                    if (!LoadSoundFromRaw(src_fp, mem_arena, mem_arena, snd_data)) {
                        LogError(s_cstr_literal("Failed to load sound from raw file \"%\"!"), src_fp);
                        return false;
                    }

                    if (!PackSound(dest_fp, snd_data, mem_arena)) {
                        LogError(s_cstr_literal("Failed to pack sound to file \"%\"!"), dest_fp);
                        return false;
                    }

                    break;
                }
                }
            }
        }

        Log(s_cstr_literal("Asset packing completed!"));

        return true;
    }
}
