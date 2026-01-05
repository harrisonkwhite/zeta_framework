#include "zap.h"

#include <cJSON.h>

namespace zf {
    enum t_asset_type : t_i32 {
        ec_asset_type_texture,
        ec_asset_type_font,
        ec_asset_type_shader,
        ec_asset_type_sound,

        ecm_asset_type_cnt
    };

    static const t_static_array<const char *, ecm_asset_type_cnt> g_asset_type_array_name_cstrs = {{
        "textures",
        "fonts",
        "shaders",
        "sounds",
    }};

    enum t_asset_field_type : t_i32 {
        ec_asset_field_type_str,
        ec_asset_field_type_num,

        ecm_asset_field_type_cnt
    };

    static const t_static_array<const char *, ecm_asset_field_type_cnt> g_asset_field_type_name_cstrs = {{
        "string",
        "number",
    }};

    struct t_asset_field {
        const char *name_cstr;
        t_asset_field_type type;
        t_b8 optional;
    };

    enum t_texture_field : t_i32 {
        ec_texture_field_file_path,
        ec_texture_field_out_file_path,

        ecm_texture_field_cnt
    };

    static const t_static_array<t_asset_field, ecm_texture_field_cnt> g_texture_fields = {{
        {.name_cstr = "file_path", .type = ec_asset_field_type_str},
        {.name_cstr = "out_file_path", .type = ec_asset_field_type_str},
    }};

    enum t_font_field : t_i32 {
        ec_font_field_file_path,
        ec_font_field_height,
        ec_font_field_extra_chrs_file_path,
        ec_font_field_out_file_path,

        ecm_font_field_cnt
    };

    static const t_static_array<t_asset_field, ecm_font_field_cnt> g_font_fields = {{
        {.name_cstr = "file_path", .type = ec_asset_field_type_str},
        {.name_cstr = "height", .type = ec_asset_field_type_num},
        {.name_cstr = "extra_chrs_file_path", .type = ec_asset_field_type_str, .optional = true},
        {.name_cstr = "out_file_path", .type = ec_asset_field_type_str},
    }};

    enum t_shader_field : t_i32 {
        ec_shader_field_file_path,
        ec_shader_field_type,
        ec_shader_field_varying_def_file_path,
        ec_shader_field_out_file_path,

        ecm_shader_field_cnt
    };

    static const t_static_array<t_asset_field, ecm_shader_field_cnt> g_shader_fields = {{
        {.name_cstr = "file_path", .type = ec_asset_field_type_str},
        {.name_cstr = "type", .type = ec_asset_field_type_str},
        {.name_cstr = "varying_def_file_path", .type = ec_asset_field_type_str},
        {.name_cstr = "out_file_path", .type = ec_asset_field_type_str},
    }};

    enum t_sound_field : t_i32 {
        ec_sound_field_file_path,
        ec_sound_field_out_file_path,

        ecm_sound_field_cnt
    };

    static const t_static_array<t_asset_field, ecm_sound_field_cnt> g_sound_fields = {{
        {.name_cstr = "file_path", .type = ec_asset_field_type_str},
        {.name_cstr = "out_file_path", .type = ec_asset_field_type_str},
    }};

    t_b8 f_pack_assets(const strs::t_str_rdonly instrs_json_file_path) {
        mem::t_arena arena = mem::f_arena_create();
        ZF_DEFER({ mem::f_arena_destroy(&arena); });

        cJSON *cj;

        {
            t_array_mut<t_u8> instrs_json_file_contents; // Not needed beyond this scope.

            if (!io::f_load_file_contents(instrs_json_file_path, &arena, &arena, &instrs_json_file_contents, true)) {
                io::f_log_error(ZF_STR_LITERAL("Failed to load packing instructions JSON file \"%\"!"), instrs_json_file_path);
                return false;
            }

            cj = cJSON_Parse(strs::f_get_as_cstr(strs::t_str_rdonly{instrs_json_file_contents}));

            if (!cj) {
                io::f_log_error(ZF_STR_LITERAL("Failed to parse packing instructions JSON file!"));
                return false;
            }
        }

        ZF_DEFER({ cJSON_Delete(cj); });

        if (!cJSON_IsObject(cj)) {
            io::f_log_error(ZF_STR_LITERAL("Packing instructions JSON root is not an object!"));
            return false;
        }

        t_static_array<cJSON *, ecm_texture_field_cnt> texture_field_cj_ptrs = {};
        t_static_array<cJSON *, ecm_font_field_cnt> font_field_cj_ptrs = {};
        t_static_array<cJSON *, ecm_shader_field_cnt> shader_field_cj_ptrs = {};
        t_static_array<cJSON *, ecm_sound_field_cnt> snd_field_cj_ptrs = {};

        for (t_i32 asset_type_index = 0; asset_type_index < ecm_asset_type_cnt; asset_type_index++) {
            const auto asset_type_arr_name_cstr = g_asset_type_array_name_cstrs[asset_type_index];

            cJSON *const cj_assets = cJSON_GetObjectItemCaseSensitive(cj, asset_type_arr_name_cstr);

            if (!cJSON_IsArray(cj_assets)) {
                io::f_log_error(ZF_STR_LITERAL("Packing instructions JSON \"%\" array does not exist or it is of the wrong type!"), strs::f_convert_cstr(asset_type_arr_name_cstr));
                return false;
            }

            cJSON *cj_asset;

            cJSON_ArrayForEach(cj_asset, cj_assets) {
                mem::f_arena_rewind(&arena);

                if (!cJSON_IsObject(cj_asset)) {
                    continue;
                }

                const auto fields = [asset_type_index]() -> t_array_rdonly<t_asset_field> {
                    switch (asset_type_index) {
                    case ec_asset_type_texture: return f_array_get_as_nonstatic(g_texture_fields);
                    case ec_asset_type_font: return f_array_get_as_nonstatic(g_font_fields);
                    case ec_asset_type_shader: return f_array_get_as_nonstatic(g_shader_fields);
                    case ec_asset_type_sound: return f_array_get_as_nonstatic(g_sound_fields);
                    }

                    return {};
                }();

                const auto field_vals = [asset_type_index, &texture_field_cj_ptrs, &font_field_cj_ptrs, &shader_field_cj_ptrs, &snd_field_cj_ptrs]() -> t_array_mut<cJSON *> {
                    switch (asset_type_index) {
                    case ec_asset_type_texture: return f_array_get_as_nonstatic(texture_field_cj_ptrs);
                    case ec_asset_type_font: return f_array_get_as_nonstatic(font_field_cj_ptrs);
                    case ec_asset_type_shader: return f_array_get_as_nonstatic(shader_field_cj_ptrs);
                    case ec_asset_type_sound: return f_array_get_as_nonstatic(snd_field_cj_ptrs);
                    }

                    return {};
                }();

                for (t_i32 fi = 0; fi < fields.len; fi++) {
                    const auto field_name_cstr = fields[fi].name_cstr;

                    field_vals[fi] = cJSON_GetObjectItem(cj_asset, field_name_cstr);

                    if (!field_vals[fi]) {
                        if (fields[fi].optional) {
                            continue;
                        }

                        io::f_log_error(ZF_STR_LITERAL("A packing instructions JSON \"%\" entry is missing required field \"%\"!"), strs::f_convert_cstr(asset_type_arr_name_cstr), strs::f_convert_cstr(field_name_cstr));

                        return false;
                    }

                    const auto is_valid = [fi, fields, field_vals]() -> t_b8 {
                        switch (fields[fi].type) {
                        case ec_asset_field_type_str:
                            return cJSON_IsString(field_vals[fi]);

                        case ec_asset_field_type_num:
                            return cJSON_IsNumber(field_vals[fi]);

                        case ecm_asset_field_type_cnt:
                            ZF_UNREACHABLE();
                        }

                        ZF_UNREACHABLE();
                    }();

                    if (!is_valid) {
                        io::f_log_error(ZF_STR_LITERAL("A packing instructions JSON \"%\" entry has field \"%\" as the wrong type! Expected a %."), strs::f_convert_cstr(asset_type_arr_name_cstr), strs::f_convert_cstr(field_name_cstr), strs::f_convert_cstr(g_asset_field_type_name_cstrs[fields[fi].type]));
                        return false;
                    }
                }

                switch (asset_type_index) {
                case ec_asset_type_texture: {
                    const auto file_path = strs::f_convert_cstr(field_vals[ec_texture_field_file_path]->valuestring);
                    const auto out_file_path = strs::f_convert_cstr(field_vals[ec_texture_field_out_file_path]->valuestring);

                    gfx::t_texture_data_mut texture_data;

                    if (!gfx::f_load_texture_from_raw(file_path, &arena, &arena, &texture_data)) {
                        io::f_log_error(ZF_STR_LITERAL("Failed to load texture from file \"%\"!"), file_path);
                        return false;
                    }

                    if (!gfx::f_pack_texture(out_file_path, texture_data, &arena)) {
                        io::f_log_error(ZF_STR_LITERAL("Failed to pack texture to file \"%\"!"), out_file_path);
                        return false;
                    }

                    break;
                }

                case ec_asset_type_font: {
                    const auto file_path = strs::f_convert_cstr(field_vals[ec_font_field_file_path]->valuestring);
                    const auto height = field_vals[ec_font_field_height]->valueint;
                    const auto out_file_path = strs::f_convert_cstr(field_vals[ec_font_field_out_file_path]->valuestring);

                    const auto code_pt_bv = mem::f_arena_push_item_zeroed<strs::t_code_pt_bit_vec>(&arena);

                    mem::f_set_bits_in_range(*code_pt_bv, strs::g_printable_ascii_range_begin, strs::g_printable_ascii_range_end); // Add the printable ASCII range as a default.

                    if (field_vals[ec_font_field_extra_chrs_file_path]) {
                        const auto extra_chrs_file_path = strs::f_convert_cstr(field_vals[ec_font_field_extra_chrs_file_path]->valuestring);

                        t_array_mut<t_u8> extra_chrs_file_contents;

                        if (!io::f_load_file_contents(extra_chrs_file_path, &arena, &arena, &extra_chrs_file_contents)) {
                            io::f_log_error(ZF_STR_LITERAL("Failed to load extra characters file \"%\"!"), extra_chrs_file_path);
                            return false;
                        }

                        strs::f_mark_code_points({extra_chrs_file_contents}, code_pt_bv);
                    }

                    // @todo: Proper check for invalid height!

                    gfx::t_font_arrangement arrangement;
                    t_array_mut<gfx::t_font_atlas_rgba> atlas_rgbas;

                    if (!gfx::f_load_font_from_raw(file_path, height, code_pt_bv, &arena, &arena, &arena, &arrangement, &atlas_rgbas)) {
                        io::f_log_error(ZF_STR_LITERAL("Failed to load font from file \"%\"!"), file_path);
                        return false;
                    }

                    if (!gfx::f_pack_font(out_file_path, arrangement, atlas_rgbas, &arena)) {
                        io::f_log_error(ZF_STR_LITERAL("Failed to pack font to file \"%\"!"), out_file_path);
                        return false;
                    }

                    break;
                }

                case ec_asset_type_shader: {
                    const auto file_path = strs::f_convert_cstr(field_vals[ec_shader_field_file_path]->valuestring);
                    const auto type = strs::f_convert_cstr(field_vals[ec_shader_field_type]->valuestring);
                    const auto varying_def_file_path = strs::f_convert_cstr(field_vals[ec_shader_field_varying_def_file_path]->valuestring);
                    const auto out_file_path = strs::f_convert_cstr(field_vals[ec_shader_field_out_file_path]->valuestring);

                    t_b8 is_frag;

                    if (strs::f_are_equal(type, ZF_STR_LITERAL("vertex"))) {
                        is_frag = false;
                    } else if (strs::f_are_equal(type, ZF_STR_LITERAL("fragment"))) {
                        is_frag = true;
                    } else {
                        io::f_log_error(ZF_STR_LITERAL("A packing instructions JSON shader entry has an invalid shader type \"%\"! Expected \"vertex\" or \"fragment\"."), type);
                        return false;
                    }

                    t_array_mut<t_u8> compiled_bin;

                    if (!f_compile_shader(file_path, varying_def_file_path, is_frag, &arena, &arena, &compiled_bin)) {
                        io::f_log_error(ZF_STR_LITERAL("Failed to compile shader from file \"%\"!"), file_path);
                        return false;
                    }

                    if (!gfx::f_pack_shader(out_file_path, compiled_bin, &arena)) {
                        io::f_log_error(ZF_STR_LITERAL("Failed to pack shader to file \"%\"!"), out_file_path);
                        return false;
                    }

                    break;
                }

                case ec_asset_type_sound: {
                    const auto file_path = strs::f_convert_cstr(field_vals[ec_sound_field_file_path]->valuestring);
                    const auto out_file_path = strs::f_convert_cstr(field_vals[ec_sound_field_out_file_path]->valuestring);

                    audio::t_sound_data_mut snd_data;

                    if (!f_sound_load_from_raw(file_path, &arena, &arena, &snd_data)) {
                        io::f_log_error(ZF_STR_LITERAL("Failed to load sound from file \"%\"!"), file_path);
                        return false;
                    }

                    if (!f_sound_pack(out_file_path, snd_data, &arena)) {
                        io::f_log_error(ZF_STR_LITERAL("Failed to pack sound to file \"%\"!"), out_file_path);
                        return false;
                    }

                    break;
                }
                }
            }
        }

        io::f_log(ZF_STR_LITERAL("Asset packing completed!"));

        return true;
    }
}
