#include "zab.h"

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

[[nodiscard]] static zcl::t_b8 BuildTexture(const zcl::t_str_rdonly file_path, const zcl::t_str_rdonly out_file_path, zcl::t_arena *const temp_arena) {
    zcl::t_texture_data_mut texture_data;

    if (!zcl::TextureLoadFromUnbuilt(file_path, temp_arena, temp_arena, &texture_data)) {
        zcl::LogError(ZCL_STR_LITERAL("Failed to load texture from file \"%\"!"), file_path);
        return false;
    }

    zcl::t_file_stream file_stream;

    if (!zcl::FileOpenRecursive(out_file_path, zcl::ek_file_access_mode_write, temp_arena, &file_stream, nullptr)) {
        zcl::LogError(ZCL_STR_LITERAL("Failed to open texture output file \"%\" for writing!"), out_file_path);
        return false;
    }

    ZCL_DEFER({ zcl::FileClose(&file_stream); });

    if (!zcl::SerializeTexture(zcl::FileStreamGetView(&file_stream), texture_data)) {
        zcl::LogError(ZCL_STR_LITERAL("Failed to serialize texture to file \"%\"!"), out_file_path);
        return false;
    }

    return true;
}

[[nodiscard]] static zcl::t_b8 BuildFont(const zcl::t_str_rdonly file_path, const zcl::t_i32 height, const zcl::t_str_rdonly extra_chrs_file_path, const zcl::t_str_rdonly out_file_path, zcl::t_arena *const temp_arena) {
    const auto code_pt_bs = zcl::ArenaPushItem<zcl::t_code_point_bitset>(temp_arena);

    if (height <= 0) {
        zcl::LogError(ZCL_STR_LITERAL("Invalid font height %! Must be greater than 0."), height);
        return false;
    }

    zcl::BitsetSetRange(*code_pt_bs, zcl::k_printable_ascii_range_begin, zcl::k_printable_ascii_range_end); // Add the printable ASCII range as a default.

    if (!zcl::StrCheckEmpty(extra_chrs_file_path)) {
        zcl::t_array_mut<zcl::t_u8> extra_chrs_file_contents;

        if (!zcl::FileLoadContents(extra_chrs_file_path, temp_arena, temp_arena, &extra_chrs_file_contents)) {
            zcl::LogError(ZCL_STR_LITERAL("Failed to load extra characters file \"%\"!"), extra_chrs_file_path);
            return false;
        }

        zcl::StrMarkCodePoints({extra_chrs_file_contents}, code_pt_bs);
    }

    zcl::t_font_arrangement arrangement;
    zcl::t_array_mut<zcl::t_font_atlas_pixels_r8> atlas_pixels_arr;

    if (!zcl::FontLoadFromUnbuilt(file_path, height, code_pt_bs, temp_arena, temp_arena, temp_arena, &arrangement, &atlas_pixels_arr)) {
        zcl::LogError(ZCL_STR_LITERAL("Failed to load font from file \"%\"!"), file_path);
        return false;
    }

    zcl::t_file_stream file_stream;

    if (!zcl::FileOpenRecursive(out_file_path, zcl::ek_file_access_mode_write, temp_arena, &file_stream)) {
        zcl::LogError(ZCL_STR_LITERAL("Failed to open font output file \"%\" for writing!"), out_file_path);
        return false;
    }

    ZCL_DEFER({ zcl::FileClose(&file_stream); });

    if (!zcl::SerializeFont(zcl::FileStreamGetView(&file_stream), arrangement, atlas_pixels_arr, temp_arena)) {
        zcl::LogError(ZCL_STR_LITERAL("Failed to serialize font to file \"%\"!"), out_file_path);
        return false;
    }

    return true;
}

[[nodiscard]] static zcl::t_b8 BuildShader(const zcl::t_str_rdonly file_path, const zcl::t_str_rdonly type, const zcl::t_str_rdonly varying_def_file_path, const zcl::t_str_rdonly out_file_path, zcl::t_arena *const temp_arena) {
    zcl::t_b8 is_frag;

    if (zcl::StrsCheckEqual(type, ZCL_STR_LITERAL("vertex"))) {
        is_frag = false;
    } else if (zcl::StrsCheckEqual(type, ZCL_STR_LITERAL("fragment"))) {
        is_frag = true;
    } else {
        zcl::LogError(ZCL_STR_LITERAL("Invalid shader type \"%\"! Expected \"vertex\" or \"fragment\"."), type);
        return false;
    }

    zcl::t_array_mut<zcl::t_u8> compiled_bin;

    if (!CompileShader(file_path, varying_def_file_path, is_frag, temp_arena, temp_arena, &compiled_bin)) {
        zcl::LogError(ZCL_STR_LITERAL("Failed to compile shader from file \"%\"!"), file_path);
        return false;
    }

    zcl::t_file_stream shader_file_stream;

    if (!zcl::FileOpenRecursive(out_file_path, zcl::ek_file_access_mode_write, temp_arena, &shader_file_stream)) {
        zcl::LogError(ZCL_STR_LITERAL("Failed to open shader output file \"%\" for writing!"), out_file_path);
        return false;
    }

    ZCL_DEFER({ zcl::FileClose(&shader_file_stream); });

    if (!zcl::SerializeShader(zcl::FileStreamGetView(&shader_file_stream), compiled_bin)) {
        zcl::LogError(ZCL_STR_LITERAL("Failed to serialize shader to file \"%\"!"), out_file_path);
        return false;
    }

    return true;
}

[[nodiscard]] static zcl::t_b8 BuildSound(const zcl::t_str_rdonly file_path, const zcl::t_str_rdonly out_file_path, zcl::t_arena *const temp_arena) {
    zcl::t_sound_data_mut snd_data;

    if (!zcl::SoundLoadFromUnbuilt(file_path, temp_arena, temp_arena, &snd_data)) {
        zcl::LogError(ZCL_STR_LITERAL("Failed to load sound from file \"%\"!"), file_path);
        return false;
    }

    zcl::t_file_stream file_stream;

    if (!zcl::FileOpenRecursive(out_file_path, zcl::ek_file_access_mode_write, temp_arena, &file_stream)) {
        zcl::LogError(ZCL_STR_LITERAL("Failed to open sound output file \"%\" for writing!"), out_file_path);
        return false;
    }

    ZCL_DEFER({ zcl::FileClose(&file_stream); });

    if (!zcl::SerializeSound(zcl::FileStreamGetView(&file_stream), snd_data)) {
        zcl::LogError(ZCL_STR_LITERAL("Failed to serialize sound to file \"%\"!"), out_file_path);
        return false;
    }

    return true;
}

zcl::t_b8 BuildAssets(const zcl::t_str_rdonly instrs_json_file_path) {
    zcl::t_arena arena = zcl::ArenaCreateBlockBased();
    ZCL_DEFER({ zcl::ArenaDestroy(&arena); });

    cJSON *cj;

    {
        zcl::t_array_mut<zcl::t_u8> instrs_json_file_contents; // Not needed beyond this scope.

        if (!zcl::FileLoadContents(instrs_json_file_path, &arena, &arena, &instrs_json_file_contents, true)) {
            zcl::LogError(ZCL_STR_LITERAL("Failed to load build instructions JSON file \"%\"!"), instrs_json_file_path);
            return false;
        }

        cj = cJSON_Parse(zcl::StrToCStr(zcl::t_str_rdonly{instrs_json_file_contents}));

        if (!cj) {
            zcl::LogError(ZCL_STR_LITERAL("Failed to parse build instructions JSON file!"));
            return false;
        }
    }

    ZCL_DEFER({ cJSON_Delete(cj); });

    if (!cJSON_IsObject(cj)) {
        zcl::LogError(ZCL_STR_LITERAL("Build instructions JSON root is not an object!"));
        return false;
    }

    zcl::t_static_array<cJSON *, ekm_texture_field_cnt> texture_field_cj_vals = {};
    zcl::t_static_array<cJSON *, ekm_font_field_cnt> font_field_cj_vals = {};
    zcl::t_static_array<cJSON *, ekm_shader_field_cnt> shader_field_cj_vals = {};
    zcl::t_static_array<cJSON *, ekm_sound_field_cnt> snd_field_cj_vals = {};

    for (zcl::t_i32 asset_type_index = 0; asset_type_index < ekm_asset_type_cnt; asset_type_index++) {
        const auto asset_type_arr_name_c_str = k_asset_type_array_name_c_strs[asset_type_index];

        cJSON *const cj_assets = cJSON_GetObjectItemCaseSensitive(cj, asset_type_arr_name_c_str);

        if (!cJSON_IsArray(cj_assets)) {
            zcl::LogError(ZCL_STR_LITERAL("Build instructions JSON \"%\" array does not exist or it is of the wrong type!"), zcl::CStrToStr(asset_type_arr_name_c_str));
            return false;
        }

        cJSON *cj_asset;
        zcl::t_i32 cj_asset_index = 0;

        cJSON_ArrayForEach(cj_asset, cj_assets) {
            ZCL_DEFER({ cj_asset_index++; });

            zcl::Log(ZCL_STR_LITERAL("Building \"%\" asset %..."), zcl::CStrToStr(asset_type_arr_name_c_str), cj_asset_index);

            {
                zcl::t_file_stream std_out = zcl::FileStreamCreateStdOut();
                zcl::FileFlush(&std_out);
            }

            zcl::ArenaRewind(&arena);

            if (!cJSON_IsObject(cj_asset)) {
                zcl::LogError(ZCL_STR_LITERAL("JSON entry is of the wrong type! Expected an object."));
                return false;
            }

            const auto fields = [asset_type_index]() -> zcl::t_array_rdonly<t_asset_field> {
                switch (asset_type_index) {
                case ek_asset_type_texture: return ArrayToNonstatic(&k_texture_fields);
                case ek_asset_type_font: return ArrayToNonstatic(&k_font_fields);
                case ek_asset_type_shader: return ArrayToNonstatic(&k_shader_fields);
                case ek_asset_type_sound: return ArrayToNonstatic(&k_sound_fields);

                default: ZCL_UNREACHABLE();
                }
            }();

            const auto field_cj_vals = [asset_type_index, &texture_field_cj_vals, &font_field_cj_vals, &shader_field_cj_vals, &snd_field_cj_vals]() -> zcl::t_array_mut<cJSON *> {
                switch (asset_type_index) {
                case ek_asset_type_texture: return ArrayToNonstatic(&texture_field_cj_vals);
                case ek_asset_type_font: return ArrayToNonstatic(&font_field_cj_vals);
                case ek_asset_type_shader: return ArrayToNonstatic(&shader_field_cj_vals);
                case ek_asset_type_sound: return ArrayToNonstatic(&snd_field_cj_vals);

                default: ZCL_UNREACHABLE();
                }
            }();

            for (zcl::t_i32 fi = 0; fi < fields.len; fi++) {
                const auto field_name_c_str = fields[fi].name_c_str;

                field_cj_vals[fi] = cJSON_GetObjectItem(cj_asset, field_name_c_str);

                if (!field_cj_vals[fi]) {
                    if (fields[fi].optional) {
                        continue;
                    }

                    zcl::LogError(ZCL_STR_LITERAL("JSON entry is missing required field \"%\"!"), zcl::CStrToStr(field_name_c_str));

                    return false;
                }

                const auto is_valid = [fi, fields, field_cj_vals]() -> zcl::t_b8 {
                    switch (fields[fi].type) {
                    case ek_asset_field_type_str: return cJSON_IsString(field_cj_vals[fi]);
                    case ek_asset_field_type_num: return cJSON_IsNumber(field_cj_vals[fi]);

                    default: ZCL_UNREACHABLE();
                    }
                }();

                if (!is_valid) {
                    const auto type_name = zcl::CStrToStr(k_asset_field_type_name_c_strs[fields[fi].type]);
                    zcl::LogError(ZCL_STR_LITERAL("JSON entry has field \"%\" as the wrong type! Expected a %."), zcl::CStrToStr(field_name_c_str), type_name);
                    return false;
                }
            }

            switch (asset_type_index) {
            case ek_asset_type_texture: {
                const zcl::t_str_rdonly file_path = zcl::CStrToStr(field_cj_vals[ek_texture_field_file_path]->valuestring);
                const zcl::t_str_rdonly out_file_path = zcl::CStrToStr(field_cj_vals[ek_texture_field_out_file_path]->valuestring);

                if (!BuildTexture(file_path, out_file_path, &arena)) {
                    return false;
                }

                break;
            }

            case ek_asset_type_font: {
                const zcl::t_str_rdonly file_path = zcl::CStrToStr(field_cj_vals[ek_font_field_file_path]->valuestring);

                const zcl::t_i32 height = field_cj_vals[ek_font_field_height]->valueint;

                const zcl::t_str_rdonly extra_chrs_file_path = field_cj_vals[ek_font_field_extra_chrs_file_path]->valuestring
                    ? zcl::CStrToStr(field_cj_vals[ek_font_field_extra_chrs_file_path]->valuestring)
                    : zcl::t_str_rdonly{};

                const zcl::t_str_rdonly out_file_path = zcl::CStrToStr(field_cj_vals[ek_font_field_out_file_path]->valuestring);

                if (!BuildFont(file_path, height, extra_chrs_file_path, out_file_path, &arena)) {
                    return false;
                }

                break;
            }

            case ek_asset_type_shader: {
                const zcl::t_str_rdonly file_path = zcl::CStrToStr(field_cj_vals[ek_shader_field_file_path]->valuestring);
                const zcl::t_str_rdonly type = zcl::CStrToStr(field_cj_vals[ek_shader_field_type]->valuestring);
                const zcl::t_str_rdonly varying_def_file_path = zcl::CStrToStr(field_cj_vals[ek_shader_field_varying_def_file_path]->valuestring);
                const zcl::t_str_rdonly out_file_path = zcl::CStrToStr(field_cj_vals[ek_shader_field_out_file_path]->valuestring);

                if (!BuildShader(file_path, type, varying_def_file_path, out_file_path, &arena)) {
                    return false;
                }

                break;
            }

            case ek_asset_type_sound: {
                const zcl::t_str_rdonly file_path = zcl::CStrToStr(field_cj_vals[ek_sound_field_file_path]->valuestring);
                const zcl::t_str_rdonly out_file_path = zcl::CStrToStr(field_cj_vals[ek_sound_field_out_file_path]->valuestring);

                if (!BuildSound(file_path, out_file_path, &arena)) {
                    return false;
                }

                break;
            }
            }

            zcl::Log(ZCL_STR_LITERAL("Built successfully!\n"));

            {
                zcl::t_file_stream std_out = zcl::FileStreamCreateStdOut();
                zcl::FileFlush(&std_out);
            }
        }
    }

    zcl::Log(ZCL_STR_LITERAL("Asset building completed!"));

    return true;
}
