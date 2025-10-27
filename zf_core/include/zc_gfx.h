#pragma once

#include "zc_mem.h"
#include "zc_math.h"
#include "zc_io.h"

namespace zf {
    namespace colors {
        constexpr s_v4 g_black = {0.0f, 0.0f, 0.0f, 1.0f};
        constexpr s_v4 g_dark_gray = {0.25f, 0.25f, 0.25f, 1.0f};
        constexpr s_v4 g_gray = {0.5f, 0.5f, 0.5f, 1.0f};
        constexpr s_v4 g_light_gray = {0.75f, 0.75f, 0.75f, 1.0f};
        constexpr s_v4 g_white = {1.0f, 1.0f, 1.0f, 1.0f};
        constexpr s_v4 g_red = {1.0f, 0.0f, 0.0f, 1.0f};
        constexpr s_v4 g_orange = {1.0f, 0.5f, 0.0f, 1.0f};
        constexpr s_v4 g_yellow = {1.0f, 1.0f, 0.0f, 1.0f};
        constexpr s_v4 g_lime = {0.75f, 1.0f, 0.0f, 1.0f};
        constexpr s_v4 g_green = {0.0f, 1.0f, 0.0f, 1.0f};
        constexpr s_v4 g_teal = {0.0f, 0.5f, 0.5f, 1.0f};
        constexpr s_v4 g_cyan = {0.0f, 1.0f, 1.0f, 1.0f};
        constexpr s_v4 g_blue = {0.0f, 0.0f, 1.0f, 1.0f};
        constexpr s_v4 g_purple = {0.5f, 0.0f, 0.5f, 1.0f};
        constexpr s_v4 g_magenta = {1.0f, 0.0f, 1.0f, 1.0f};
        constexpr s_v4 g_pink = {1.0f, 0.75f, 0.8f, 1.0f};
        constexpr s_v4 g_brown = {0.6f, 0.3f, 0.0f, 1.0f};
    }

    namespace origins {
        constexpr s_v2 g_origin_top_left = {0.0f, 0.0f};
        constexpr s_v2 g_origin_top_center = {0.5f, 0.0f};
        constexpr s_v2 g_origin_top_right = {1.0f, 0.0f};
        constexpr s_v2 g_origin_center_left = {0.0f, 0.5f};
        constexpr s_v2 g_origin_center = {0.5f, 0.5f};
        constexpr s_v2 g_origin_center_right = {1.0f, 0.5f};
        constexpr s_v2 g_origin_bottom_left = {0.0f, 1.0f};
        constexpr s_v2 g_origin_bottom_center = {0.5f, 1.0f};
        constexpr s_v2 g_origin_bottom_right = {1.0f, 1.0f};
    }

    namespace alignments {
        constexpr s_v2 g_alignment_top_left = {0.0f, 0.0f};
        constexpr s_v2 g_alignment_top_center = {0.5f, 0.0f};
        constexpr s_v2 g_alignment_top_right = {1.0f, 0.0f};
        constexpr s_v2 g_alignment_center_left = {0.0f, 0.5f};
        constexpr s_v2 g_alignment_center = {0.5f, 0.5f};
        constexpr s_v2 g_alignment_center_right = {1.0f, 0.5f};
        constexpr s_v2 g_alignment_bottom_left = {0.0f, 1.0f};
        constexpr s_v2 g_alignment_bottom_center = {0.5f, 1.0f};
        constexpr s_v2 g_alignment_bottom_right = {1.0f, 1.0f};
    }

    struct s_int_rgba {
        t_u8 r = 0;
        t_u8 g = 0;
        t_u8 b = 0;
        t_u8 a = 0;

        t_u32 RGBA() const {
            return *reinterpret_cast<const t_u32*>(this);
        }
    };

    static inline s_int_rgba ToIntRGBA(const s_v4 flt) {
        return {
            static_cast<t_u8>(roundf(flt.x * 255.0f)),
            static_cast<t_u8>(roundf(flt.y * 255.0f)),
            static_cast<t_u8>(roundf(flt.z * 255.0f)),
            static_cast<t_u8>(roundf(flt.w * 255.0f))
        };
    }

    struct s_rgba_texture {
        s_v2_s32 dims;
        c_array<t_u8> px_data;
    };

    struct s_font_arrangement {
        t_s32 line_height = 0;

        s_static_array<s_v2_s32, g_ascii_printable_range_len> chr_offsets;
        s_static_array<s_v2_s32, g_ascii_printable_range_len> chr_sizes;
        s_static_array<t_s32, g_ascii_printable_range_len> chr_advances;
    };

    struct s_font_texture_meta {
        s_v2_s32 size;
        s_static_array<t_s32, g_ascii_printable_range_len> chr_xs;
    };

    [[nodiscard]] bool LoadRGBATextureFromRawFile(s_rgba_texture& tex, c_mem_arena& mem_arena, const c_string_view file_path);
    bool LoadFontFromRawFile(s_font_arrangement& arrangement, s_font_texture_meta& tex_meta, c_array<const t_u8>& tex_rgba_px_data, const c_string_view file_path, const t_s32 height, c_mem_arena& temp_mem_arena);

    bool PackTexture(c_file_writer& fw, const s_rgba_texture rgba_tex);
    void UnpackTexture(c_file_reader& fr, s_rgba_texture& rgba_tex);

    bool PackFont(c_file_writer& fw, const s_font_arrangement& arrangement, const s_font_texture_meta tex_meta, const c_array<const t_u8> tex_rgba_px_data);
    void UnpackFont(c_file_reader& fr, s_font_arrangement& arrangement, s_font_texture_meta tex_meta, c_array<const t_u8>& tex_rgba_px_data);

    struct s_font {
        s_font_arrangement arrangement;
        s_font_texture_meta tex_meta;
    };

    [[nodiscard]] bool GenStrChrRenderPositions(c_array<s_v2>& positions, c_mem_arena& mem_arena, const c_string_view str, const s_font_group& font_group, t_s32 font_index, s_v2 pos, s_v2 alignment);
    [[nodiscard]] bool GenStrCollider(s_rect& rect, const c_string_view str, const s_font_group& font_group, t_s32 font_index, s_v2 pos, s_v2 alignment, c_mem_arena& temp_mem_arena);
    [[nodiscard]] bool RenderStr(const s_rendering_context& rendering_context, const c_string_view str, const s_font_group& fonts, t_s32 font_index, s_v2 pos, s_v2 alignment, s_v4 color, c_mem_arena& temp_mem_arena);
}
