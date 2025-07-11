#ifndef ZFW_AUDIO_H
#define ZFW_AUDIO_H

#define SND_LIMIT 32

#define VOL_DEFAULT 1.0f
#define PAN_DEFAULT 0.0f
#define PITCH_DEFAULT 1.0f

#include <miniaudio.h>
#include "zfw_utils.h"

typedef struct {
    t_byte* sample_buf;
    int frame_cnt;
    int channel_cnt;
    int sample_rate;
    ma_format format;
} s_sound_type;

typedef struct {
    s_sound_type* buf;
    int cnt;
} s_sound_types;

typedef struct {
    ma_engine eng;

    ma_sound snds[SND_LIMIT];
    ma_audio_buffer audio_bufs[SND_LIMIT];
    t_byte snd_activity[BITS_TO_BYTES(SND_LIMIT)];
} s_audio_sys;

typedef const char* (*t_sound_type_index_to_file_path)(const int index);

bool InitAudioSys(s_audio_sys* const audio_sys);
void CleanAudioSys(s_audio_sys* const audio_sys);
void UpdateAudioSys(s_audio_sys* const audio_sys);

bool LoadSoundTypesFromFiles(s_sound_types* const types, s_mem_arena* const mem_arena, const int cnt, const t_sound_type_index_to_file_path index_to_fp);

bool PlaySound(s_audio_sys* const audio_sys, const s_sound_types* const snd_types, const int type_index, const float vol, const float pan, const float pitch);

#endif
