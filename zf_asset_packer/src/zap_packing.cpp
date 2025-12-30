#include "zap.h"

#include <cJSON.h>

namespace zf {
    enum e_asset_type : t_i32 {
        ek_asset_type_texture,
        ek_asset_type_font,
        ek_asset_type_shader,
        ek_asset_type_sound,

        eks_asset_type_cnt
    };

    constexpr s_static_array<s_cstr_literal, eks_asset_type_cnt> g_asset_type_arr_names = {{
        "textures",
        "fonts",
        "shaders",
        "sounds",
    }};

    enum e_asset_field_type : t_i32 {
        ek_asset_field_type_str,
        ek_asset_field_type_num,

        eks_asset_field_type_cnt
    };

    constexpr s_static_array<s_cstr_literal, eks_asset_field_type_cnt> g_asset_field_type_names = {{
        "string",
        "number",
    }};

    struct s_asset_field {
        s_cstr_literal name;
        e_asset_field_type type;
        t_b8 optional;
    };

    enum e_texture_field : t_i32 {
        ek_texture_field_file_path,
        ek_texture_field_out_file_path,
        eks_texture_field_cnt
    };

    constexpr s_static_array<s_asset_field, eks_texture_field_cnt> g_texture_fields = {{
        {.name = "file_path", .type = ek_asset_field_type_str},
        {.name = "out_file_path", .type = ek_asset_field_type_str},
    }};

    enum e_font_field : t_i32 {
        ek_font_field_file_path,
        ek_font_field_height,
        ek_font_field_extra_chrs_file_path,
        ek_font_field_out_file_path,

        eks_font_field_cnt
    };

    constexpr s_static_array<s_asset_field, eks_font_field_cnt> g_font_fields = {{
        {.name = "file_path", .type = ek_asset_field_type_str},
        {.name = "height", .type = ek_asset_field_type_num},
        {.name = "extra_chrs_file_path", .type = ek_asset_field_type_str, .optional = true},
        {.name = "out_file_path", .type = ek_asset_field_type_str},
    }};

    enum e_shader_field : t_i32 {
        ek_shader_field_file_path,
        ek_shader_field_type,
        ek_shader_field_varying_def_file_path,
        ek_shader_field_out_file_path,

        eks_shader_field_cnt
    };

    constexpr s_static_array<s_asset_field, eks_shader_field_cnt> g_shader_fields = {{
        {.name = "file_path", .type = ek_asset_field_type_str},
        {.name = "type", .type = ek_asset_field_type_str},
        {.name = "varying_def_file_path", .type = ek_asset_field_type_str},
        {.name = "out_file_path", .type = ek_asset_field_type_str},
    }};

    enum e_sound_field : t_i32 {
        ek_sound_field_file_path,
        ek_sound_field_out_file_path,
        eks_sound_field_cnt
    };

    constexpr static s_static_array<s_asset_field, eks_sound_field_cnt> g_sound_fields = {{
        {.name = "file_path", .type = ek_asset_field_type_str},
        {.name = "out_file_path", .type = ek_asset_field_type_str},
    }};

    t_b8 PackAssets(const s_str_rdonly instrs_json_file_path) {
        s_arena arena = CreateArena();
        ZF_DEFER({ DestroyArena(&arena); });

        ZF_DEFINE_UNINITTED(cJSON *, cj);

        {
            ZF_DEFINE_UNINITTED(s_array_mut<t_u8>, instrs_json_file_contents); // Not needed beyond this scope.

            if (!LoadFileContents(instrs_json_file_path, &arena, &arena, &instrs_json_file_contents, true)) {
                LogError(s_cstr_literal("Failed to load packing instructions JSON file \"%\"!"), instrs_json_file_path);
                return false;
            }

            cj = cJSON_Parse(s_str(instrs_json_file_contents).Cstr());

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

        s_static_array<cJSON *, eks_texture_field_cnt> texture_field_cj_ptrs;
        PoisonUnittedItem(&texture_field_cj_ptrs);

        s_static_array<cJSON *, eks_font_field_cnt> font_field_cj_ptrs;
        PoisonUnittedItem(&font_field_cj_ptrs);

        s_static_array<cJSON *, eks_shader_field_cnt> shader_field_cj_ptrs;
        PoisonUnittedItem(&shader_field_cj_ptrs);

        s_static_array<cJSON *, eks_sound_field_cnt> snd_field_cj_ptrs;
        PoisonUnittedItem(&snd_field_cj_ptrs);

        for (t_i32 asset_type_index = 0; asset_type_index < eks_asset_type_cnt; asset_type_index++) {
            const auto asset_type_arr_name = g_asset_type_arr_names[asset_type_index];

            cJSON *const cj_assets = cJSON_GetObjectItemCaseSensitive(cj, asset_type_arr_name.buf.raw);

            if (!cJSON_IsArray(cj_assets)) {
                LogError(s_cstr_literal("Packing instructions JSON \"%\" array does not exist or it is of the wrong type!"), asset_type_arr_name);
                return false;
            }

            ZF_DEFINE_UNINITTED(cJSON *, cj_asset);

            cJSON_ArrayForEach(cj_asset, cj_assets) {
                RewindArena(&arena);

                if (!cJSON_IsObject(cj_asset)) {
                    continue;
                }

                const auto fields = [asset_type_index]() -> s_array_rdonly<s_asset_field> {
                    switch (asset_type_index) {
                    case ek_asset_type_texture: return g_texture_fields;
                    case ek_asset_type_font: return g_font_fields;
                    case ek_asset_type_shader: return g_shader_fields;
                    case ek_asset_type_sound: return g_sound_fields;
                    }

                    return {};
                }();

                const auto field_vals = [asset_type_index, &texture_field_cj_ptrs, &font_field_cj_ptrs, &shader_field_cj_ptrs, &snd_field_cj_ptrs]() -> s_array_mut<cJSON *> {
                    switch (asset_type_index) {
                    case ek_asset_type_texture: return texture_field_cj_ptrs;
                    case ek_asset_type_font: return font_field_cj_ptrs;
                    case ek_asset_type_shader: return shader_field_cj_ptrs;
                    case ek_asset_type_sound: return snd_field_cj_ptrs;
                    }

                    return {};
                }();

                for (t_i32 fi = 0; fi < fields.len; fi++) {
                    const auto field_name = fields[fi].name;

                    field_vals[fi] = cJSON_GetObjectItem(cj_asset, field_name.buf.raw);

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
                            ZF_UNREACHABLE();
                        }

                        ZF_UNREACHABLE();
                    }();

                    if (!is_valid) {
                        LogError(s_cstr_literal("A packing instructions JSON \"%\" entry has field \"%\" as the wrong type! Expected a %."), asset_type_arr_name, field_name, g_asset_field_type_names[fields[fi].type]);
                        return false;
                    }
                }

                switch (asset_type_index) {
                case ek_asset_type_texture: {
                    const auto file_path = ConvertCstr(field_vals[ek_texture_field_file_path]->valuestring);
                    const auto out_file_path = ConvertCstr(field_vals[ek_texture_field_out_file_path]->valuestring);

                    ZF_DEFINE_UNINITTED(s_texture_data, texture_data);

                    if (!LoadTextureDataFromRaw(file_path, &arena, &arena, &texture_data)) {
                        LogError(s_cstr_literal("Failed to load texture from file \"%\"!"), file_path);
                        return false;
                    }

                    if (!PackTexture(out_file_path, texture_data, &arena)) {
                        LogError(s_cstr_literal("Failed to pack texture to file \"%\"!"), out_file_path);
                        return false;
                    }

                    break;
                }

                case ek_asset_type_font: {
                    const auto file_path = ConvertCstr(field_vals[ek_font_field_file_path]->valuestring);
                    const auto height = field_vals[ek_font_field_height]->valueint;
                    const auto out_file_path = ConvertCstr(field_vals[ek_font_field_out_file_path]->valuestring);

                    const auto code_pt_bv = Alloc<t_code_pt_bit_vec>(&arena);

                    SetBitsInRange(*code_pt_bv, g_printable_ascii_range_begin, g_printable_ascii_range_end); // Add the printable ASCII range as a default.

                    if (field_vals[ek_font_field_extra_chrs_file_path]) {
                        const auto extra_chrs_file_path = ConvertCstr(field_vals[ek_font_field_extra_chrs_file_path]->valuestring);

                        ZF_DEFINE_UNINITTED(s_array_mut<t_u8>, extra_chrs_file_contents);

                        if (!LoadFileContents(extra_chrs_file_path, &arena, &arena, &extra_chrs_file_contents)) {
                            LogError(s_cstr_literal("Failed to load extra characters file \"%\"!"), extra_chrs_file_path);
                            return false;
                        }

                        MarkStrCodePoints({extra_chrs_file_contents}, code_pt_bv);
                    }

                    // @todo: Proper check for invalid height!

                    ZF_DEFINE_UNINITTED(s_font_arrangement, arrangement);
                    ZF_DEFINE_UNINITTED(s_array_mut<t_font_atlas_rgba>, atlas_rgbas);

                    if (!LoadFontDataFromRaw(file_path, height, code_pt_bv, &arena, &arena, &arena, &arrangement, &atlas_rgbas)) {
                        LogError(s_cstr_literal("Failed to load font from file \"%\"!"), file_path);
                        return false;
                    }

                    if (!PackFont(out_file_path, arrangement, atlas_rgbas, &arena)) {
                        LogError(s_cstr_literal("Failed to pack font to file \"%\"!"), out_file_path);
                        return false;
                    }

                    break;
                }

                case ek_asset_type_shader: {
                    const auto file_path = ConvertCstr(field_vals[ek_shader_field_file_path]->valuestring);
                    const auto type = ConvertCstr(field_vals[ek_shader_field_type]->valuestring);
                    const auto varying_def_file_path = ConvertCstr(field_vals[ek_shader_field_varying_def_file_path]->valuestring);
                    const auto out_file_path = ConvertCstr(field_vals[ek_shader_field_out_file_path]->valuestring);

                    ZF_DEFINE_UNINITTED(t_b8, is_frag);

                    if (AreStrsEqual(type, s_cstr_literal("vertex"))) {
                        is_frag = false;
                    } else if (AreStrsEqual(type, s_cstr_literal("fragment"))) {
                        is_frag = true;
                    } else {
                        LogError(s_cstr_literal("A packing instructions JSON shader entry has an invalid shader type \"%\"! Expected \"vertex\" or \"fragment\"."), type);
                        return false;
                    }

                    ZF_DEFINE_UNINITTED(s_array_mut<t_u8>, compiled_bin);

                    if (!CompileShader(file_path, varying_def_file_path, is_frag, &arena, &arena, &compiled_bin)) {
                        LogError(s_cstr_literal("Failed to compile shader from file \"%\"!"), file_path);
                        return false;
                    }

                    if (!PackShader(out_file_path, compiled_bin, &arena)) {
                        LogError(s_cstr_literal("Failed to pack shader to file \"%\"!"), out_file_path);
                        return false;
                    }

                    break;
                }

                case ek_asset_type_sound: {
                    const auto file_path = ConvertCstr(field_vals[ek_sound_field_file_path]->valuestring);
                    const auto out_file_path = ConvertCstr(field_vals[ek_sound_field_out_file_path]->valuestring);

                    ZF_DEFINE_UNINITTED(s_sound_data_mut, snd_data);

                    if (!LoadSoundDataFromRaw(file_path, &arena, &arena, &snd_data)) {
                        LogError(s_cstr_literal("Failed to load sound from file \"%\"!"), file_path);
                        return false;
                    }

                    if (!PackSound(out_file_path, snd_data, &arena)) {
                        LogError(s_cstr_literal("Failed to pack sound to file \"%\"!"), out_file_path);
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
