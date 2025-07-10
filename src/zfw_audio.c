#include <stdio.h>
#include "zfw_audio.h"
#include "zfw_utils.h"

bool InitAudio(s_audio* const audio, s_mem_arena* const mem_arena, const int audio_cnt, const t_audio_index_to_file_path audio_index_to_fp) {
    assert(audio && IS_ZERO(*audio));
    assert(audio_cnt > 0);
    assert(audio_index_to_fp);

    if (ma_engine_init(NULL, &audio->eng) != MA_SUCCESS) {
        return false;
    }

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
    for (int i = 0; i < audio->audio_cnt; i++) {
        ma_decoder_uninit(&audio->decoders[i]);
    }

    ma_engine_uninit(&audio->eng);

    ZERO_OUT(*audio);
}

void UpdateAudio(s_audio* const audio) {
    for (int i = 0; i < SND_LIMIT; i++) {
        if (!IsBitActive(i, audio->snd_activity, SND_LIMIT)) {
            continue;
        }

        ma_sound* const snd = &audio->snds[i];

        if (!ma_sound_is_playing(snd)) {
            ma_sound_uninit(snd);
            DeactivateBit(i, audio->snd_activity, SND_LIMIT);
        }
    }
}

bool PlayAudio(s_audio* const audio, const int index, const float vol, const float pan, const float pitch) {
    assert(audio);
    assert(index >= 0 && index < audio->audio_cnt);
    assert(vol >= 0.0f && vol <= 1.0f);
    assert(pan >= -1.0f && pan <= 1.0f);
    assert(pitch > 0.0f);

    const int snd_index = FirstInactiveBitIndex(audio->snd_activity, SND_LIMIT);

    if (snd_index == -1) {
        fprintf(stderr, "Failed to play audio due to insufficient space!\n");
        return false;
    }

    ActivateBit(snd_index, audio->snd_activity, SND_LIMIT);

    ma_sound* const snd = &audio->snds[snd_index];

    const ma_result result = ma_sound_init_from_data_source(&audio->eng, &audio->decoders[index], MA_SOUND_FLAG_ASYNC | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, snd);

    if (result != MA_SUCCESS) {
        return false;
    }

    ma_sound_set_volume(snd, vol);
    ma_sound_set_pan(snd, pan);
    ma_sound_set_pitch(snd, pitch);
    ma_sound_start(snd);

    return true;
}
