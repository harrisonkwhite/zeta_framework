#include <zgl/zgl_gfx_private.h>

namespace zgl {
    t_gfx_resource *TextureCreateFromExternal(const t_gfx_ticket_mut gfx_ticket, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(gfx_ticket));

        zcl::t_texture_data_mut texture_data;

        if (!zcl::TextureLoadFromExternal(file_path, temp_arena, temp_arena, &texture_data)) {
            ZCL_FATAL();
        }

        return TextureCreate(gfx_ticket, texture_data, group);
    }

    t_gfx_resource *TextureCreateFromPacked(const t_gfx_ticket_mut gfx_ticket, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(gfx_ticket));

        zcl::t_file_stream file_stream;

        if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
            ZCL_FATAL();
        }

        zcl::t_texture_data_mut texture_data;

        if (!zcl::DeserializeTexture(file_stream, temp_arena, &texture_data)) {
            ZCL_FATAL();
        }

        zcl::FileClose(&file_stream);

        return TextureCreate(gfx_ticket, texture_data, group);
    }

    t_gfx_resource *ShaderProgCreateFromPacked(const t_gfx_ticket_mut gfx_ticket, const zcl::t_str_rdonly vert_shader_file_path, const zcl::t_str_rdonly frag_shader_file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(gfx_ticket));

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

        return ShaderProgCreate(gfx_ticket, vert_shader_compiled_bin, frag_shader_compiled_bin, group);
    }

    t_font FontCreateFromExternal(const t_gfx_ticket_mut gfx_ticket, const zcl::t_str_rdonly file_path, const zcl::t_i32 height, zcl::t_code_point_bitset *const code_pts, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(gfx_ticket));

        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<zcl::t_font_atlas_pixels_r8> atlas_pixels_arr;

        if (!zcl::FontLoadFromExternal(file_path, height, code_pts, GFXResourceGroupGetArena(gfx_ticket, resource_group), temp_arena, temp_arena, &arrangement, &atlas_pixels_arr)) {
            ZCL_FATAL();
        }

        const zcl::t_array_mut<t_gfx_resource *> atlas_textures = zcl::ArenaPushArray<t_gfx_resource *>(GFXResourceGroupGetArena(gfx_ticket, resource_group), atlas_pixels_arr.len);

        for (zcl::t_i32 i = 0; i < atlas_pixels_arr.len; i++) {
            const zcl::t_texture_data_rdonly atlas_texture_data = {
                .dims = zcl::k_font_atlas_size,
                .format = zcl::ek_texture_format_r8,
                .pixels = {.r8 = atlas_pixels_arr[i]},
            };

            atlas_textures[i] = TextureCreate(gfx_ticket, atlas_texture_data, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlas_textures = atlas_textures,
        };
    }

    t_font FontCreateFromPacked(const t_gfx_ticket_mut gfx_ticket, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(gfx_ticket));

        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<zcl::t_font_atlas_pixels_r8> atlas_pixels_arr;

        {
            zcl::t_file_stream file_stream;

            if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
                ZCL_FATAL();
            }

            if (!zcl::DeserializeFont(file_stream, GFXResourceGroupGetArena(gfx_ticket, resource_group), temp_arena, temp_arena, &arrangement, &atlas_pixels_arr)) {
                ZCL_FATAL();
            }

            zcl::FileClose(&file_stream);
        }

        const auto atlas_textures = zcl::ArenaPushArray<t_gfx_resource *>(GFXResourceGroupGetArena(gfx_ticket, resource_group), atlas_pixels_arr.len);

        for (zcl::t_i32 i = 0; i < atlas_pixels_arr.len; i++) {
            const zcl::t_texture_data_rdonly atlas_texture_data = {
                .dims = zcl::k_font_atlas_size,
                .format = zcl::ek_texture_format_r8,
                .pixels = {.r8 = atlas_pixels_arr[i]},
            };

            atlas_textures[i] = TextureCreate(gfx_ticket, atlas_texture_data, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlas_textures = atlas_textures,
        };
    }
}
