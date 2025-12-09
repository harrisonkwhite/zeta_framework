#include "zap_packing.h"

#include <cJSON.h>

namespace zf {
#if 0
    constexpr t_len g_mem_arena_size = Megabytes(40);

    enum e_asset_type : t_i32 {
        ek_asset_type_texture,
        ek_asset_type_font,
        ek_asset_type_shader_prog,
        ek_asset_type_sound,

        eks_asset_type_cnt
    };

    static s_static_array<s_str_rdonly, eks_asset_type_cnt> g_asset_type_arr_names = {{"textures", "fonts", "shader_progs", "sounds"}};

    enum e_asset_field_type : t_i32 {
        ek_asset_field_type_str,
        ek_asset_field_type_num,

        eks_asset_field_type_cnt
    };

    static s_static_array<s_str_rdonly, eks_asset_field_type_cnt> g_asset_field_type_names = {{"string", "number"}};

    struct s_asset_field {
        s_str_rdonly name;
        e_asset_field_type type;
        t_b8 optional;
    };

    enum e_tex_field : t_i32 {
        ek_tex_field_src_file_path,
        ek_tex_field_dest_file_path,

        eks_tex_field_cnt
    };

    static s_static_array<s_asset_field, eks_tex_field_cnt> g_tex_fields = {{{"src_file_path", ek_asset_field_type_str}, {"dest_file_path", ek_asset_field_type_str}}};

    enum e_font_field : t_i32 {
        ek_font_field_src_file_path,
        ek_font_field_height,
        ek_font_field_extra_chrs_file_path,
        ek_font_field_dest_file_path,

        eks_font_field_cnt
    };

    static s_static_array<s_asset_field, eks_font_field_cnt> g_font_fields = {{{"src_file_path", ek_asset_field_type_str}, {"height", ek_asset_field_type_num}, {"extra_chrs_file_path", ek_asset_field_type_str, true}, {"dest_file_path", ek_asset_field_type_str}}};

    enum e_shader_prog_field : t_i32 {
        ek_shader_prog_field_src_vs_file_path,
        ek_shader_prog_field_src_fs_file_path,
        ek_shader_prog_field_dest_file_path,

        eks_shader_prog_field_cnt
    };

    static s_static_array<s_asset_field, eks_shader_prog_field_cnt> g_shader_prog_fields = {{{"src_vs_file_path", ek_asset_field_type_str}, {"src_fs_file_path", ek_asset_field_type_str}, {"dest_file_path", ek_asset_field_type_str}}};

    enum e_snd_field : t_i32 {
        ek_snd_field_src_file_path,
        ek_snd_field_dest_file_path,

        eks_snd_field_cnt
    };

    static s_static_array<s_asset_field, eks_snd_field_cnt> g_snd_fields = {{{"src_file_path", ek_asset_field_type_str}, {"dest_file_path", ek_asset_field_type_str}}};

    t_b8 RunPacker(const s_str_rdonly instrs_json_file_path) {
        s_mem_arena mem_arena;
        MarkUninitted(&mem_arena);

        if (!CreateMemArena(g_mem_arena_size, &mem_arena)) {
            LogError("Failed to allocate memory arena!");
            return false;
        }

        ZF_DEFER({ DestroyMemArena(&mem_arena); });

        const auto cj = [instrs_json_file_path, &mem_arena]() -> cJSON * {
            s_array<t_u8> instrs_json_file_contents;

            if (!LoadFileContents(instrs_json_file_path, &mem_arena, &instrs_json_file_contents)) {
                LogError("Failed to load packing instructions JSON file \"%\"!", instrs_json_file_path);
                return nullptr;
            }

            return cJSON_Parse(StrRaw(instrs_json_str_terminated));
        }();

        if (!cj) {
            LogError("Failed to parse packing instructions JSON file");
            return false;
        }

        ZF_DEFER({ cJSON_Delete(cj); });

        if (!cJSON_IsObject(cj)) {
            LogError("Packing instructions JSON root is not an object!");
            return false;
        }

        s_static_array<cJSON *, eks_tex_field_cnt> tex_field_cj_ptrs = {};
        s_static_array<cJSON *, eks_font_field_cnt> font_field_cj_ptrs = {};
        s_static_array<cJSON *, eks_shader_prog_field_cnt> shader_prog_field_cj_ptrs = {};
        s_static_array<cJSON *, eks_snd_field_cnt> snd_field_cj_ptrs = {};

        for (t_len asset_type_index = 0; asset_type_index < eks_asset_type_cnt; asset_type_index++) {
            const auto asset_type_arr_name = g_asset_type_arr_names[asset_type_index];
            s_str asset_type_arr_name_terminated;

            if (!CloneStrButAddTerminator(asset_type_arr_name, mem_arena, asset_type_arr_name_terminated)) {
                ZF_REPORT_ERROR();
                return false;
            }

            cJSON *const cj_assets = cJSON_GetObjectItemCaseSensitive(cj, StrRaw(asset_type_arr_name_terminated));

            if (!cJSON_IsArray(cj_assets)) {
                LogError("Packing instructions JSON \"%\" array does not exist or it is of "
                         "the wrong type!",
                         asset_type_arr_name);
                return false;
            }

            cJSON *cj_asset = nullptr;

            cJSON_ArrayForEach(cj_asset, cj_assets) {
                RewindMemArena(&mem_arena, 0);

                if (!cJSON_IsObject(cj_asset)) {
                    continue;
                }

                const auto fields = [asset_type_index]() -> s_array_rdonly<s_asset_field> {
                    switch (asset_type_index) {
                    case ek_asset_type_texture:
                        return g_tex_fields;

                    case ek_asset_type_font:
                        return g_font_fields;

                    case ek_asset_type_shader_prog:
                        return g_shader_prog_fields;

                    case ek_asset_type_sound:
                        return g_snd_fields;
                    }

                    return {};
                }();

                const auto field_vals = [asset_type_index, &tex_field_cj_ptrs, &font_field_cj_ptrs, &shader_prog_field_cj_ptrs, &snd_field_cj_ptrs]() -> s_array<cJSON *> {
                    switch (asset_type_index) {
                    case ek_asset_type_texture:
                        return tex_field_cj_ptrs;

                    case ek_asset_type_font:
                        return font_field_cj_ptrs;

                    case ek_asset_type_shader_prog:
                        return shader_prog_field_cj_ptrs;

                    case ek_asset_type_sound:
                        return snd_field_cj_ptrs;
                    }

                    return {};
                }();

                for (t_len fi = 0; fi < fields.len; fi++) {
                    const auto field_name = fields[fi].name;
                    s_str field_name_terminated;

                    if (!CloneStrButAddTerminator(field_name, mem_arena, field_name_terminated)) {
                        ZF_REPORT_ERROR();
                        return false;
                    }

                    field_vals[fi] = cJSON_GetObjectItem(cj_asset, StrRaw(field_name_terminated));

                    if (!field_vals[fi]) {
                        if (fields[fi].optional) {
                            continue;
                        }

                        LogError("A packing instructions JSON \"%\" entry is missing required "
                                 "field \"%\"!",
                                 asset_type_arr_name, field_name);
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
                        LogError("A packing instructions JSON \"%\" entry has field \"%\" as "
                                 "the wrong type! Expected a %.",
                                 asset_type_arr_name, field_name, g_asset_field_type_names[fields[fi].type]);
                        return false;
                    }
                }

                switch (asset_type_index) {
                case ek_asset_type_texture: {
                    const auto dest_fp = StrFromRaw(field_vals[ek_tex_field_dest_file_path]->valuestring);
                    const auto src_fp = StrFromRaw(field_vals[ek_tex_field_src_file_path]->valuestring);

                    if (!PackTexture(dest_fp, src_fp, mem_arena)) {
                        ZF_REPORT_ERROR();
                        return false;
                    }

                    break;
                }

                case ek_asset_type_font: {
                    const auto dest_fp = StrFromRaw(field_vals[ek_font_field_dest_file_path]->valuestring);
                    const auto src_fp = StrFromRaw(field_vals[ek_font_field_src_file_path]->valuestring);
                    const auto height = field_vals[ek_font_field_height]->valueint;
                    const auto extra_chrs_fp = StrFromRaw(field_vals[ek_font_field_extra_chrs_file_path]->valuestring);

                    s_array<t_unicode_code_pt_bit_vec> code_pt_bvs; // First is for input code points, second is for output
                                                                    // unsupported ones.

                    if (!AllocArray(&code_pt_bvs, 2, &mem_arena)) {
                        ZF_REPORT_ERROR();
                        return false;
                    }

                    SetBitsInRange(code_pt_bvs[0], g_printable_ascii_range_begin,
                                   g_printable_ascii_range_end); // Add the printable ASCII
                                                                 // range as a default.

                    if (field_vals[ek_font_field_extra_chrs_file_path]) {
                        s_array<t_u8> extra_chrs_file_contents;

                        if (!LoadFileContents(extra_chrs_fp, mem_arena, mem_arena, extra_chrs_file_contents)) {
                            LogError("Failed to load extra characters file \"%\" for font \"%\"!", extra_chrs_fp, src_fp);
                            return false;
                        }

                        MarkStrCodePoints({extra_chrs_file_contents}, code_pt_bvs[0]);

                        // Unset irrelevant code points.
                        UnsetBit(code_pt_bvs[0], 10);
                    }

                    e_font_load_from_raw_result load_from_raw_res;

                    if (!PackFont(dest_fp, src_fp, height, code_pt_bvs[0], mem_arena, load_from_raw_res, &code_pt_bvs[1])) {
                        switch (load_from_raw_res) {
                        case ek_font_load_from_raw_result_unsupported_code_pt:
                            LogError("Unsupported code points:");

                            ZF_FOR_EACH_SET_BIT(code_pt_bvs[1], i) {
                                Log("- %", i);
                            }

                            return false;

                        default:
                            break;
                        }

                        ZF_REPORT_ERROR();
                        return false;
                    }

                    break;
                }

                case ek_asset_type_shader_prog: {
                    const auto dest_fp = StrFromRaw(field_vals[ek_shader_prog_field_dest_file_path]->valuestring);
                    const auto vs_fp = StrFromRaw(field_vals[ek_shader_prog_field_src_vs_file_path]->valuestring);
                    const auto fs_fp = StrFromRaw(field_vals[ek_shader_prog_field_src_fs_file_path]->valuestring);

                    if (!PackShaderProg(dest_fp, vs_fp, fs_fp, mem_arena)) {
                        ZF_REPORT_ERROR();
                        return false;
                    }

                    break;
                }

                case ek_asset_type_sound: {
                    const auto dest_fp = StrFromRaw(field_vals[ek_snd_field_dest_file_path]->valuestring);
                    const auto src_fp = StrFromRaw(field_vals[ek_snd_field_src_file_path]->valuestring);

                    s_sound_data snd_data;

                    if (!LoadSoundFromRaw(src_fp, &snd_data, &mem_arena, &mem_arena)) {
                        ZF_REPORT_ERROR();
                        return false;
                    }

                    if (!PackSound(dest_fp, snd_data, &mem_arena)) {
                        ZF_REPORT_ERROR();
                        return false;
                    }

                    break;
                }
                }
            }
        }

        Log("Asset packing completed!");

        return true;
    }
#endif
}
