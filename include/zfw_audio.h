#ifndef ZFW_AUDIO_H
#define ZFW_AUDIO_H

#include <miniaudio.h>
#include <stdio.h>
#include "zfw_utils.h"

typedef struct {
    ma_engine eng;
    ma_decoder* decoders;
    int audio_cnt;
} s_audio;

typedef const char* (*t_audio_index_to_file_path)(const int index);

bool InitAudio(s_audio* const audio, s_mem_arena* const mem_arena, const int audio_cnt, const t_audio_index_to_file_path audio_index_to_fp) {
    assert(audio && IS_ZERO(*audio));
    assert(audio_cnt > 0);
    assert(audio_index_to_fp);

    audio->decoders = MEM_ARENA_PUSH_TYPE_MANY(mem_arena, ma_decoder, audio_cnt);

    if (!audio->decoders) {
        return false;
    }

    while (audio->audio_cnt < audio_cnt) {
        const int i = audio->audio_cnt;
        const char* const fp = audio_index_to_fp(i);
        const ma_result result = ma_decoder_init_file(fp, NULL, &audio->decoders[i]);

        if (result != MA_SUCCESS) {
            fprintf(stderr, "Failed to load sound \"%s\".\n", fp);
            return false;
        }

        audio->audio_cnt++;
    }

    return true;
}

void CleanAudio(s_audio* const audio) {

}

#endif
