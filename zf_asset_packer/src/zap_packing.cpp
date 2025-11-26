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

    static s_static_array<const char*, eks_asset_type_cnt> g_asset_type_arr_raw_names = {{
        "textures",
        "fonts",
        "sounds"
    }};

    enum e_asset_field_type : t_s32 {
        ek_asset_field_type_str,
        ek_asset_field_type_num,

        eks_asset_field_type_cnt
    };

    static s_static_array<const char*, eks_asset_field_type_cnt> g_asset_field_type_raw_names = {{
        "string",
        "number"
    }};

    struct s_asset_field {
        const char* name_raw;
        e_asset_field_type type;
        t_b8 optional;
    };

    enum e_tex_field : t_s32 {
        ek_tex_field_src_file_path,
        ek_tex_field_dest_file_path,

        eks_tex_field_cnt
    };

    static s_static_array<s_asset_field, eks_tex_field_cnt> g_tex_fields = {{
        {"src_file_path", ek_asset_field_type_str},
        {"dest_file_path", ek_asset_field_type_str}
    }};

    enum e_font_field : t_s32 {
        ek_font_field_src_file_path,
        ek_font_field_height,
        ek_font_field_extra_chrs_file_path,
        ek_font_field_dest_file_path,

        eks_font_field_cnt
    };

    static s_static_array<s_asset_field, eks_font_field_cnt> g_font_fields = {{
        {"src_file_path", ek_asset_field_type_str},
        {"height", ek_asset_field_type_num},
        {"extra_chrs_file_path", ek_asset_field_type_str, true},
        {"dest_file_path", ek_asset_field_type_str}
    }};

    enum e_snd_field : t_s32 {
        ek_snd_field_src_file_path,
        ek_snd_field_dest_file_path,

        eks_snd_field_cnt
    };

    static s_static_array<s_asset_field, eks_snd_field_cnt> g_snd_fields = {{
        {"src_file_path", ek_asset_field_type_str},
        {"dest_file_path", ek_asset_field_type_str}
    }};

    t_b8 RunPacker(const s_str_rdonly instrs_json_file_path) {
        s_mem_arena mem_arena;

        if (!AllocMemArena(g_mem_arena_size, mem_arena)) {
            fprintf(stderr, "Failed to allocate memory arena!");
            return false;
        }

        ZF_DEFER({ FreeMemArena(mem_arena); });

        const auto cj = [instrs_json_file_path, &mem_arena]() -> cJSON* {
            s_array<t_u8> instrs_json_file_contents;

            if (!LoadFileContents(instrs_json_file_path, mem_arena, mem_arena, instrs_json_file_contents)) {
                //fprintf(stderr, "Failed to load packing instructions JSON file \"%s\"!", StrRaw(instrs_json_file_path));
                fprintf(stderr, "Failed to load packing instructions JSON file!");
                return nullptr;
            }

            s_str instrs_json_str_terminated;

            if (!CloneStrButAddTerminator({instrs_json_file_contents}, mem_arena, instrs_json_str_terminated)) {
                fprintf(stderr, "Failed to clone packing instructions JSON file contents to null-terminated string!");
                return nullptr;
            }

            return cJSON_Parse(StrRaw(instrs_json_str_terminated));
        }();

        if (!cj) {
            fprintf(stderr, "Failed to parse packing instructions JSON file");
            return false;
        }

        ZF_DEFER({ cJSON_Delete(cj); });

        if (!cJSON_IsObject(cj)) {
            fprintf(stderr, "Packing instructions JSON root is not an object!");
            return false;
        }

        s_static_array<cJSON*, eks_tex_field_cnt> tex_field_cj_ptrs = {};
        s_static_array<cJSON*, eks_font_field_cnt> font_field_cj_ptrs = {};
        s_static_array<cJSON*, eks_snd_field_cnt> snd_field_cj_ptrs = {};

        for (t_size asset_type_index = 0; asset_type_index < eks_asset_type_cnt; asset_type_index++) {
            cJSON* const cj_assets = cJSON_GetObjectItemCaseSensitive(cj, g_asset_type_arr_raw_names[asset_type_index]);

            if (!cJSON_IsArray(cj_assets)) {
                fprintf(stderr, "Packing instructions JSON \"%s\" array does not exist or it is of the wrong type!", g_asset_type_arr_raw_names[asset_type_index]);
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
                    field_vals[fi] = cJSON_GetObjectItem(cj_asset, fields[fi].name_raw);

                    if (!field_vals[fi]) {
                        if (fields[fi].optional) {
                            continue;
                        }

                        fprintf(stderr, "A packing instructions JSON \"%s\" entry is missing required field \"%s\"!", g_asset_type_arr_raw_names[asset_type_index], fields[fi].name_raw);
                        return false;
                    }

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
                        fprintf(stderr, "A packing instructions JSON \"%s\" entry has field \"%s\" as the wrong type! Expected a %s.", g_asset_type_arr_raw_names[asset_type_index], fields[fi].name_raw, g_asset_field_type_raw_names[fields[fi].type]);
                        return false;
                    }
                }

                switch (asset_type_index) {
                    case ek_asset_type_texture:
                        {
                            const auto dest_fp_raw = field_vals[ek_tex_field_dest_file_path]->valuestring;
                            const auto src_fp_raw = field_vals[ek_tex_field_src_file_path]->valuestring;

                            if (!PackTexture(StrFromRaw(dest_fp_raw), StrFromRaw(src_fp_raw), mem_arena)) {
                                fprintf(stderr, "Failed to pack texture \"%s\" to \"%s\"!", src_fp_raw, dest_fp_raw);
                                return false;
                            }
                        }

                        break;

                    case ek_asset_type_font:
                        {
                            const auto dest_fp_raw = field_vals[ek_font_field_dest_file_path]->valuestring;
                            const auto src_fp_raw = field_vals[ek_font_field_src_file_path]->valuestring;
                            const auto height = field_vals[ek_font_field_height]->valueint;
                            const auto extra_chrs_fp_raw = field_vals[ek_font_field_extra_chrs_file_path]->valuestring;

                            const auto code_pts = PushToMemArena<t_unicode_code_pt_bit_vector>(mem_arena);

                            if (!code_pts) {
                                fprintf(stderr, "Failed to allocate code point bit vector for font \"%s\"!", src_fp_raw);
                                return false;
                            }

                            ApplyMask(*code_pts, g_unicode_printable_ascii_code_pts, ek_bitwise_mask_op_or);

                            if (field_vals[ek_font_field_extra_chrs_file_path]) {
                                s_array<t_u8> extra_chrs_file_contents;

                                if (!LoadFileContents(StrFromRaw(extra_chrs_fp_raw), mem_arena, mem_arena, extra_chrs_file_contents)) {
                                    fprintf(stderr, "Failed to load extra characters file \"%s\" for font \"%s\"!", extra_chrs_fp_raw, src_fp_raw);
                                    return false;
                                }

                                MarkStrCodePoints({extra_chrs_file_contents}, *code_pts);
                            }

                            if (!PackFont(StrFromRaw(dest_fp_raw), StrFromRaw(src_fp_raw), height, *code_pts, mem_arena)) {
                                fprintf(stderr, "Failed to pack font \"%s\" to \"%s\"!", src_fp_raw, dest_fp_raw);
                                return false;
                            }
                        }

                        break;

                    case ek_asset_type_sound:
                        {
                            const auto dest_fp_raw = field_vals[ek_snd_field_dest_file_path]->valuestring;
                            const auto src_fp_raw = field_vals[ek_snd_field_src_file_path]->valuestring;

                            if (!PackSound(StrFromRaw(dest_fp_raw), StrFromRaw(src_fp_raw), mem_arena)) {
                                fprintf(stderr, "Failed to pack sound \"%s\" to \"%s\"!", src_fp_raw, dest_fp_raw);
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
