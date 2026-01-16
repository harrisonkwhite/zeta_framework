#pragma once

#include <zcl.h>
#include <zgl/zgl_audio.h>
#include <miniaudio.h>

namespace zgl {
    constexpr zcl::t_i32 k_sound_limit = 32;

    struct t_audio_sys {
        zcl::t_u64 magic;

        zcl::t_b8 paused;

        ma_engine ma_eng;

        struct {
            zcl::t_static_array<ma_sound, k_sound_limit> ma_snds;
            zcl::t_static_array<ma_audio_buffer_ref, k_sound_limit> ma_buf_refs;
            zcl::t_static_array<const t_sound_type *, k_sound_limit> types;
            zcl::t_static_array<t_sound_state, k_sound_limit> states;
            zcl::t_static_bitset<k_sound_limit> active;
            zcl::t_static_array<zcl::t_i32, k_sound_limit> versions;
        } snd_insts;
    };

    namespace detail {
#ifdef ZCL_DEBUG
        zcl::t_b8 AudioSysCheckValid(const t_audio_sys *const sys);
#endif
    };
}
