#include "zfwc_graphics.h"

#include "cu_mem.h"
#include "zfws.h"

s_rect_edges GenTextureCoords(const s_rect_s32 src_rect, const s_v2_s32 tex_size) {
    const s_v2 half_texel = {
        0.5f / static_cast<float>(tex_size.x),
        0.5f / static_cast<float>(tex_size.y)
    };

    return s_rect_edges{
        .left = (static_cast<float>(src_rect.x) + half_texel.x) / static_cast<float>(tex_size.x),
        .top = (static_cast<float>(src_rect.y) + half_texel.y) / static_cast<float>(tex_size.y),
        .right = (static_cast<float>(src_rect.x + src_rect.width) - half_texel.x) / static_cast<float>(tex_size.x),
        .bottom = (static_cast<float>(src_rect.y + src_rect.height) - half_texel.y) / static_cast<float>(tex_size.y)
    };
}

bool LoadRGBATextureFromPackedFile(s_rgba_texture& tex, const c_array<const char> file_path, c_mem_arena& mem_arena) {
    c_file_reader fr;
    fr.DeferClose();

    if (!fr.Open(file_path)) {
        //LOG_ERROR("Failed to open \"%s\"!", file_path.Raw());
        return false;
    }

    if (!fr.ReadItem(tex.tex_size)) {
        //LOG_ERROR("Failed to read texture size from file stream!");
        return false;
    }

    tex.px_data = PushArrayToMemArena<t_u8>(mem_arena, 4 * tex.tex_size.x * tex.tex_size.y);

    if (fr.Read(tex.px_data) < tex.px_data.Len()) {
        //LOG_ERROR("Failed to read RGBA pixel data from file stream!");
        return false;
    }

    return true;
}

static inline s_v2_s32 GLTextureSizeLimit() {
    t_s32 size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
    return {size, size};
}

t_gl_id GenGLTextureFromRGBA(const s_rgba_texture& rgba_tex) {
    const s_v2_s32 tex_size_limit = GLTextureSizeLimit();

    if (rgba_tex.tex_size.x > tex_size_limit.x || rgba_tex.tex_size.y > tex_size_limit.y) {
        //LOG_ERROR("Texture size (%d, %d) exceeds OpenGL limits (%d, %d)!", rgba_tex.tex_size.x, rgba_tex.tex_size.y, tex_size_limit.x, tex_size_limit.y);
        return 0;
    }

    t_gl_id tex_gl_id;
    glGenTextures(1, &tex_gl_id);

    glBindTexture(GL_TEXTURE_2D, tex_gl_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba_tex.tex_size.x, rgba_tex.tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_tex.px_data.Raw());

    return tex_gl_id;
}

bool InitTextureGroup(s_texture_group& texture_group, const t_s32 tex_cnt, const t_texture_group_rgba_loader_func rgba_loader_func, c_mem_arena& mem_arena, s_gl_resource_arena& gl_res_arena, c_mem_arena& temp_mem_arena) {
    //assert(IS_ZERO(texture_group));
    assert(tex_cnt > 0);

    const auto sizes = PushArrayToMemArena<s_v2_s32>(mem_arena, tex_cnt);

    if (sizes.IsEmpty()) {
        //LOG_ERROR("Failed to reserve memory for texture sizes!");
        return false;
    }

    const c_array<t_gl_id> gl_ids = PushArrayToGLResourceArena(gl_res_arena, tex_cnt, ek_gl_resource_type_texture);

    if (gl_ids.IsEmpty()) {
        //LOG_ERROR("Failed to reserve OpenGL texture IDs!");
        return false;
    }

    for (t_s32 i = 0; i < tex_cnt; i++) {
        s_rgba_texture rgba;

        if (!rgba_loader_func(rgba, i, temp_mem_arena)) {
            //LOG_ERROR("Failed to load RGBA texture for texture with index %d!", i);
            return false;
        }

        gl_ids[i] = GenGLTextureFromRGBA(rgba);

        if (!gl_ids[i]) {
            //LOG_ERROR("Failed to generate OpenGL texture for texture with index %d!", i);
            return false;
        }

        sizes[i] = rgba.tex_size;
    }

    texture_group = {
        .sizes = sizes,
        .gl_ids = gl_ids
    };

    return true;
}

void RenderTexture(const s_rendering_context& rendering_context, const s_texture_group& textures, const t_s32 tex_index, const s_rect_s32 src_rect, const s_v2 pos, const s_v2 origin, const s_v2 scale, const float rot, const s_v4 blend) {
    assert(origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f);

    const s_v2_s32 tex_size = textures.sizes[tex_index];

    s_rect_s32 src_rect_to_use;

    /*if (src_rect == (s_rect_s32){}) {
        src_rect_to_use = {0, 0, tex_size.x, tex_size.y};
    } else {
        src_rect_to_use = src_rect;
        assert(src_rect.x + src_rect.width <= tex_size.x && src_rect.y + src_rect.height <= tex_size.y);
    }*/

    assert(false);

    const s_batch_slot_write_info write_info = {
        .tex_gl_id = textures.gl_ids[tex_index],
        .tex_coords = GenTextureCoords(src_rect_to_use, tex_size),
        .pos = pos,
        .size = {src_rect_to_use.width * scale.x, src_rect_to_use.height * scale.y},
        .origin = origin,
        .rot = rot,
        .blend = blend
    };

    Render(rendering_context, write_info);
}

static inline s_rect InnerRect(const s_rect rect, const float outline_thickness) {
    return s_rect{
        rect.x + outline_thickness,
        rect.y + outline_thickness,
        rect.width - (outline_thickness * 2.0f),
        rect.height - (outline_thickness * 2.0f)
    };
}

void RenderRectWithOutline(const s_rendering_context& rendering_context, const s_rect rect, const s_v4 fill_color, const s_v4 outline_color, const float outline_thickness) {
    // Top Outline
    RenderRect(rendering_context, {rect.x, rect.y, rect.width - outline_thickness, outline_thickness}, outline_color);

    // Right Outline
    RenderRect(rendering_context, {rect.x + rect.width - outline_thickness, rect.y, outline_thickness, rect.height - outline_thickness}, outline_color);

    // Bottom Outline
    RenderRect(rendering_context, {rect.x + outline_thickness, rect.y + rect.height - outline_thickness, rect.width - outline_thickness, outline_thickness}, outline_color);

    // Left Outline
    RenderRect(rendering_context, {rect.x, rect.y + outline_thickness, outline_thickness, rect.height - outline_thickness}, outline_color);

    // Inside
    RenderRect(rendering_context, InnerRect(rect, outline_thickness), fill_color);
}

void RenderRectWithOutlineAndOpaqueFill(const s_rendering_context& rendering_context, const s_rect rect, const s_v3 fill_color, const s_v4 outline_color, const float outline_thickness) {
    // Outline
    RenderRect(rendering_context, rect, outline_color);

    // Inside
    RenderRect(rendering_context, InnerRect(rect, outline_thickness), s_v4{fill_color.x, fill_color.y, fill_color.z, 1.0f});
}

void RenderBarHor(const s_rendering_context& rendering_context, const s_rect rect, const float perc, const s_v4 front_color, const s_v4 bg_color) {
    assert(perc >= 0.0f && perc <= 1.0f);

    const float front_rect_width = rect.width * perc;

    if (front_rect_width > 0.0f) {
        RenderRect(rendering_context, s_rect{rect.x, rect.y, front_rect_width, rect.height}, front_color);
    }

    const float bg_rect_x = rect.x + front_rect_width;
    const float bg_rect_width = rect.width - front_rect_width;

    if (bg_rect_width > 0.0f) {
        RenderRect(rendering_context, s_rect{bg_rect_x, rect.y, bg_rect_width, rect.height}, bg_color);
    }
}

void RenderBarVertical(const s_rendering_context& rendering_context, const s_rect rect, const float perc, const s_v4 front_color, const s_v4 bg_color) {
    assert(perc >= 0.0f && perc <= 1.0f);

    const float front_rect_height = rect.height * perc;

    if (front_rect_height > 0.0f) {
        RenderRect(rendering_context, s_rect{rect.x, rect.y, rect.width, front_rect_height}, front_color);
    }

    const float bg_rect_y = rect.y + front_rect_height;
    const float bg_rect_height = rect.height - front_rect_height;

    if (bg_rect_height > 0.0f) {
        RenderRect(rendering_context, s_rect{rect.x, bg_rect_y, rect.width, bg_rect_height}, bg_color);
    }
}
