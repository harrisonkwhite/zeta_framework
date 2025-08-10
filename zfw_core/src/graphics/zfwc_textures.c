#include "zfwc_graphics.h"

s_rect_edges GenTextureCoords(const s_rect_s32 src_rect, const s_v2_s32 tex_size) {
    return (s_rect_edges){
        .left = (float)src_rect.x / tex_size.x,
        .top = (float)src_rect.y / tex_size.y,
        .right = (float)(src_rect.x + src_rect.width) / tex_size.x,
        .bottom = (float)(src_rect.y + src_rect.height) / tex_size.y
    };
}

static s_rgba_texture LoadRGBATextureFromFS(FILE* const fs, s_mem_arena *const mem_arena) {
    s_v2_s32 tex_size;

    if (fread(&tex_size, sizeof(tex_size), 1, fs) < 1) {
        return (s_rgba_texture){0};
    }

    if (tex_size.x <= 0 || tex_size.y <= 0) {
        return (s_rgba_texture){0};
    }

    const s_u8_array px_data = PushU8ArrayToMemArena(mem_arena, 4 * tex_size.x * tex_size.y);

    if (fread(px_data.buf_raw, 1, px_data.len, fs) < px_data.len) {
        return (s_rgba_texture){0};
    }

    return (s_rgba_texture){
        .tex_size = tex_size,
        .px_data = px_data
    };
}

s_rgba_texture LoadRGBATextureFromFile(const s_char_array_view file_path, s_mem_arena *const mem_arena) {
    FILE* const fs = fopen(file_path.buf_raw, "rb");

    if (!fs) {
        return (s_rgba_texture){0};
    }

    const s_rgba_texture tex = LoadRGBATextureFromFS(fs, mem_arena);

    fclose(fs);

    return tex;
}

static inline s_v2_s32 GLTextureSizeLimit() {
    t_s32 size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
    return (s_v2_s32){size, size};
}

t_gl_id GenGLTextureFromRGBA(const s_rgba_texture rgba_tex) {
    const s_v2_s32 tex_size_limit = GLTextureSizeLimit();

    if (rgba_tex.tex_size.x > tex_size_limit.x || rgba_tex.tex_size.y > tex_size_limit.y) {
        LOG_ERROR("Texture size (%d, %d) exceeds OpenGL limits (%d, %d)!", rgba_tex.tex_size.x, rgba_tex.tex_size.y, tex_size_limit.x, tex_size_limit.y);
        return 0;
    }

    t_gl_id tex_gl_id;
    glGenTextures(1, &tex_gl_id);

    glBindTexture(GL_TEXTURE_2D, tex_gl_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba_tex.tex_size.x, rgba_tex.tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_tex.px_data.buf_raw);

    return tex_gl_id;
}

s_texture_group GenTextureGroup(const t_s32 tex_cnt, const t_texture_group_rgba_generator_func rgba_generator_func, s_mem_arena *const mem_arena, s_gl_resource_arena* const gl_res_arena, s_mem_arena* const temp_mem_arena) {
    assert(tex_cnt > 0);

    const s_v2_s32_array sizes = PushV2S32ArrayToMemArena(mem_arena, tex_cnt);

    if (IS_ZERO(sizes)) {
        LOG_ERROR("Failed to reserve memory for texture sizes!");
        return (s_texture_group){0};
    }

    const s_gl_id_array gl_ids = PushToGLResourceArena(gl_res_arena, tex_cnt, ek_gl_resource_type_texture);

    if (IS_ZERO(gl_ids)) {
        LOG_ERROR("Failed to reserve OpenGL texture IDs!");
        return (s_texture_group){0};
    }

    for (t_s32 i = 0; i < tex_cnt; i++) {
        const s_rgba_texture rgba_tex = rgba_generator_func(i, temp_mem_arena);

        if (IS_ZERO(rgba_tex)) {
            LOG_ERROR("Failed to generate RGBA texture for texture with index %d!", i);
            return (s_texture_group){0};
        }

        t_gl_id* const gl_id = GLIDElem(gl_ids, i);
        *gl_id = GenGLTextureFromRGBA(rgba_tex);

        if (!*gl_id) {
            LOG_ERROR("Failed to generate OpenGL texture for texture with index %d!", i);
            return (s_texture_group){0};
        }

        *V2S32Elem(sizes, i) = rgba_tex.tex_size;
    }

    return (s_texture_group){
        .sizes = V2S32ArrayView(sizes),
        .gl_ids = GLIDArrayView(gl_ids)
    };
}

void RenderTexture(const s_rendering_context* const rendering_context, const s_texture_group* const textures, const t_s32 tex_index, const s_rect_s32 src_rect, const s_v2 pos, const s_v2 origin, const s_v2 scale, const float rot, const u_v4 blend) {
    assert(origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f);

    const s_v2_s32 tex_size = *V2S32ElemView(textures->sizes, tex_index);

    s_rect_s32 src_rect_to_use;

    if (IS_ZERO(src_rect)) {
        src_rect_to_use = (s_rect_s32){0, 0, tex_size.x, tex_size.y};
    } else {
        src_rect_to_use = src_rect;
        assert(src_rect.x + src_rect.width <= tex_size.x && src_rect.y + src_rect.height <= tex_size.y);
    }

    const s_batch_slot_write_info write_info = {
        .tex_gl_id = *GLIDElemView(textures->gl_ids, tex_index),
        .tex_coords = GenTextureCoords(src_rect_to_use, tex_size),
        .pos = pos,
        .size = {src_rect_to_use.width * scale.x, src_rect_to_use.height * scale.y},
        .origin = origin,
        .rot = rot,
        .blend = blend
    };

    Render(rendering_context, &write_info);
}
