#include <zgl/zgl_gfx.h>

#include <bgfx/bgfx.h>
#include <zgl/zgl_platform.h>

namespace zf {
    // ============================================================
    // @section: Types and Declarations
    // ============================================================
    enum e_state {
        ek_state_uninitted,
        ek_state_initted,
        ek_state_rendering
    };

    struct {
        e_state state = ek_state_uninitted;
        s_v2_i resolution_cache;
        s_gfx_resource_arena perm_resource_arena;
    } g_state;

    static bgfx::ProgramHandle CreateBGFXShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin);
    static s_gfx_resource &PushGFXResource(const e_gfx_resource_type type, s_gfx_resource_arena &arena);
    [[nodiscard]] static t_b8 CreateFontResource(const s_font_arrangement &arrangement, const s_array<t_font_atlas_rgba> atlas_rgbas, s_gfx_resource_arena &arena, s_ptr<s_gfx_resource> &o_resource);

    struct s_gfx_resource {
    public:
        e_gfx_resource_type type = ek_gfx_resource_type_invalid;
        s_ptr<s_gfx_resource> next = nullptr;

        s_gfx_resource() = default;
        s_gfx_resource(const s_gfx_resource &) = delete;

        auto &Texture() {
            ZF_ASSERT(type == ek_gfx_resource_type_texture);
            return type_data.texture;
        }

        auto &Texture() const {
            ZF_ASSERT(type == ek_gfx_resource_type_texture);
            return type_data.texture;
        }

        auto &Font() {
            ZF_ASSERT(type == ek_gfx_resource_type_font);
            return type_data.font;
        }

        auto &Font() const {
            ZF_ASSERT(type == ek_gfx_resource_type_font);
            return type_data.font;
        }

    private:
        union {
            struct {
                bgfx::TextureHandle bgfx_hdl;
                s_v2_i size;
            } texture;

            struct {
                s_font_arrangement arrangement;
                s_array<s_ptr<s_gfx_resource>> atlases;
            } font;
        } type_data = {};
    };

    // @todo: "Batch" is probably not the right name for this.
    struct s_batch_vert {
        s_v2 pos;
        s_color_rgba32f blend;
        s_v2 uv;
    };

    extern const t_u8 g_batch_vert_shader_src_raw[];
    extern const t_i32 g_batch_vert_shader_src_len;

    extern const t_u8 g_batch_frag_shader_src_raw[];
    extern const t_i32 g_batch_frag_shader_src_len;

    constexpr t_i32 g_batch_vert_limit_per_frame = 8192; // @todo: This should definitely be modifiable if the user wants.

    struct s_rendering_basis {
        bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl;
        bgfx::ProgramHandle shader_prog_bgfx_hdl;
        bgfx::UniformHandle texture_sampler_uniform_bgfx_hdl;

        const s_gfx_resource &px_texture;

        s_rendering_basis(bgfx::DynamicVertexBufferHandle vb_hdl, bgfx::ProgramHandle prog_hdl, bgfx::UniformHandle tex_sampler_uniform_hdl, s_gfx_resource &px_tex)
            : vert_buf_bgfx_hdl(vb_hdl), shader_prog_bgfx_hdl(prog_hdl), texture_sampler_uniform_bgfx_hdl(tex_sampler_uniform_hdl), px_texture(px_tex) {}

        s_rendering_basis(const s_rendering_basis &) = delete;
    };

    struct s_rendering_context {
        const s_rendering_basis &basis;

        struct {
            s_static_array<s_batch_vert, g_batch_vert_limit_per_frame> verts;
            t_i32 vert_offs;
            t_i32 vert_cnt;

            s_ptr<const s_gfx_resource> texture;
        } batch_state = {};

        s_rendering_context(const s_rendering_basis &basis) : basis(basis) {}
        s_rendering_context(const s_rendering_context &) = delete;
    };

    static s_rendering_basis &CreateRenderingBasis(s_mem_arena &mem_arena, s_gfx_resource_arena &resource_arena);
    static void Flush(s_rendering_context &rc);
    static s_array<s_batch_vert> ReserveBatchVerts(s_rendering_context &rc, const t_i32 cnt, const s_ptr<const s_gfx_resource> texture);

    // ============================================================
    // @section: General
    // ============================================================
    s_rendering_basis &InitGFX(s_mem_arena &mem_arena) {
        ZF_ASSERT(g_state.state == ek_state_uninitted);

        g_state.state = ek_state_initted;

        bgfx::Init init = {};

        init.type = bgfx::RendererType::Count;

        init.resolution.reset = BGFX_RESET_VSYNC;

        const auto fb_size_cache = WindowFramebufferSizeCache();

        init.resolution.width = static_cast<uint32_t>(fb_size_cache.x);
        init.resolution.height = static_cast<uint32_t>(fb_size_cache.y);

        g_state.resolution_cache = fb_size_cache;

        init.platformData.nwh = internal::NativeWindowHandle();
        init.platformData.ndt = internal::NativeDisplayHandle();
        init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(init)) {
            ZF_FATAL();
        }

        g_state.perm_resource_arena = CreateGFXResourceArena(mem_arena);

        return CreateRenderingBasis(mem_arena, g_state.perm_resource_arena);
    }

    void ShutdownGFX(s_rendering_basis &rendering_state) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        bgfx::destroy(rendering_state.texture_sampler_uniform_bgfx_hdl);
        bgfx::destroy(rendering_state.shader_prog_bgfx_hdl);
        bgfx::destroy(rendering_state.vert_buf_bgfx_hdl);

        DestroyGFXResources(g_state.perm_resource_arena);

        bgfx::shutdown();

        g_state = {};
    }

    // ============================================================
    // @section: Resources
    // ============================================================
    static bgfx::ProgramHandle CreateBGFXShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin) {
        const bgfx::Memory *const vert_shader_bgfx_mem = bgfx::makeRef(vert_shader_bin.Ptr(), static_cast<uint32_t>(vert_shader_bin.Len()));
        const bgfx::ShaderHandle vert_shader_bgfx_hdl = bgfx::createShader(vert_shader_bgfx_mem);

        if (!bgfx::isValid(vert_shader_bgfx_hdl)) {
            return BGFX_INVALID_HANDLE;
        }

        const bgfx::Memory *const frag_shader_bgfx_mem = bgfx::makeRef(frag_shader_bin.Ptr(), static_cast<uint32_t>(frag_shader_bin.Len()));
        const bgfx::ShaderHandle frag_shader_bgfx_hdl = bgfx::createShader(frag_shader_bgfx_mem);

        if (!bgfx::isValid(frag_shader_bgfx_hdl)) {
            return BGFX_INVALID_HANDLE;
        }

        return bgfx::createProgram(vert_shader_bgfx_hdl, frag_shader_bgfx_hdl, true);
    }

    static s_gfx_resource &PushGFXResource(const e_gfx_resource_type type, s_gfx_resource_arena &arena) {
        ZF_ASSERT(type != ek_gfx_resource_type_invalid);

        s_gfx_resource &resource = Alloc<s_gfx_resource>(*arena.mem_arena);

        if (!arena.head) {
            arena.head = &resource;
            arena.tail = &resource;
        } else {
            arena.tail->next = &resource;
            arena.tail = &resource;
        }

        resource.type = type;

        return resource;
    }

    void DestroyGFXResources(s_gfx_resource_arena &arena) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        s_ptr<s_gfx_resource> resource = arena.head;

        while (resource) {
            switch (resource->type) {
            case ek_gfx_resource_type_invalid:
                break;

            case ek_gfx_resource_type_texture:
                bgfx::destroy(resource->Texture().bgfx_hdl);
                break;

            case ek_gfx_resource_type_font:
                break;
            }

            const auto next = resource->next;
            *resource = {};
            resource = next;
        }

        arena = {};
    }

    t_b8 CreateTextureResource(const s_texture_data_rdonly texture_data, s_ptr<s_gfx_resource> &o_resource, const s_ptr<s_gfx_resource_arena> arena) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.SizeInPixels().x), static_cast<uint16_t>(texture_data.SizeInPixels().y), false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(texture_data.RGBAPixelData().Ptr(), static_cast<uint32_t>(texture_data.RGBAPixelData().SizeInBytes())));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            return false;
        }

        o_resource = &PushGFXResource(ek_gfx_resource_type_texture, arena ? *arena : g_state.perm_resource_arena);
        o_resource->Texture().bgfx_hdl = texture_bgfx_hdl;
        o_resource->Texture().size = texture_data.SizeInPixels();

        return true;
    }

    s_v2_i TextureSize(const s_gfx_resource &texture) {
        ZF_ASSERT(g_state.state != ek_state_uninitted);
        return texture.Texture().size;
    }

    static t_b8 CreateFontResource(const s_font_arrangement &arrangement, const s_array<t_font_atlas_rgba> atlas_rgbas, s_gfx_resource_arena &arena, s_ptr<s_gfx_resource> &o_resource) {
        o_resource = &PushGFXResource(ek_gfx_resource_type_font, arena);
        o_resource->Font().arrangement = arrangement;

        o_resource->Font().atlases = AllocArray<s_ptr<s_gfx_resource>>(atlas_rgbas.Len(), *arena.mem_arena);

        for (t_i32 i = 0; i < atlas_rgbas.Len(); i++) {
            if (!CreateTextureResource({g_font_atlas_size, atlas_rgbas[i]}, o_resource->Font().atlases[i], &arena)) {
                return false;
            }
        }

        return true;
    }

    t_b8 CreateFontResourceFromRaw(const s_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec &code_pts, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_resource, const s_ptr<s_gfx_resource_arena> resource_arena) {
        auto &resource_arena_to_use = resource_arena ? *resource_arena : g_state.perm_resource_arena;

        s_font_arrangement arrangement;
        s_array<t_font_atlas_rgba> atlas_rgbas;

        if (!zf::LoadFontFromRaw(file_path, height, code_pts, *resource_arena_to_use.mem_arena, temp_mem_arena, temp_mem_arena, arrangement, atlas_rgbas)) {
            return false;
        }

        return CreateFontResource(arrangement, atlas_rgbas, resource_arena_to_use, o_resource);
    }

    t_b8 CreateFontResourceFromPacked(const s_str_rdonly file_path, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_resource, const s_ptr<s_gfx_resource_arena> resource_arena) {
        auto &resource_arena_to_use = resource_arena ? *resource_arena : g_state.perm_resource_arena;

        s_font_arrangement arrangement;
        s_array<t_font_atlas_rgba> atlas_rgbas;

        if (!zf::UnpackFont(file_path, *resource_arena_to_use.mem_arena, temp_mem_arena, temp_mem_arena, arrangement, atlas_rgbas)) {
            return false;
        }

        return CreateFontResource(arrangement, atlas_rgbas, resource_arena_to_use, o_resource);
    }

    // ============================================================
    // @section: Rendering
    // ============================================================
    static s_rendering_basis &CreateRenderingBasis(s_mem_arena &mem_arena, s_gfx_resource_arena &px_texture_resource_arena) {
        bgfx::VertexLayout vert_layout = {};
        vert_layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float).add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end();

        const auto vert_buf_bgfx_hdl = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(g_batch_vert_limit_per_frame), vert_layout);

        if (!bgfx::isValid(vert_buf_bgfx_hdl)) {
            ZF_FATAL();
        }

        const auto shader_prog_bgfx_hdl = CreateBGFXShaderProg({g_batch_vert_shader_src_raw, g_batch_vert_shader_src_len}, {g_batch_frag_shader_src_raw, g_batch_frag_shader_src_len});

        if (!bgfx::isValid(shader_prog_bgfx_hdl)) {
            ZF_FATAL();
        }

        const auto texture_sampler_uniform_bgfx_hdl = bgfx::createUniform("u_tex", bgfx::UniformType::Sampler);

        if (!bgfx::isValid(texture_sampler_uniform_bgfx_hdl)) {
            ZF_FATAL();
        }

        s_ptr<s_gfx_resource> px_texture;
        const s_static_array<t_u8, 4> px_texture_rgba = {255, 255, 255, 255};

        if (!CreateTextureResource({{1, 1}, px_texture_rgba}, px_texture, &px_texture_resource_arena)) {
            ZF_FATAL();
        }

        return Alloc<s_rendering_basis>(mem_arena, vert_buf_bgfx_hdl, shader_prog_bgfx_hdl, texture_sampler_uniform_bgfx_hdl, *px_texture);
    }

    s_rendering_context &internal::BeginFrame(const s_rendering_basis &rendering_basis, const s_color_rgb24f clear_col, s_mem_arena &mem_arena) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        const auto fb_size_cache = WindowFramebufferSizeCache();

        if (g_state.resolution_cache != fb_size_cache) {
            bgfx::reset(static_cast<uint32_t>(fb_size_cache.x), static_cast<uint32_t>(fb_size_cache.y), BGFX_RESET_VSYNC);
            g_state.resolution_cache = fb_size_cache;
        }

        bgfx::setViewMode(0, bgfx::ViewMode::Sequential);

        const auto view_mat = CreateIdentityMatrix();

        auto proj_mat = CreateIdentityMatrix();
        proj_mat.elems[0][0] = 1.0f / (static_cast<t_f32>(fb_size_cache.x) / 2.0f);
        proj_mat.elems[1][1] = -1.0f / (static_cast<t_f32>(fb_size_cache.y) / 2.0f);
        proj_mat.elems[3][0] = -1.0f;
        proj_mat.elems[3][1] = 1.0f;

        bgfx::setViewTransform(0, &view_mat, &proj_mat);

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR, ColorToHex(clear_col));

        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(fb_size_cache.x), static_cast<uint16_t>(fb_size_cache.y));

        bgfx::touch(0);

        g_state.state = ek_state_rendering;

        return Alloc<s_rendering_context>(mem_arena, rendering_basis);
    }

    void internal::EndFrame(s_rendering_context &rc) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        Flush(rc);

        bgfx::frame();

        g_state.state = ek_state_initted;
    }

    static void Flush(s_rendering_context &rc) {
        if (rc.batch_state.vert_cnt == 0) {
            return;
        }

        const auto verts = rc.batch_state.verts.ToNonstatic().Slice(rc.batch_state.vert_offs, rc.batch_state.vert_offs + rc.batch_state.vert_cnt);
        const auto verts_bgfx_ref = bgfx::makeRef(verts.Ptr(), static_cast<uint32_t>(verts.SizeInBytes()));
        bgfx::update(rc.basis.vert_buf_bgfx_hdl, static_cast<uint32_t>(rc.batch_state.vert_offs), verts_bgfx_ref);

        const auto texture_bgfx_hdl = rc.batch_state.texture ? rc.batch_state.texture->Texture().bgfx_hdl : rc.basis.px_texture.Texture().bgfx_hdl;
        bgfx::setTexture(0, rc.basis.texture_sampler_uniform_bgfx_hdl, texture_bgfx_hdl);

        bgfx::setVertexBuffer(0, rc.basis.vert_buf_bgfx_hdl, static_cast<uint32_t>(rc.batch_state.vert_offs), static_cast<uint32_t>(rc.batch_state.vert_cnt));

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, rc.basis.shader_prog_bgfx_hdl);

        rc.batch_state.vert_offs += rc.batch_state.vert_cnt;
        rc.batch_state.vert_cnt = 0;
    }

    static s_array<s_batch_vert> ReserveBatchVerts(s_rendering_context &rc, const t_i32 cnt, const s_ptr<const s_gfx_resource> texture) {
        ZF_ASSERT(cnt >= 0);

        if (texture != rc.batch_state.texture) {
            Flush(rc);
            rc.batch_state.texture = texture;
        }

        if (rc.batch_state.vert_offs + rc.batch_state.vert_cnt + cnt > rc.batch_state.verts.g_len) {
            ZF_FATAL();
        }

        rc.batch_state.vert_cnt += cnt;

        return rc.batch_state.verts.ToNonstatic().Slice(rc.batch_state.vert_offs + rc.batch_state.vert_cnt - cnt, rc.batch_state.vert_offs + rc.batch_state.vert_cnt);
    }

    // @todo: I think everything beyond this point could be pulled out of here and sandboxed off from the global state.

    void DrawTriangle(s_rendering_context &rc, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        const auto verts = ReserveBatchVerts(rc, 3, nullptr);
        verts[0] = {pts[0], pt_colors[0], {}};
        verts[1] = {pts[1], pt_colors[1], {}};
        verts[2] = {pts[2], pt_colors[2], {}};
    }

    void DrawRect(s_rendering_context &rc, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        const auto verts = ReserveBatchVerts(rc, 6, nullptr);

        verts[0] = {rect.TopLeft(), color_topleft, {0.0f, 0.0f}};
        verts[1] = {rect.TopRight(), color_topright, {1.0f, 0.0f}};
        verts[2] = {rect.BottomRight(), color_bottomright, {1.0f, 1.0f}};

        verts[3] = {rect.BottomRight(), color_bottomright, {1.0f, 1.0f}};
        verts[4] = {rect.BottomLeft(), color_bottomleft, {0.0f, 1.0f}};
        verts[5] = {rect.TopLeft(), color_topleft, {0.0f, 0.0f}};
    }

    void DrawTexture(s_rendering_context &rc, const s_gfx_resource &texture, const s_v2 pos, const s_rect_i src_rect) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        const auto verts = ReserveBatchVerts(rc, 6, &texture);

        const auto texture_size = texture.Texture().size;

        s_rect_i src_rect_to_use;

        if (src_rect == s_rect_i()) {
            src_rect_to_use = {{}, texture.Texture().size};
        } else {
            ZF_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && src_rect.Right() <= texture_size.x && src_rect.Bottom() <= texture_size.y);
            src_rect_to_use = src_rect;
        }

        const s_rect_f rect = {pos, src_rect_to_use.Size().ToV2()};
        const s_rect_f uv_rect = UVRect(src_rect_to_use, texture_size);

        verts[0] = {rect.TopLeft(), colors::g_white, uv_rect.TopLeft()};
        verts[1] = {rect.TopRight(), colors::g_white, uv_rect.TopRight()};
        verts[2] = {rect.BottomRight(), colors::g_white, uv_rect.BottomRight()};

        verts[3] = {rect.BottomRight(), colors::g_white, uv_rect.BottomRight()};
        verts[4] = {rect.BottomLeft(), colors::g_white, uv_rect.BottomLeft()};
        verts[5] = {rect.TopLeft(), colors::g_white, uv_rect.TopLeft()};
    }

    s_array<s_v2> LoadStrChrDrawPositions(const s_str_rdonly str, const s_font_arrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, s_mem_arena &mem_arena) {
        ZF_ASSERT(IsStrValidUTF8(str));
        ZF_ASSERT(IsAlignmentValid(alignment));

        // Calculate some useful string metadata.
        struct s_str_meta {
            t_i32 len = 0;
            t_i32 line_cnt = 0;
        };

        const auto str_meta = [str]() {
            s_str_meta meta = {.line_cnt = 1};

            ZF_WALK_STR(str, chr_info) {
                meta.len++;

                if (chr_info.code_pt == '\n') {
                    meta.line_cnt++;
                }
            }

            return meta;
        }();

        // Reserve memory for the character positions.
        const auto positions = AllocArray<s_v2>(str_meta.len, mem_arena);

        // From the line count we can determine the vertical alignment offset to apply.
        const t_f32 alignment_offs_y = static_cast<t_f32>(-(str_meta.line_cnt * font_arrangement.line_height)) * alignment.y;

        // Calculate the position of each character.
        t_i32 chr_index = 0;
        s_v2 chr_pos_pen = {}; // The position of the current character.
        t_i32 line_begin_chr_index = 0;
        t_i32 line_len = 0;
        t_code_pt code_pt_last;

        const auto apply_hor_alignment_offs = [&]() {
            if (line_len > 0) {
                const auto line_width = chr_pos_pen.x;

                for (t_i32 i = line_begin_chr_index; i < chr_index; i++) {
                    positions[i].x -= line_width * alignment.x;
                }
            }
        };

        ZF_WALK_STR(str, chr_info) {
            ZF_DEFER({
                chr_index++;
                code_pt_last = chr_info.code_pt;
            });

            if (line_len == 0) {
                line_begin_chr_index = chr_index;
            }

            if (chr_info.code_pt == '\n') {
                apply_hor_alignment_offs();

                chr_pos_pen.x = 0.0f;
                chr_pos_pen.y += static_cast<t_f32>(font_arrangement.line_height);

                line_len = 0;

                continue;
            }

            s_ptr<s_font_glyph_info> glyph_info;

            if (!font_arrangement.code_pts_to_glyph_infos.Find(chr_info.code_pt, &glyph_info)) {
                ZF_ASSERT(false && "Unsupported code point!");
                continue;
            }

            if (chr_index > 0 && font_arrangement.has_kernings) {
                s_ptr<t_i32> kerning;

                if (font_arrangement.code_pt_pairs_to_kernings.Find({code_pt_last, chr_info.code_pt}, &kerning)) {
                    chr_pos_pen.x += static_cast<t_f32>(*kerning);
                }
            }

            positions[chr_index] = pos + chr_pos_pen + glyph_info->offs.ToV2();
            positions[chr_index].y += alignment_offs_y;

            chr_pos_pen.x += static_cast<t_f32>(glyph_info->adv);

            line_len++;
        }

        apply_hor_alignment_offs();

        return positions;
    }

    void DrawStr(s_rendering_context &rc, const s_str_rdonly str, const s_gfx_resource &font, const s_v2 pos, const s_v2 alignment, const s_color_rgba32f blend, s_mem_arena &temp_mem_arena) {
        ZF_ASSERT(IsStrValidUTF8(str));
        ZF_ASSERT(IsAlignmentValid(alignment));

        if (str.IsEmpty()) {
            return;
        }

        const auto &font_arrangement = font.Font().arrangement;
        const auto &font_atlases = font.Font().atlases;

        const s_array<s_v2> chr_positions = LoadStrChrDrawPositions(str, font_arrangement, pos, alignment, temp_mem_arena);

        t_i32 chr_index = 0;

        ZF_WALK_STR(str, chr_info) {
            if (chr_info.code_pt == ' ' || chr_info.code_pt == '\n') {
                chr_index++;
                continue;
            }

            s_ptr<s_font_glyph_info> glyph_info;

            if (!font_arrangement.code_pts_to_glyph_infos.Find(chr_info.code_pt, &glyph_info)) {
                ZF_ASSERT(false && "Unsupported code point!");
                continue;
            }

            DrawTexture(rc, *font_atlases[glyph_info->atlas_index], chr_positions[chr_index], glyph_info->atlas_rect);

            chr_index++;
        };
    }
}
