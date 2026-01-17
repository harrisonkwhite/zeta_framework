#pragma once

#include <zcl.h>
#include <zgl/zgl_audio_public.h>

namespace zgl {
    struct t_sound_type {
        zcl::t_b8 valid;

        zcl::t_b8 streamable;
        zcl::t_sound_data_rdonly nonstream_snd_data;
        zcl::t_str_rdonly stream_external_file_path_terminated; // Terminated since it often needs to be converted to C-string.

        t_sound_type *next;
    };
}
