#include "zfwc_graphics.h"

s_rect_edges GenTextureCoords(const s_rect_s32 src_rect, const s_v2_s32 tex_size) {
    const s_v2 half_texel = {
        0.5f / (t_r32)tex_size.x,
        0.5f / (t_r32)tex_size.y
    };

    return (s_rect_edges){
        .left = ((t_r32)src_rect.x + half_texel.x) / (t_r32)tex_size.x,
        .top = ((t_r32)src_rect.y + half_texel.y) / (t_r32)tex_size.y,
        .right = ((t_r32)(src_rect.x + src_rect.width)  - half_texel.x) / (t_r32)tex_size.x,
        .bottom = ((t_r32)(src_rect.y + src_rect.height) - half_texel.y) / (t_r32)tex_size.y
    };
}

static s_rgba_texture LoadRGBATextureFromPackedFS(FILE* const fs, s_mem_arena *const mem_arena) {
    s_v2_s32 tex_size;

    if (fread(&tex_size, sizeof(tex_size), 1, fs) < 1) {
        LOG_ERROR("Failed to read texture size from file stream!");
        return (s_rgba_texture){0};
    }

    const s_u8_array px_data = PushU8ArrayToMemArena(mem_arena, 4 * tex_size.x * tex_size.y);

    if (fread(px_data.buf_raw, 1, px_data.elem_cnt, fs) < px_data.elem_cnt) {
        LOG_ERROR("Failed to read RGBA pixel data from file stream!");
        return (s_rgba_texture){0};
    }

    return (s_rgba_texture){
        .tex_size = tex_size,
        .px_data = px_data
    };
}

s_rgba_texture LoadRGBATextureFromPackedFile(const s_char_array_view file_path, s_mem_arena *const mem_arena) {
    FILE* const fs = fopen(file_path.buf_raw, "rb");

    if (!fs) {
        LOG_ERROR("Failed to open \"%s\"!", file_path.buf_raw);
        return (s_rgba_texture){0};
    }

    const s_rgba_texture tex = LoadRGBATextureFromPackedFS(fs, mem_arena);

    if (IS_ZERO(tex)) {
        LOG_ERROR("Failed to load RGBA texture from packed texture file \"%s\"!", file_path.buf_raw);
    }

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

bool InitTextureGroup(s_texture_group* const texture_group, const t_s32 tex_cnt, const t_texture_group_rgba_generator_func rgba_generator_func, s_mem_arena *const mem_arena, s_gl_resource_arena* const gl_res_arena, s_mem_arena* const temp_mem_arena) {
    assert(IS_ZERO(*texture_group));
    assert(tex_cnt > 0);

    const s_v2_s32_array sizes = PushV2S32ArrayToMemArena(mem_arena, tex_cnt);

    if (IS_ZERO(sizes)) {
        LOG_ERROR("Failed to reserve memory for texture sizes!");
        return false;
    }

    const s_gl_id_array gl_ids = PushToGLResourceArena(gl_res_arena, tex_cnt, ek_gl_resource_type_texture);

    if (IS_ZERO(gl_ids)) {
        LOG_ERROR("Failed to reserve OpenGL texture IDs!");
        return false;
    }

    for (t_s32 i = 0; i < tex_cnt; i++) {
        const s_rgba_texture rgba_tex = rgba_generator_func(i, temp_mem_arena);

        if (IS_ZERO(rgba_tex)) {
            LOG_ERROR("Failed to generate RGBA texture for texture with index %d!", i);
            return false;
        }

        t_gl_id* const gl_id = GLIDElem(gl_ids, i);
        *gl_id = GenGLTextureFromRGBA(rgba_tex);

        if (!*gl_id) {
            LOG_ERROR("Failed to generate OpenGL texture for texture with index %d!", i);
            return false;
        }

        *V2S32Elem(sizes, i) = rgba_tex.tex_size;
    }

    *texture_group = (s_texture_group){
        .sizes = V2S32ArrayView(sizes),
        .gl_ids = GLIDArrayView(gl_ids)
    };

    return true;
}

void RenderTexture(const s_rendering_context* const rendering_context, const s_texture_group* const textures, const t_s32 tex_index, const s_rect_s32 src_rect, const s_v2 pos, const s_v2 origin, const s_v2 scale, const t_r32 rot, const u_v4 blend) {
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

static inline s_rect InnerRect(const s_rect rect, const t_r32 outline_thickness) {
    return (s_rect){
        rect.x + outline_thickness,
        rect.y + outline_thickness,
        rect.width - (outline_thickness * 2.0f),
        rect.height - (outline_thickness * 2.0f)
    };
}

void RenderRectWithOutline(const s_rendering_context* const rendering_context, const s_rect rect, const u_v4 fill_color, const u_v4 outline_color, const t_r32 outline_thickness) {
    // Top Outline
    RenderRect(rendering_context, (s_rect){rect.x, rect.y, rect.width - outline_thickness, outline_thickness}, outline_color);

    // Right Outline
    RenderRect(rendering_context, (s_rect){rect.x + rect.width - outline_thickness, rect.y, outline_thickness, rect.height - outline_thickness}, outline_color);

    // Bottom Outline
    RenderRect(rendering_context, (s_rect){rect.x + outline_thickness, rect.y + rect.height - outline_thickness, rect.width - outline_thickness, outline_thickness}, outline_color);

    // Left Outline
    RenderRect(rendering_context, (s_rect){rect.x, rect.y + outline_thickness, outline_thickness, rect.height - outline_thickness}, outline_color);

    // Inside
    RenderRect(rendering_context, InnerRect(rect, outline_thickness), fill_color);
}

void RenderRectWithOutlineAndOpaqueFill(const s_rendering_context* const rendering_context, const s_rect rect, const u_v3 fill_color, const u_v4 outline_color, const t_r32 outline_thickness) {
    // Outline
    RenderRect(rendering_context, rect, outline_color);

    // Inside
    RenderRect(rendering_context, InnerRect(rect, outline_thickness), (u_v4){fill_color.r, fill_color.g, fill_color.b, 1.0f});
}

void RenderBarHor(const s_rendering_context* const rendering_context, const s_rect rect, const t_r32 perc, const u_v4 front_color, const u_v4 bg_color) {
    assert(perc >= 0.0f && perc <= 1.0f);

    const t_r32 front_rect_width = rect.width * perc;

    if (front_rect_width > 0.0f) {
        RenderRect(rendering_context, (s_rect){rect.x, rect.y, front_rect_width, rect.height}, front_color);
    }

    const t_r32 bg_rect_x = rect.x + front_rect_width;
    const t_r32 bg_rect_width = rect.width - front_rect_width;

    if (bg_rect_width > 0.0f) {
        RenderRect(rendering_context, (s_rect){bg_rect_x, rect.y, bg_rect_width, rect.height}, bg_color);
    }
}

void RenderBarVertical(const s_rendering_context* const rendering_context, const s_rect rect, const t_r32 perc, const u_v4 front_color, const u_v4 bg_color) {
    assert(perc >= 0.0f && perc <= 1.0f);

    const t_r32 front_rect_height = rect.height * perc;

    if (front_rect_height > 0.0f) {
        RenderRect(rendering_context, (s_rect){rect.x, rect.y, rect.width, front_rect_height}, front_color);
    }

    const t_r32 bg_rect_y = rect.y + front_rect_height;
    const t_r32 bg_rect_height = rect.height - front_rect_height;

    if (bg_rect_height > 0.0f) {
        RenderRect(rendering_context, (s_rect){rect.x, bg_rect_y, rect.width, bg_rect_height}, bg_color);
    }
}
