#include <stdio.h>
#include "zfw_audio.h"

static bool LoadSoundTypeFromFile(zfw_s_sound_type* const type, const char* const fp, zfw_s_mem_arena* const mem_arena) {
    assert(ZFW_IS_ZERO(*type));

    // Decode the audio file, store properties (e.g. sample rate, channel count).
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

    // Allocate memory for the audio sample buffer.
    const int sample_buf_size = frame_size * frame_cnt;

    type->sample_buf = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_t_byte, sample_buf_size);

    if (!type->sample_buf) {
        ma_decoder_uninit(&decoder);
        return false;
    }

    // Populate the buffer, also get the number of frames read for error checking.
    ma_uint64 frames_read;
    const ma_result res = ma_decoder_read_pcm_frames(&decoder, type->sample_buf, frame_cnt, &frames_read);

    ma_decoder_uninit(&decoder);

    if (frames_read < frame_cnt) {
        fprintf(stderr, "Only read %llu of %llu frames for audio file \"%s\"!\n", frames_read, frame_cnt, fp);
        return false;
    }

    return true;
}

bool ZFWInitAudioSys(zfw_s_audio_sys* const audio_sys) {
    assert(ZFW_IS_ZERO(*audio_sys));

    if (ma_engine_init(NULL, &audio_sys->eng) != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialise miniaudio engine!\n");
        return false;
    }

    return true;
}

void ZFWCleanAudioSys(zfw_s_audio_sys* const audio_sys) {
    for (int i = 0; i < ZFW_SND_LIMIT; i++) {
        if (ZFWIsBitActive(i, audio_sys->snd_activity, ZFW_SND_LIMIT)) {
            ma_sound_uninit(&audio_sys->snds[i]);
            ma_audio_buffer_uninit(&audio_sys->audio_bufs[i]);
        }
    }

    ma_engine_uninit(&audio_sys->eng);

    ZFW_ZERO_OUT(*audio_sys);
}

void ZFWUpdateAudioSys(zfw_s_audio_sys* const audio_sys) {
    // Find any active sound slots where the sound has finished playing and deactivate them.
    for (int i = 0; i < ZFW_SND_LIMIT; i++) {
        if (!ZFWIsBitActive(i, audio_sys->snd_activity, ZFW_SND_LIMIT)) {
            continue;
        }

        ma_sound* const snd = &audio_sys->snds[i];

        if (!ma_sound_is_playing(snd)) {
            ma_sound_uninit(snd);
            ma_audio_buffer_uninit(&audio_sys->audio_bufs[i]);
            ZFWDeactivateBit(i, audio_sys->snd_activity, ZFW_SND_LIMIT);
        }
    }
}

bool ZFWLoadSoundTypesFromFiles(zfw_s_sound_types* const types, zfw_s_mem_arena* const mem_arena, const int cnt, const zfw_t_sound_type_index_to_file_path index_to_fp) {
    assert(ZFW_IS_ZERO(*types));
    assert(cnt > 0);

    *types = (zfw_s_sound_types){
        .buf = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_s_sound_type, cnt),
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

bool ZFWPlaySound(zfw_s_audio_sys* const audio_sys, const zfw_s_sound_types* const snd_types, const int type_index, const float vol, const float pan, const float pitch) {
    assert(type_index >= 0 && type_index < snd_types->cnt);
    assert(vol >= 0.0f && vol <= 1.0f);
    assert(pan >= -1.0f && pan <= 1.0f);
    assert(pitch > 0.0f);

    // Get the index of the first inactive sound slot.
    const int index = ZFWFirstInactiveBitIndex(audio_sys->snd_activity, ZFW_SND_LIMIT);

    if (index == -1) {
        fprintf(stderr, "Failed to play sound due to insufficient space!\n");
        return false;
    }

    // Activate the sound slot.
    ZFWActivateBit(index, audio_sys->snd_activity, ZFW_SND_LIMIT);

    const zfw_s_sound_type* const type = &snd_types->buf[type_index];
    ma_sound* const snd = &audio_sys->snds[index];

    // Set up a new audio buffer using the sound type sample buffer.
    const ma_audio_buffer_config buf_config = ma_audio_buffer_config_init(type->format, type->channel_cnt, type->frame_cnt, type->sample_buf, NULL);

    ma_audio_buffer_init_copy(&buf_config, &audio_sys->audio_bufs[index]);

    // Bind the buffer to the sound.
    ma_sound_init_from_data_source(&audio_sys->eng, &audio_sys->audio_bufs[index], 0, NULL, snd);

    // Set properties and play.
    ma_sound_set_volume(snd, vol);
    ma_sound_set_pan(snd, pan);
    ma_sound_set_pitch(snd, pitch);

    ma_sound_start(snd);

    return true;
}
