#include <stdio.h>
#include "zfw_audio.h"

static bool LoadSoundTypeFromFile(s_sound_type* const type, const char* const fp, s_mem_arena* const mem_arena) {
    assert(type && IS_ZERO(*type));
    assert(fp);

    ma_decoder decoder;

    if (ma_decoder_init_file(fp, NULL, &decoder) != MA_SUCCESS) {
        fprintf(stderr, "Failed to open audio file \"%s\".\n", fp);
        return false;
    }

    type->channel_cnt = decoder.outputChannels;
    type->sample_rate = decoder.outputSampleRate;
    type->format = decoder.outputFormat;

    const int frame_size = ma_get_bytes_per_frame(type->format, type->channel_cnt);

    ma_uint64 frame_cnt;
    ma_decoder_get_length_in_pcm_frames(&decoder, &frame_cnt);

    type->frame_cnt = frame_cnt;

    const int sample_buf_size = frame_size * frame_cnt;

    type->sample_buf = MEM_ARENA_PUSH_TYPE_MANY(mem_arena, t_byte, sample_buf_size);

    if (!type->sample_buf) {
        ma_decoder_uninit(&decoder);
        return false;
    }

    ma_uint64 frames_read;
    const ma_result res = ma_decoder_read_pcm_frames(&decoder, type->sample_buf, frame_cnt, &frames_read);

    ma_decoder_uninit(&decoder);

    if (frames_read < frame_cnt) {
        fprintf(stderr, "Only read %llu of %llu frames for audio file \"%s\"!\n", frames_read, frame_cnt, fp);
        return false;
    }

    return true;
}

bool InitAudioSys(s_audio_sys* const audio_sys) {
    assert(audio_sys && IS_ZERO(*audio_sys));

    if (ma_engine_init(NULL, &audio_sys->eng) != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialise miniaudio engine!\n");
        return false;
    }

    return true;
}

void CleanAudioSys(s_audio_sys* const audio_sys) {
    for (int i = 0; i < SND_LIMIT; i++) {
        if (IsBitActive(i, audio_sys->snd_activity, SND_LIMIT)) {
            ma_sound_uninit(&audio_sys->snds[i]);
            ma_audio_buffer_uninit(&audio_sys->audio_bufs[i]);
        }
    }

    ma_engine_uninit(&audio_sys->eng);

    ZERO_OUT(*audio_sys);
}

void UpdateAudioSys(s_audio_sys* const audio_sys) {
    for (int i = 0; i < SND_LIMIT; i++) {
        if (!IsBitActive(i, audio_sys->snd_activity, SND_LIMIT)) {
            continue;
        }

        ma_sound* const snd = &audio_sys->snds[i];

        if (!ma_sound_is_playing(snd)) {
            ma_sound_uninit(snd);
            ma_audio_buffer_uninit(&audio_sys->audio_bufs[i]);
            DeactivateBit(i, audio_sys->snd_activity, SND_LIMIT);
        }
    }
}

bool LoadSoundTypesFromFiles(s_sound_types* const types, s_mem_arena* const mem_arena, const int cnt, const t_sound_type_index_to_file_path index_to_fp) {
    assert(types && IS_ZERO(*types));
    assert(cnt > 0);
    assert(index_to_fp);

    *types = (s_sound_types){
        .buf = MEM_ARENA_PUSH_TYPE_MANY(mem_arena, s_sound_type, cnt),
        .cnt = cnt
    };

    if (!types->buf) {
        return false;
    }

    for (int i = 0; i < cnt; i++) {
        const char* const fp = index_to_fp(i);

        if (!LoadSoundTypeFromFile(&types->buf[i], fp, mem_arena)) {
            return false;
        }
    }

    return true;
}

bool PlaySound(s_audio_sys* const audio_sys, const s_sound_types* const snd_types, const int type_index, const float vol, const float pan, const float pitch) {
    assert(audio_sys);
    assert(type_index >= 0 && type_index < snd_types->cnt);
    assert(vol >= 0.0f && vol <= 1.0f);
    assert(pan >= -1.0f && pan <= 1.0f);
    assert(pitch > 0.0f);

    const int index = FirstInactiveBitIndex(audio_sys->snd_activity, SND_LIMIT);

    if (index == -1) {
        fprintf(stderr, "Failed to play sound due to insufficient space!\n");
        return false;
    }

    ActivateBit(index, audio_sys->snd_activity, SND_LIMIT);

    const s_sound_type* const type = &snd_types->buf[type_index];
    ma_sound* const snd = &audio_sys->snds[index];

    const ma_audio_buffer_config buf_config = ma_audio_buffer_config_init(type->format, type->channel_cnt, type->frame_cnt, type->sample_buf, NULL);

    ma_audio_buffer_init_copy(&buf_config, &audio_sys->audio_bufs[index]);

    ma_sound_init_from_data_source(&audio_sys->eng, &audio_sys->audio_bufs[index], 0, NULL, snd);

    ma_sound_set_volume(snd, vol);
    ma_sound_set_pan(snd, pan);
    ma_sound_set_pitch(snd, pitch);
    ma_sound_start(snd);

    return true;
}
