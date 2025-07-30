#include "zfw_audio.h"
#include "mem.h"

#include <stdio.h>

static zfw_s_sound_type LoadSoundTypeFromFile(const char* const fp, s_mem_arena* const mem_arena) {
    assert(fp);
    assert(mem_arena && IsMemArenaValid(mem_arena));

    // Decode the audio file, store properties (e.g. sample rate, channel count).
    ma_decoder decoder;

    if (ma_decoder_init_file(fp, NULL, &decoder) != MA_SUCCESS) {
        LOG_ERROR("Failed to open audio file \"%s\".", fp);
        return (zfw_s_sound_type){0};
    }

    const int sample_rate = decoder.outputSampleRate;
    const ma_format format = decoder.outputFormat;

    const int frame_size = ma_get_bytes_per_frame(format, decoder.outputChannels);

    ma_uint64 frame_cnt;
    ma_decoder_get_length_in_pcm_frames(&decoder, &frame_cnt);

    // Allocate memory for the audio sample buffer.
    const int sample_buf_size = frame_size * frame_cnt;
    t_u8* const sample_buf = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, t_u8, sample_buf_size);

    if (!sample_buf) {
        LOG_ERROR("Failed to reserve memory for audio sample buffer!");
        ma_decoder_uninit(&decoder);
        return (zfw_s_sound_type){0};
    }

    // Populate the buffer, also get the number of frames read for error checking.
    ma_uint64 frames_read;
    const ma_result res = ma_decoder_read_pcm_frames(&decoder, sample_buf, frame_cnt, &frames_read);

    ma_decoder_uninit(&decoder);

    if (frames_read < frame_cnt) {
        LOG_ERROR("Only read %llu of %llu frames for audio file \"%s\"!", frames_read, frame_cnt, fp);
        ma_decoder_uninit(&decoder);
        return (zfw_s_sound_type){0};
    }

    const zfw_s_sound_type snd_type = {
        .sample_buf = sample_buf,
        .frame_cnt = frame_cnt,
        .channel_cnt = decoder.outputChannels,
        .sample_rate = decoder.outputSampleRate,
        .format = decoder.outputFormat
    };

    ma_decoder_uninit(&decoder);

    return snd_type;
}

static zfw_s_sound_types LoadSoundTypesFromFiles(s_mem_arena* const mem_arena, const int cnt, const char* const* const snd_type_fps) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(cnt > 0);
    assert(snd_type_fps);

    zfw_s_sound_type* const buf = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_sound_type, cnt);

    if (!buf) {
        LOG_ERROR("Failed to reserve memory for sound types!");
        return (zfw_s_sound_types){0};
    }

    for (int i = 0; i < cnt; i++) {
        buf[i] = LoadSoundTypeFromFile(snd_type_fps[i], mem_arena);

        if (IS_ZERO(buf[i])) {
            LOG_ERROR("Failed to load sound type \"%s\"!", snd_type_fps[i]);
            return (zfw_s_sound_types){0};
        }
    }

    return (zfw_s_sound_types){
        .buf = buf,
        .cnt = cnt
    };
}

bool ZFW_InitAudioSys(zfw_s_audio_sys* const audio_sys, s_mem_arena* const mem_arena, const int snd_type_cnt, const char* const* const snd_type_fps) {
    assert(audio_sys && IS_ZERO(*audio_sys));

    if (ma_engine_init(NULL, &audio_sys->eng) != MA_SUCCESS) {
        LOG_ERROR("Failed to initialise miniaudio engine!");
        return false;
    }

    if (audio_sys->snd_types.cnt > 0) {
        audio_sys->snd_types = LoadSoundTypesFromFiles(mem_arena, snd_type_cnt, snd_type_fps);

        if (IS_ZERO(audio_sys->snd_types)) {
            LOG_ERROR("Failed to load sound types!");
            return false;
        }
    }

    return true;
}

void ZFW_CleanAudioSys(zfw_s_audio_sys* const audio_sys) {
    for (int i = 0; i < ZFW_SND_LIMIT; i++) {
        if (IsBitActive(i, audio_sys->snd_activity, ZFW_SND_LIMIT)) {
            ma_sound_uninit(&audio_sys->snds[i]);
            ma_audio_buffer_uninit(&audio_sys->audio_bufs[i]);
        }
    }

    ma_engine_uninit(&audio_sys->eng);

    ZERO_OUT(*audio_sys);
}

void ZFW_UpdateAudioSys(zfw_s_audio_sys* const audio_sys) {
    // Find any active sound slots where the sound has finished playing and deactivate them.
    for (int i = 0; i < ZFW_SND_LIMIT; i++) {
        if (!IsBitActive(i, audio_sys->snd_activity, ZFW_SND_LIMIT)) {
            continue;
        }

        ma_sound* const snd = &audio_sys->snds[i];

        if (!ma_sound_is_playing(snd)) {
            ma_sound_uninit(snd);
            ma_audio_buffer_uninit(&audio_sys->audio_bufs[i]);
            DeactivateBit(i, audio_sys->snd_activity, ZFW_SND_LIMIT);
        }
    }
}

bool ZFW_PlaySound(zfw_s_audio_sys* const audio_sys, const int snd_type_index, const float vol, const float pan, const float pitch) {
    assert(snd_type_index >= 0 && snd_type_index < audio_sys->snd_types.cnt);
    assert(vol >= 0.0f && vol <= 1.0f);
    assert(pan >= -1.0f && pan <= 1.0f);
    assert(pitch > 0.0f);

    // Get the index of the first inactive sound slot.
    const int index = FirstInactiveBitIndex(audio_sys->snd_activity, ZFW_SND_LIMIT);

    if (index == -1) {
        LOG_ERROR("Failed to play sound due to insufficient space!");
        return false;
    }

    // Activate the sound slot.
    ActivateBit(index, audio_sys->snd_activity, ZFW_SND_LIMIT);

    const zfw_s_sound_type* const snd_type = &audio_sys->snd_types.buf[snd_type_index];
    ma_sound* const snd = &audio_sys->snds[index];

    // Set up a new audio buffer using the sound type sample buffer.
    const ma_audio_buffer_config buf_config = ma_audio_buffer_config_init(snd_type->format, snd_type->channel_cnt, snd_type->frame_cnt, snd_type->sample_buf, NULL);

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
