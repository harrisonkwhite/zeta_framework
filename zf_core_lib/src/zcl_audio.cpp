#include <zcl/zcl_audio.h>

#include <miniaudio.h>

namespace zf {
    t_b8 LoadSoundDataFromRaw(const s_str_rdonly file_path, s_arena *const snd_data_mem_arena, s_arena *const temp_mem_arena, s_sound_data_mut *const o_snd_data) {
        const s_str_rdonly file_path_terminated = AllocStrCloneButAddTerminator(file_path, temp_mem_arena);

        ZF_DEFINE_UNINITTED(ma_decoder, decoder);
        ma_decoder_config decoder_config = ma_decoder_config_init(ma_format_f32, 0, 0);

        if (ma_decoder_init_file(file_path_terminated.Cstr(), &decoder_config, &decoder) != MA_SUCCESS) {
            return false;
        }

        ZF_DEFER({ ma_decoder_uninit(&decoder); });

        ZF_DEFINE_UNINITTED(ma_uint64, frame_cnt);

        if (ma_decoder_get_length_in_pcm_frames(&decoder, &frame_cnt) != MA_SUCCESS) {
            return false;
        }

        const s_sound_meta meta = {
            .sample_rate = static_cast<t_i32>(decoder.outputSampleRate),
            .channel_cnt = static_cast<t_i32>(decoder.outputChannels),
            .frame_cnt = static_cast<t_i32>(frame_cnt),
        };

        const auto pcm = AllocArray<t_f32>(SampleCount(meta), snd_data_mem_arena);

        if (ma_decoder_read_pcm_frames(&decoder, pcm.raw, frame_cnt, nullptr) != MA_SUCCESS) {
            return false;
        }

        *o_snd_data = {meta, pcm};

        return true;
    }

    t_b8 PackSound(const s_str_rdonly file_path, const s_sound_data_mut snd_data, s_arena *const temp_mem_arena) {
        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        ZF_DEFINE_UNINITTED(s_stream, fs);

        if (!OpenFile(file_path, ek_file_access_mode_write, temp_mem_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(&fs); });

        return SerializeSound(fs, snd_data);
    }

    t_b8 UnpackSound(const s_str_rdonly file_path, s_arena *const snd_data_mem_arena, s_arena *const temp_mem_arena, s_sound_data_mut *const o_snd_data) {
        ZF_DEFINE_UNINITTED(s_stream, fs);

        if (!OpenFile(file_path, ek_file_access_mode_read, temp_mem_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(&fs); });

        s_sound_meta meta;

        if (!fs.ReadItem(meta)) {
            return false;
        }

        const auto pcm = AllocArray<t_f32>(SampleCount(meta), snd_data_mem_arena);

        if (!fs.ReadItemsIntoArray(pcm, pcm.len)) {
            return false;
        }

        *o_snd_data = {meta, pcm};

        return true;
    }

    t_b8 SerializeSound(s_stream *const stream, const s_sound_data_mut snd_data) {
        if (!stream->WriteItem(snd_data.meta)) {
            return false;
        }

        if (!SerializeArray(stream, snd_data.pcm)) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeSound(s_stream *const stream, s_arena *const snd_data_mem_arena, s_sound_data_mut *const o_snd_data) {
        ZF_DEFINE_UNINITTED(s_sound_meta, meta);

        if (!stream->ReadItem(meta)) {
            return false;
        }

        ZF_DEFINE_UNINITTED(s_array_mut<t_f32>, pcm);

        if (!DeserializeArray(stream, snd_data_mem_arena, pcm)) {
            return false;
        }

        *o_snd_data = {meta, pcm};

        return true;
    }
}
