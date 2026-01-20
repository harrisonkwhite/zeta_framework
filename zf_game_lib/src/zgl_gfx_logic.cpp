#include <zgl/zgl_gfx.h>

namespace zgl {
    t_gfx_resource *TextureCreateFromExternal(t_gfx *const gfx, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
        zcl::t_texture_data_mut texture_data;

        if (!zcl::TextureLoadFromExternal(file_path, temp_arena, temp_arena, &texture_data)) {
            ZCL_FATAL();
        }

        return TextureCreate(gfx, texture_data, group);
    }

    t_gfx_resource *TextureCreateFromPacked(t_gfx *const gfx, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
        zcl::t_file_stream file_stream;

        if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
            ZCL_FATAL();
        }

        zcl::t_texture_data_mut texture_data;

        if (!zcl::DeserializeTexture(file_stream, temp_arena, &texture_data)) {
            ZCL_FATAL();
        }

        zcl::FileClose(&file_stream);

        return TextureCreate(gfx, texture_data, group);
    }

    t_gfx_resource *ShaderProgCreateFromPacked(t_gfx *const gfx, const zcl::t_str_rdonly vert_shader_file_path, const zcl::t_str_rdonly frag_shader_file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
        zcl::t_array_mut<zcl::t_u8> vert_shader_compiled_bin;

        {
            zcl::t_file_stream vert_shader_file_stream;

            if (!zcl::FileOpen(vert_shader_file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &vert_shader_file_stream)) {
                ZCL_FATAL();
            }

            if (!zcl::DeserializeShader(vert_shader_file_stream, temp_arena, &vert_shader_compiled_bin)) {
                ZCL_FATAL();
            }

            zcl::FileClose(&vert_shader_file_stream);
        }

        zcl::t_array_mut<zcl::t_u8> frag_shader_compiled_bin;

        {
            zcl::t_file_stream frag_shader_file_stream;

            if (!zcl::FileOpen(frag_shader_file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &frag_shader_file_stream)) {
                ZCL_FATAL();
            }

            if (!zcl::DeserializeShader(frag_shader_file_stream, temp_arena, &frag_shader_compiled_bin)) {
                ZCL_FATAL();
            }

            zcl::FileClose(&frag_shader_file_stream);
        }

        return ShaderProgCreate(gfx, vert_shader_compiled_bin, frag_shader_compiled_bin, group);
    }

    t_font FontCreateFromExternal(t_gfx *const gfx, const zcl::t_str_rdonly file_path, const zcl::t_i32 height, zcl::t_code_point_bitset *const code_pts, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena) {
        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<zcl::t_font_atlas_rgba> atlas_rgbas;

        if (!zcl::FontLoadFromExternal(file_path, height, code_pts, GFXResourceGroupGetArena(gfx, resource_group), temp_arena, temp_arena, &arrangement, &atlas_rgbas)) {
            ZCL_FATAL();
        }

        const zcl::t_array_mut<t_gfx_resource *> atlases = zcl::ArenaPushArray<t_gfx_resource *>(GFXResourceGroupGetArena(gfx, resource_group), atlas_rgbas.len);

        for (zcl::t_i32 i = 0; i < atlas_rgbas.len; i++) {
            atlases[i] = TextureCreate(gfx, {zcl::k_font_atlas_size, atlas_rgbas[i]}, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    t_font FontCreateFromPacked(t_gfx *const gfx, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena) {
        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<zcl::t_font_atlas_rgba> atlas_rgbas;

        {
            zcl::t_file_stream file_stream;

            if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
                ZCL_FATAL();
            }

            if (!zcl::DeserializeFont(file_stream, GFXResourceGroupGetArena(gfx, resource_group), temp_arena, temp_arena, &arrangement, &atlas_rgbas)) {
                ZCL_FATAL();
            }

            zcl::FileClose(&file_stream);
        }

        const auto atlases = zcl::ArenaPushArray<t_gfx_resource *>(GFXResourceGroupGetArena(gfx, resource_group), atlas_rgbas.len);

        for (zcl::t_i32 i = 0; i < atlas_rgbas.len; i++) {
            atlases[i] = TextureCreate(gfx, {zcl::k_font_atlas_size, atlas_rgbas[i]}, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }
}
