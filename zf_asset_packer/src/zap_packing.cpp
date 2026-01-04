#include "zap.h"

#include <cJSON.h>

namespace zf {
    enum e_asset_type : I32 {
        ek_asset_type_texture,
        ek_asset_type_font,
        ek_asset_type_shader,
        ek_asset_type_sound,

        eks_asset_type_cnt
    };

    static const s_static_array<const char *, eks_asset_type_cnt> g_asset_type_arr_name_cstrs = {{
        "textures",
        "fonts",
        "shaders",
        "sounds",
    }};

    enum e_asset_field_type : I32 {
        ek_asset_field_type_str,
        ek_asset_field_type_num,

        eks_asset_field_type_cnt
    };

    static const s_static_array<const char *, eks_asset_field_type_cnt> g_asset_field_type_name_cstrs = {{
        "string",
        "number",
    }};

    struct s_asset_field {
        const char *name_cstr;
        e_asset_field_type type;
        B8 optional;
    };

    enum e_texture_field : I32 {
        ek_texture_field_file_path,
        ek_texture_field_out_file_path,
        eks_texture_field_cnt
    };

    static const s_static_array<s_asset_field, eks_texture_field_cnt> g_texture_fields = {{
        {.name_cstr = "file_path", .type = ek_asset_field_type_str},
        {.name_cstr = "out_file_path", .type = ek_asset_field_type_str},
    }};

    enum e_font_field : I32 {
        ek_font_field_file_path,
        ek_font_field_height,
        ek_font_field_extra_chrs_file_path,
        ek_font_field_out_file_path,

        eks_font_field_cnt
    };

    static const s_static_array<s_asset_field, eks_font_field_cnt> g_font_fields = {{
        {.name_cstr = "file_path", .type = ek_asset_field_type_str},
        {.name_cstr = "height", .type = ek_asset_field_type_num},
        {.name_cstr = "extra_chrs_file_path", .type = ek_asset_field_type_str, .optional = true},
        {.name_cstr = "out_file_path", .type = ek_asset_field_type_str},
    }};

    enum e_shader_field : I32 {
        ek_shader_field_file_path,
        ek_shader_field_type,
        ek_shader_field_varying_def_file_path,
        ek_shader_field_out_file_path,

        eks_shader_field_cnt
    };

    static const s_static_array<s_asset_field, eks_shader_field_cnt> g_shader_fields = {{
        {.name_cstr = "file_path", .type = ek_asset_field_type_str},
        {.name_cstr = "type", .type = ek_asset_field_type_str},
        {.name_cstr = "varying_def_file_path", .type = ek_asset_field_type_str},
        {.name_cstr = "out_file_path", .type = ek_asset_field_type_str},
    }};

    enum e_sound_field : I32 {
        ek_sound_field_file_path,
        ek_sound_field_out_file_path,
        eks_sound_field_cnt
    };

    static const s_static_array<s_asset_field, eks_sound_field_cnt> g_sound_fields = {{
        {.name_cstr = "file_path", .type = ek_asset_field_type_str},
        {.name_cstr = "out_file_path", .type = ek_asset_field_type_str},
    }};

    B8 PackAssets(const strs::StrRdonly instrs_json_file_path) {
        s_arena arena = CreateArena();
        ZF_DEFER({ ArenaDestroy(&arena); });

        cJSON *cj;

        {
            s_array_mut<U8> instrs_json_file_contents; // Not needed beyond this scope.

            if (!LoadFileContents(instrs_json_file_path, &arena, &arena, &instrs_json_file_contents, true)) {
                LogError(ZF_STR_LITERAL("Failed to load packing instructions JSON file \"%\"!"), instrs_json_file_path);
                return false;
            }

            cj = cJSON_Parse(strs::get_as_cstr(strs::StrRdonly{instrs_json_file_contents}));

            if (!cj) {
                LogError(ZF_STR_LITERAL("Failed to parse packing instructions JSON file!"));
                return false;
            }
        }

        ZF_DEFER({ cJSON_Delete(cj); });

        if (!cJSON_IsObject(cj)) {
            LogError(ZF_STR_LITERAL("Packing instructions JSON root is not an object!"));
            return false;
        }

        s_static_array<cJSON *, eks_texture_field_cnt> texture_field_cj_ptrs = {};
        s_static_array<cJSON *, eks_font_field_cnt> font_field_cj_ptrs = {};
        s_static_array<cJSON *, eks_shader_field_cnt> shader_field_cj_ptrs = {};
        s_static_array<cJSON *, eks_sound_field_cnt> snd_field_cj_ptrs = {};

        for (I32 asset_type_index = 0; asset_type_index < eks_asset_type_cnt; asset_type_index++) {
            const auto asset_type_arr_name_cstr = g_asset_type_arr_name_cstrs[asset_type_index];

            cJSON *const cj_assets = cJSON_GetObjectItemCaseSensitive(cj, asset_type_arr_name_cstr);

            if (!cJSON_IsArray(cj_assets)) {
                LogError(ZF_STR_LITERAL("Packing instructions JSON \"%\" array does not exist or it is of the wrong type!"), strs::convert_cstr(asset_type_arr_name_cstr));
                return false;
            }

            cJSON *cj_asset;

            cJSON_ArrayForEach(cj_asset, cj_assets) {
                zf_mem_rewind_arena(&arena);

                if (!cJSON_IsObject(cj_asset)) {
                    continue;
                }

                const auto fields = [asset_type_index]() -> s_array_rdonly<s_asset_field> {
                    switch (asset_type_index) {
                    case ek_asset_type_texture: return AsNonstatic(g_texture_fields);
                    case ek_asset_type_font: return AsNonstatic(g_font_fields);
                    case ek_asset_type_shader: return AsNonstatic(g_shader_fields);
                    case ek_asset_type_sound: return AsNonstatic(g_sound_fields);
                    }

                    return {};
                }();

                const auto field_vals = [asset_type_index, &texture_field_cj_ptrs, &font_field_cj_ptrs, &shader_field_cj_ptrs, &snd_field_cj_ptrs]() -> s_array_mut<cJSON *> {
                    switch (asset_type_index) {
                    case ek_asset_type_texture: return AsNonstatic(texture_field_cj_ptrs);
                    case ek_asset_type_font: return AsNonstatic(font_field_cj_ptrs);
                    case ek_asset_type_shader: return AsNonstatic(shader_field_cj_ptrs);
                    case ek_asset_type_sound: return AsNonstatic(snd_field_cj_ptrs);
                    }

                    return {};
                }();

                for (I32 fi = 0; fi < fields.len; fi++) {
                    const auto field_name_cstr = fields[fi].name_cstr;

                    field_vals[fi] = cJSON_GetObjectItem(cj_asset, field_name_cstr);

                    if (!field_vals[fi]) {
                        if (fields[fi].optional) {
                            continue;
                        }

                        LogError(ZF_STR_LITERAL("A packing instructions JSON \"%\" entry is missing required field \"%\"!"), strs::convert_cstr(asset_type_arr_name_cstr), strs::convert_cstr(field_name_cstr));

                        return false;
                    }

                    const auto is_valid = [fi, fields, field_vals]() -> B8 {
                        switch (fields[fi].type) {
                        case ek_asset_field_type_str:
                            return cJSON_IsString(field_vals[fi]);

                        case ek_asset_field_type_num:
                            return cJSON_IsNumber(field_vals[fi]);

                        case eks_asset_field_type_cnt:
                            ZF_UNREACHABLE();
                        }

                        ZF_UNREACHABLE();
                    }();

                    if (!is_valid) {
                        LogError(ZF_STR_LITERAL("A packing instructions JSON \"%\" entry has field \"%\" as the wrong type! Expected a %."), strs::convert_cstr(asset_type_arr_name_cstr), strs::convert_cstr(field_name_cstr), strs::convert_cstr(g_asset_field_type_name_cstrs[fields[fi].type]));
                        return false;
                    }
                }

                switch (asset_type_index) {
                case ek_asset_type_texture: {
                    const auto file_path = strs::convert_cstr(field_vals[ek_texture_field_file_path]->valuestring);
                    const auto out_file_path = strs::convert_cstr(field_vals[ek_texture_field_out_file_path]->valuestring);

                    s_texture_data texture_data;

                    if (!LoadTextureDataFromRaw(file_path, &arena, &arena, &texture_data)) {
                        LogError(ZF_STR_LITERAL("Failed to load texture from file \"%\"!"), file_path);
                        return false;
                    }

                    if (!PackTexture(out_file_path, texture_data, &arena)) {
                        LogError(ZF_STR_LITERAL("Failed to pack texture to file \"%\"!"), out_file_path);
                        return false;
                    }

                    break;
                }

                case ek_asset_type_font: {
                    const auto file_path = strs::convert_cstr(field_vals[ek_font_field_file_path]->valuestring);
                    const auto height = field_vals[ek_font_field_height]->valueint;
                    const auto out_file_path = strs::convert_cstr(field_vals[ek_font_field_out_file_path]->valuestring);

                    const auto code_pt_bv = mem_push_item_zeroed<strs::CodePointBitVector>(&arena);

                    SetBitsInRange(*code_pt_bv, strs::g_printable_ascii_range_begin, strs::g_printable_ascii_range_end); // Add the printable ASCII range as a default.

                    if (field_vals[ek_font_field_extra_chrs_file_path]) {
                        const auto extra_chrs_file_path = strs::convert_cstr(field_vals[ek_font_field_extra_chrs_file_path]->valuestring);

                        s_array_mut<U8> extra_chrs_file_contents;

                        if (!LoadFileContents(extra_chrs_file_path, &arena, &arena, &extra_chrs_file_contents)) {
                            LogError(ZF_STR_LITERAL("Failed to load extra characters file \"%\"!"), extra_chrs_file_path);
                            return false;
                        }

                        strs::mark_code_points({extra_chrs_file_contents}, code_pt_bv);
                    }

                    // @todo: Proper check for invalid height!

                    s_font_arrangement arrangement;
                    s_array_mut<t_font_atlas_rgba> atlas_rgbas;

                    if (!LoadFontDataFromRaw(file_path, height, code_pt_bv, &arena, &arena, &arena, &arrangement, &atlas_rgbas)) {
                        LogError(ZF_STR_LITERAL("Failed to load font from file \"%\"!"), file_path);
                        return false;
                    }

                    if (!PackFont(out_file_path, arrangement, atlas_rgbas, &arena)) {
                        LogError(ZF_STR_LITERAL("Failed to pack font to file \"%\"!"), out_file_path);
                        return false;
                    }

                    break;
                }

                case ek_asset_type_shader: {
                    const auto file_path = strs::convert_cstr(field_vals[ek_shader_field_file_path]->valuestring);
                    const auto type = strs::convert_cstr(field_vals[ek_shader_field_type]->valuestring);
                    const auto varying_def_file_path = strs::convert_cstr(field_vals[ek_shader_field_varying_def_file_path]->valuestring);
                    const auto out_file_path = strs::convert_cstr(field_vals[ek_shader_field_out_file_path]->valuestring);

                    B8 is_frag;

                    if (strs::determine_are_equal(type, ZF_STR_LITERAL("vertex"))) {
                        is_frag = false;
                    } else if (strs::determine_are_equal(type, ZF_STR_LITERAL("fragment"))) {
                        is_frag = true;
                    } else {
                        LogError(ZF_STR_LITERAL("A packing instructions JSON shader entry has an invalid shader type \"%\"! Expected \"vertex\" or \"fragment\"."), type);
                        return false;
                    }

                    s_array_mut<U8> compiled_bin;

                    if (!CompileShader(file_path, varying_def_file_path, is_frag, &arena, &arena, &compiled_bin)) {
                        LogError(ZF_STR_LITERAL("Failed to compile shader from file \"%\"!"), file_path);
                        return false;
                    }

                    if (!PackShader(out_file_path, compiled_bin, &arena)) {
                        LogError(ZF_STR_LITERAL("Failed to pack shader to file \"%\"!"), out_file_path);
                        return false;
                    }

                    break;
                }

                case ek_asset_type_sound: {
                    const auto file_path = strs::convert_cstr(field_vals[ek_sound_field_file_path]->valuestring);
                    const auto out_file_path = strs::convert_cstr(field_vals[ek_sound_field_out_file_path]->valuestring);

                    audio::SoundDataMut snd_data;

                    if (!load_sound_data_from_raw(file_path, &arena, &arena, &snd_data)) {
                        LogError(ZF_STR_LITERAL("Failed to load sound from file \"%\"!"), file_path);
                        return false;
                    }

                    if (!pack_sound(out_file_path, snd_data, &arena)) {
                        LogError(ZF_STR_LITERAL("Failed to pack sound to file \"%\"!"), out_file_path);
                        return false;
                    }

                    break;
                }
                }
            }
        }

        Log(ZF_STR_LITERAL("Asset packing completed!"));

        return true;
    }
}
