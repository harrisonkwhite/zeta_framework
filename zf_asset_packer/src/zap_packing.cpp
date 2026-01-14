#include "zap.h"

#include <cJSON.h>

enum t_asset_type : zcl::t_i32 {
    ek_asset_type_texture,
    ek_asset_type_font,
    ek_asset_type_shader,
    ek_asset_type_sound,

    ekm_asset_type_cnt
};

constexpr zcl::t_static_array<const char *, ekm_asset_type_cnt> k_asset_type_array_name_c_strs = {{
    "textures",
    "fonts",
    "shaders",
    "sounds",
}};

enum t_asset_field_type : zcl::t_i32 {
    ek_asset_field_type_str,
    ek_asset_field_type_num,

    ekm_asset_field_type_cnt
};

constexpr zcl::t_static_array<const char *, ekm_asset_field_type_cnt> k_asset_field_type_name_c_strs = {{
    "string",
    "number",
}};

struct t_asset_field {
    const char *name_c_str;
    t_asset_field_type type;
    zcl::t_b8 optional;
};

enum t_texture_field : zcl::t_i32 {
    ek_texture_field_file_path,
    ek_texture_field_out_file_path,

    ekm_texture_field_cnt
};

constexpr zcl::t_static_array<t_asset_field, ekm_texture_field_cnt> k_texture_fields = {{
    {.name_c_str = "file_path", .type = ek_asset_field_type_str},
    {.name_c_str = "out_file_path", .type = ek_asset_field_type_str},
}};

enum t_font_field : zcl::t_i32 {
    ek_font_field_file_path,
    ek_font_field_height,
    ek_font_field_extra_chrs_file_path,
    ek_font_field_out_file_path,

    ekm_font_field_cnt
};

constexpr zcl::t_static_array<t_asset_field, ekm_font_field_cnt> k_font_fields = {{
    {.name_c_str = "file_path", .type = ek_asset_field_type_str},
    {.name_c_str = "height", .type = ek_asset_field_type_num},
    {.name_c_str = "extra_chrs_file_path", .type = ek_asset_field_type_str, .optional = true},
    {.name_c_str = "out_file_path", .type = ek_asset_field_type_str},
}};

enum t_shader_field : zcl::t_i32 {
    ek_shader_field_file_path,
    ek_shader_field_type,
    ek_shader_field_varying_def_file_path,
    ek_shader_field_out_file_path,

    ekm_shader_field_cnt
};

constexpr zcl::t_static_array<t_asset_field, ekm_shader_field_cnt> k_shader_fields = {{
    {.name_c_str = "file_path", .type = ek_asset_field_type_str},
    {.name_c_str = "type", .type = ek_asset_field_type_str},
    {.name_c_str = "varying_def_file_path", .type = ek_asset_field_type_str},
    {.name_c_str = "out_file_path", .type = ek_asset_field_type_str},
}};

enum t_sound_field : zcl::t_i32 {
    ek_sound_field_file_path,
    ek_sound_field_out_file_path,

    ekm_sound_field_cnt
};

constexpr zcl::t_static_array<t_asset_field, ekm_sound_field_cnt> k_sound_fields = {{
    {.name_c_str = "file_path", .type = ek_asset_field_type_str},
    {.name_c_str = "out_file_path", .type = ek_asset_field_type_str},
}};

zcl::t_b8 pack_assets(const zcl::t_str_rdonly instrs_json_file_path) {
    zcl::t_arena arena = zcl::arena_create_blockbased();
    ZCL_DEFER({ zcl::arena_destroy(&arena); });

    cJSON *cj;

    {
        zcl::t_array_mut<zcl::t_u8> instrs_json_file_contents; // Not needed beyond this scope.

        if (!zcl::FileLoadContents(instrs_json_file_path, &arena, &arena, &instrs_json_file_contents, true)) {
            zcl::LogError(ZCL_STR_LITERAL("Failed to load packing instructions JSON file \"%\"!"), instrs_json_file_path);
            return false;
        }

        cj = cJSON_Parse(zcl::StrToCStr(zcl::t_str_rdonly{instrs_json_file_contents}));

        if (!cj) {
            zcl::LogError(ZCL_STR_LITERAL("Failed to parse packing instructions JSON file!"));
            return false;
        }
    }

    ZCL_DEFER({ cJSON_Delete(cj); });

    if (!cJSON_IsObject(cj)) {
        zcl::LogError(ZCL_STR_LITERAL("Packing instructions JSON root is not an object!"));
        return false;
    }

    zcl::t_static_array<cJSON *, ekm_texture_field_cnt> texture_field_cj_ptrs = {};
    zcl::t_static_array<cJSON *, ekm_font_field_cnt> font_field_cj_ptrs = {};
    zcl::t_static_array<cJSON *, ekm_shader_field_cnt> shader_field_cj_ptrs = {};
    zcl::t_static_array<cJSON *, ekm_sound_field_cnt> snd_field_cj_ptrs = {};

    for (zcl::t_i32 asset_type_index = 0; asset_type_index < ekm_asset_type_cnt; asset_type_index++) {
        const auto asset_type_arr_name_c_str = k_asset_type_array_name_c_strs[asset_type_index];

        cJSON *const cj_assets = cJSON_GetObjectItemCaseSensitive(cj, asset_type_arr_name_c_str);

        if (!cJSON_IsArray(cj_assets)) {
            zcl::LogError(ZCL_STR_LITERAL("Packing instructions JSON \"%\" array does not exist or it is of the wrong type!"), zcl::CStrToStr(asset_type_arr_name_c_str));
            return false;
        }

        cJSON *cj_asset;

        cJSON_ArrayForEach(cj_asset, cj_assets) {
            zcl::arena_rewind(&arena);

            if (!cJSON_IsObject(cj_asset)) {
                continue;
            }

            const auto fields = [asset_type_index]() -> zcl::t_array_rdonly<t_asset_field> {
                switch (asset_type_index) {
                case ek_asset_type_texture: return array_to_nonstatic(&k_texture_fields);
                case ek_asset_type_font: return array_to_nonstatic(&k_font_fields);
                case ek_asset_type_shader: return array_to_nonstatic(&k_shader_fields);
                case ek_asset_type_sound: return array_to_nonstatic(&k_sound_fields);
                }

                return {};
            }();

            const auto field_vals = [asset_type_index, &texture_field_cj_ptrs, &font_field_cj_ptrs, &shader_field_cj_ptrs, &snd_field_cj_ptrs]() -> zcl::t_array_mut<cJSON *> {
                switch (asset_type_index) {
                case ek_asset_type_texture: return array_to_nonstatic(&texture_field_cj_ptrs);
                case ek_asset_type_font: return array_to_nonstatic(&font_field_cj_ptrs);
                case ek_asset_type_shader: return array_to_nonstatic(&shader_field_cj_ptrs);
                case ek_asset_type_sound: return array_to_nonstatic(&snd_field_cj_ptrs);
                }

                return {};
            }();

            for (zcl::t_i32 fi = 0; fi < fields.len; fi++) {
                const auto field_name_c_str = fields[fi].name_c_str;

                field_vals[fi] = cJSON_GetObjectItem(cj_asset, field_name_c_str);

                if (!field_vals[fi]) {
                    if (fields[fi].optional) {
                        continue;
                    }

                    zcl::LogError(ZCL_STR_LITERAL("A packing instructions JSON \"%\" entry is missing required field \"%\"!"), zcl::CStrToStr(asset_type_arr_name_c_str), zcl::CStrToStr(field_name_c_str));

                    return false;
                }

                const auto is_valid = [fi, fields, field_vals]() -> zcl::t_b8 {
                    switch (fields[fi].type) {
                    case ek_asset_field_type_str:
                        return cJSON_IsString(field_vals[fi]);

                    case ek_asset_field_type_num:
                        return cJSON_IsNumber(field_vals[fi]);

                    case ekm_asset_field_type_cnt:
                        ZCL_UNREACHABLE();
                    }

                    ZCL_UNREACHABLE();
                }();

                if (!is_valid) {
                    zcl::LogError(ZCL_STR_LITERAL("A packing instructions JSON \"%\" entry has field \"%\" as the wrong type! Expected a %."), zcl::CStrToStr(asset_type_arr_name_c_str), zcl::CStrToStr(field_name_c_str), zcl::CStrToStr(k_asset_field_type_name_c_strs[fields[fi].type]));
                    return false;
                }
            }

            switch (asset_type_index) {
            case ek_asset_type_texture: {
                const auto file_path = zcl::CStrToStr(field_vals[ek_texture_field_file_path]->valuestring);
                const auto out_file_path = zcl::CStrToStr(field_vals[ek_texture_field_out_file_path]->valuestring);

                zcl::t_texture_data_mut texture_data;

                if (!zcl::texture_load_from_raw(file_path, &arena, &arena, &texture_data)) {
                    zcl::LogError(ZCL_STR_LITERAL("Failed to load texture from file \"%\"!"), file_path);
                    return false;
                }

                if (!zcl::texture_pack(out_file_path, texture_data, &arena)) {
                    zcl::LogError(ZCL_STR_LITERAL("Failed to pack texture to file \"%\"!"), out_file_path);
                    return false;
                }

                break;
            }

            case ek_asset_type_font: {
                const auto file_path = zcl::CStrToStr(field_vals[ek_font_field_file_path]->valuestring);
                const auto height = field_vals[ek_font_field_height]->valueint;
                const auto out_file_path = zcl::CStrToStr(field_vals[ek_font_field_out_file_path]->valuestring);

                const auto code_pt_bs = zcl::arena_push_item<zcl::t_code_point_bitset>(&arena);

                zcl::bitset_set_range(*code_pt_bs, zcl::k_printable_ascii_range_begin, zcl::k_printable_ascii_range_end); // Add the printable ASCII range as a default.

                if (field_vals[ek_font_field_extra_chrs_file_path]) {
                    const auto extra_chrs_file_path = zcl::CStrToStr(field_vals[ek_font_field_extra_chrs_file_path]->valuestring);

                    zcl::t_array_mut<zcl::t_u8> extra_chrs_file_contents;

                    if (!zcl::FileLoadContents(extra_chrs_file_path, &arena, &arena, &extra_chrs_file_contents)) {
                        zcl::LogError(ZCL_STR_LITERAL("Failed to load extra characters file \"%\"!"), extra_chrs_file_path);
                        return false;
                    }

                    zcl::StrMarkCodePoints({extra_chrs_file_contents}, code_pt_bs);
                }

                // @todo: Proper check for invalid height!

                zcl::t_font_arrangement arrangement;
                zcl::t_array_mut<zcl::t_font_atlas_rgba> atlas_rgbas;

                if (!zcl::font_load_from_raw(file_path, height, code_pt_bs, &arena, &arena, &arena, &arrangement, &atlas_rgbas)) {
                    zcl::LogError(ZCL_STR_LITERAL("Failed to load font from file \"%\"!"), file_path);
                    return false;
                }

                if (!zcl::font_pack(out_file_path, arrangement, atlas_rgbas, &arena)) {
                    zcl::LogError(ZCL_STR_LITERAL("Failed to pack font to file \"%\"!"), out_file_path);
                    return false;
                }

                break;
            }

            case ek_asset_type_shader: {
                const auto file_path = zcl::CStrToStr(field_vals[ek_shader_field_file_path]->valuestring);
                const auto type = zcl::CStrToStr(field_vals[ek_shader_field_type]->valuestring);
                const auto varying_def_file_path = zcl::CStrToStr(field_vals[ek_shader_field_varying_def_file_path]->valuestring);
                const auto out_file_path = zcl::CStrToStr(field_vals[ek_shader_field_out_file_path]->valuestring);

                zcl::t_b8 is_frag;

                if (zcl::StrsCheckEqual(type, ZCL_STR_LITERAL("vertex"))) {
                    is_frag = false;
                } else if (zcl::StrsCheckEqual(type, ZCL_STR_LITERAL("fragment"))) {
                    is_frag = true;
                } else {
                    zcl::LogError(ZCL_STR_LITERAL("A packing instructions JSON shader entry has an invalid shader type \"%\"! Expected \"vertex\" or \"fragment\"."), type);
                    return false;
                }

                zcl::t_array_mut<zcl::t_u8> compiled_bin;

                if (!compile_shader(file_path, varying_def_file_path, is_frag, &arena, &arena, &compiled_bin)) {
                    zcl::LogError(ZCL_STR_LITERAL("Failed to compile shader from file \"%\"!"), file_path);
                    return false;
                }

                if (!zcl::shader_pack(out_file_path, compiled_bin, &arena)) {
                    zcl::LogError(ZCL_STR_LITERAL("Failed to pack shader to file \"%\"!"), out_file_path);
                    return false;
                }

                break;
            }

            case ek_asset_type_sound: {
                const auto file_path = zcl::CStrToStr(field_vals[ek_sound_field_file_path]->valuestring);
                const auto out_file_path = zcl::CStrToStr(field_vals[ek_sound_field_out_file_path]->valuestring);

                zcl::t_sound_data_mut snd_data;

                if (!zcl::sound_load_from_raw(file_path, &arena, &arena, &snd_data)) {
                    zcl::LogError(ZCL_STR_LITERAL("Failed to load sound from file \"%\"!"), file_path);
                    return false;
                }

                if (!zcl::sound_pack(out_file_path, snd_data, &arena)) {
                    zcl::LogError(ZCL_STR_LITERAL("Failed to pack sound to file \"%\"!"), out_file_path);
                    return false;
                }

                break;
            }
            }
        }
    }

    zcl::Log(ZCL_STR_LITERAL("Asset packing completed!"));

    return true;
}
