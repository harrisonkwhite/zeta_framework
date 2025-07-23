#include <stb_image.h>
#include "zfw_rendering.h"

bool ZFWLoadTexturesFromFiles(zfw_s_textures* const textures, zfw_s_mem_arena* const mem_arena, const int tex_cnt, const zfw_t_texture_index_to_file_path tex_index_to_fp) {
    assert(textures && ZFW_IS_ZERO(*textures));
    assert(mem_arena && ZFWIsMemArenaValid(mem_arena));
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

        unsigned char* const px_data = stbi_load(fp, &sizes[i].x, &sizes[i].y, NULL, ZFW_TEXTURE_CHANNEL_CNT);

        if (!px_data) {
            fprintf(stderr, "Failed to load image \"%s\"! STB Error: %s\n", fp, stbi_failure_reason());
            return false;
        }

        // TODO: Extract into helper function.
        glBindTexture(GL_TEXTURE_2D, gl_ids[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizes[i].x, sizes[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data);

        stbi_image_free(px_data);
    }

    *textures = (zfw_s_textures){
        .gl_ids = gl_ids,
        .sizes = sizes,
        .cnt = tex_cnt
    };

    return true;
}

void ZFWUnloadTextures(zfw_s_textures* const textures) {
    if (textures->gl_ids && textures->cnt > 0) {
        glDeleteTextures(textures->cnt, textures->gl_ids);
    }

    ZFW_ZERO_OUT(*textures);
}
