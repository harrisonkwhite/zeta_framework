#include "zfw_graphics.h"

#include <stb_image.h>
#include "zfw_io.h"

void ZFW_SetUpTexture(const zfw_t_gl_id tex_gl_id, const zfw_s_vec_2d_i tex_size, const zfw_t_byte* const rgba_px_data) {
    assert(tex_gl_id != 0);
    assert(tex_size.x > 0 && tex_size.y > 0);
    assert(rgba_px_data);

    glBindTexture(GL_TEXTURE_2D, tex_gl_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size.x, tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_px_data);
}

bool ZFW_LoadTexturesFromFiles(zfw_s_textures* const textures, zfw_s_mem_arena* const mem_arena, const int tex_cnt, const zfw_t_texture_index_to_file_path tex_index_to_fp) {
    assert(textures && ZFW_IS_ZERO(*textures));
    assert(mem_arena && ZFW_IsMemArenaValid(mem_arena));
    assert(tex_cnt > 0);
    assert(tex_index_to_fp);

    // Reserve buffers.
    zfw_t_gl_id* const gl_ids = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_t_gl_id, tex_cnt);

    if (!gl_ids) {
        return false;
    }

    zfw_s_vec_2d_i* const sizes = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_s_vec_2d_i, tex_cnt);

    if (!sizes) {
        return false;
    }

    // Generate each texture using the pixel data of the file mapped to with the given function pointer.
    glGenTextures(tex_cnt, gl_ids);

    for (int i = 0; i < tex_cnt; ++i) {
        const char* const fp = tex_index_to_fp(i); // Using a function pointer instead of an array of strings for safety.
        assert(fp);

        unsigned char* const px_data = stbi_load(fp, &sizes[i].x, &sizes[i].y, NULL, ZFW_TEXTURE_CHANNEL_CNT);

        if (!px_data) {
            ZFW_LogError("Failed to load image \"%s\"!", fp);
            return false;
        }

        ZFW_SetUpTexture(gl_ids[i], sizes[i], px_data);

        stbi_image_free(px_data);
    }

    *textures = (zfw_s_textures){
        .gl_ids = gl_ids,
        .sizes = sizes,
        .cnt = tex_cnt
    };

    return true;
}

void ZFW_UnloadTextures(zfw_s_textures* const textures) {
    assert(textures && ZFW_IsTexturesValid(textures));

    if (textures->cnt > 0) {
        glDeleteTextures(textures->cnt, textures->gl_ids);
    }

    ZFW_ZERO_OUT(*textures);
}

void ZFW_RenderTexture(const zfw_s_rendering_context* const context, const int tex_index, const zfw_s_textures* const textures, const zfw_s_rect_i src_rect, const zfw_s_vec_2d pos, const zfw_s_vec_2d origin, const zfw_s_vec_2d scale, const float rot, const zfw_s_vec_4d blend) {
    // TODO: Add assertions.

    const zfw_s_batch_slot_write_info write_info = {
        .tex_gl_id = textures->gl_ids[tex_index],
        .tex_coords = ZFW_TextureCoords(src_rect, textures->sizes[tex_index]),
        .pos = pos,
        .size = {src_rect.width * scale.x, src_rect.height * scale.y},
        .origin = origin,
        .rot = rot,
        .blend = blend
    };

    ZFW_Render(context, &write_info);
}

zfw_s_rect_edges ZFW_TextureCoords(const zfw_s_rect_i src_rect, const zfw_s_vec_2d_i tex_size) {
    assert(ZFW_IsSrcRectValid(src_rect, tex_size));
    assert(tex_size.x > 0 && tex_size.y > 0);

    return (zfw_s_rect_edges){
        .left = (float)src_rect.x / tex_size.x,
        .top = (float)src_rect.y / tex_size.y,
        .right = (float)ZFW_RectIRight(src_rect) / tex_size.x,
        .bottom = (float)ZFW_RectIBottom(src_rect) / tex_size.y
    };
}
