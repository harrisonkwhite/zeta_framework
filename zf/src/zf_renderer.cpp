#include <zf/zf_renderer.h>

#include <glad/glad.h>

namespace zf::renderer {
    using t_gl_id = GLuint;

    constexpr t_size g_texture_limit = 1024;

    struct {
        s_static_array<t_gl_id, g_texture_limit> gl_ids;
        s_static_array<s_v2<t_s32>, g_texture_limit> sizes;
        s_static_bit_vec<g_texture_limit> activity;
    } g_textures;

    static inline s_v2<t_s32> GLTextureSizeLimit() {
        t_s32 size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
        return { size, size };
    }

    s_resource_hdl CreateTexture(const s_rgba_texture_data_rdonly& tex_data) {
        const t_size index = IndexOfFirstUnsetBit(g_textures.activity);

        if (index == -1) {
            LogError("Out of room - no available slots for new texture!");
            return {};
        }

        const s_v2<t_s32> tex_size_limit = GLTextureSizeLimit();

        if (tex_data.size_in_pxs.x > tex_size_limit.x || tex_data.size_in_pxs.y > tex_size_limit.y) {
            LogError("Texture size % exceeds limits %!", tex_data.size_in_pxs, tex_size_limit);
            return {};
        }

        t_gl_id gl_id;
        glGenTextures(1, &gl_id);

        glBindTexture(GL_TEXTURE_2D, gl_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_data.size_in_pxs.x, tex_data.size_in_pxs.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.px_data.buf_raw);

        g_textures.gl_ids[index] = gl_id;
        g_textures.sizes[index] = tex_data.size_in_pxs;

        return {ek_resource_type_texture, index};
    }
}
