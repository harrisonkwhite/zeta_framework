#include "zf_rendering.h"

namespace zf {
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
            ZF_LOG_ERROR("Failed to open \"%s\"!", file_path.Raw());
            return false;
        }

        if (!fr.ReadItem(tex.tex_size)) {
            ZF_LOG_ERROR("Failed to read texture size from file stream!");
            return false;
        }

        tex.px_data = PushArrayToMemArena<t_u8>(mem_arena, 4 * tex.tex_size.x * tex.tex_size.y);

        if (fr.Read(tex.px_data) < tex.px_data.Len()) {
            ZF_LOG_ERROR("Failed to read RGBA pixel data from file stream!");
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
            ZF_LOG_ERROR("Texture size (%d, %d) exceeds OpenGL limits (%d, %d)!", rgba_tex.tex_size.x, rgba_tex.tex_size.y, tex_size_limit.x, tex_size_limit.y);
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
            ZF_LOG_ERROR("Failed to reserve memory for texture sizes!");
            return false;
        }

        const c_array<t_gl_id> gl_ids = PushArrayToGLResourceArena(gl_res_arena, tex_cnt, ek_gl_resource_type_texture);

        if (gl_ids.IsEmpty()) {
            ZF_LOG_ERROR("Failed to reserve OpenGL texture IDs!");
            return false;
        }

        for (t_s32 i = 0; i < tex_cnt; i++) {
            s_rgba_texture rgba;

            if (!rgba_loader_func(rgba, i, temp_mem_arena)) {
                ZF_LOG_ERROR("Failed to load RGBA texture for texture with index %d!", i);
                return false;
            }

            gl_ids[i] = GenGLTextureFromRGBA(rgba);

            if (!gl_ids[i]) {
                ZF_LOG_ERROR("Failed to generate OpenGL texture for texture with index %d!", i);
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
}
