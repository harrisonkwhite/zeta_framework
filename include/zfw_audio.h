#ifndef ZFW_AUDIO_H
#define ZFW_AUDIO_H

#include <miniaudio.h>
#include "zfw_utils.h"

#define ZFW_SND_LIMIT 32

#define ZFW_VOL_DEFAULT 1.0f
#define ZFW_PAN_DEFAULT 0.0f
#define ZFW_PITCH_DEFAULT 1.0f

typedef struct {
    zfw_t_byte* sample_buf;
    int frame_cnt;
    int channel_cnt;
    int sample_rate;
    ma_format format;
} zfw_s_sound_type;

typedef struct {
    zfw_s_sound_type* buf;
    int cnt;
} zfw_s_sound_types;

typedef struct {
    ma_engine eng;

    ma_sound snds[ZFW_SND_LIMIT];
    ma_audio_buffer audio_bufs[ZFW_SND_LIMIT];
    zfw_t_byte snd_activity[ZFW_BITS_TO_BYTES(ZFW_SND_LIMIT)];
} zfw_s_audio_sys;

typedef const char* (*zfw_t_sound_type_index_to_file_path)(const int index);

bool ZFWInitAudioSys(zfw_s_audio_sys* const audio_sys);
void ZFWCleanAudioSys(zfw_s_audio_sys* const audio_sys);
void ZFWUpdateAudioSys(zfw_s_audio_sys* const audio_sys);

bool ZFWLoadSoundTypesFromFiles(zfw_s_sound_types* const types, zfw_s_mem_arena* const mem_arena, const int cnt, const zfw_t_sound_type_index_to_file_path index_to_fp);

bool ZFWPlaySound(zfw_s_audio_sys* const audio_sys, const zfw_s_sound_types* const snd_types, const int type_index, const float vol, const float pan, const float pitch);

#endif
