#ifndef ZFW_AUDIO_H
#define ZFW_AUDIO_H

#define SND_LIMIT 32

#define VOL_DEFAULT 1.0f
#define PAN_DEFAULT 0.0f
#define PITCH_DEFAULT 1.0f

#include <miniaudio.h>
#include "zfw_utils.h"

typedef struct {
    ma_engine eng;
    ma_decoder* decoders;
    int audio_cnt;
    ma_sound snds[SND_LIMIT];
    t_byte snd_activity[BITS_TO_BYTES(SND_LIMIT)];
} s_audio;

typedef const char* (*t_audio_index_to_file_path)(const int index);

bool InitAudio(s_audio* const audio, s_mem_arena* const mem_arena, const int audio_cnt, const t_audio_index_to_file_path audio_index_to_fp);
void CleanAudio(s_audio* const audio);
void UpdateAudio(s_audio* const audio);
bool PlayAudio(s_audio* const audio, const int index, const float vol, const float pan, const float pitch);

#endif
