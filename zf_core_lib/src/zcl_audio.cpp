#include <zcl/zcl_audio.h>

#include <miniaudio.h>

namespace zf {
    t_b8 LoadSoundDataFromRaw(const s_str_rdonly file_path, s_arena *const snd_data_arena, s_arena *const temp_arena, s_sound_data_mut *const o_snd_data) {
        const s_str_rdonly file_path_terminated = AllocStrCloneButAddTerminator(file_path, temp_arena);

        ma_decoder decoder;
        ma_decoder_config decoder_config = ma_decoder_config_init(ma_format_f32, 0, 0);

        if (ma_decoder_init_file(file_path_terminated.AsCstr(), &decoder_config, &decoder) != MA_SUCCESS) {
            return false;
        }

        ZF_DEFER({ ma_decoder_uninit(&decoder); });

        ma_uint64 frame_cnt;

        if (ma_decoder_get_length_in_pcm_frames(&decoder, &frame_cnt) != MA_SUCCESS) {
            return false;
        }

        o_snd_data->meta = {
            .sample_rate = static_cast<t_i32>(decoder.outputSampleRate),
            .channel_cnt = static_cast<t_i32>(decoder.outputChannels),
            .frame_cnt = static_cast<t_i32>(frame_cnt),
        };

        o_snd_data->pcm = PushArray<t_f32>(snd_data_arena, SampleCount(o_snd_data->meta));

        if (ma_decoder_read_pcm_frames(&decoder, o_snd_data->pcm.raw, frame_cnt, nullptr) != MA_SUCCESS) {
            return false;
        }

        return true;
    }

    t_b8 PackSound(const s_str_rdonly file_path, const s_sound_data_mut snd_data, s_arena *const temp_arena) {
        if (!CreateFileAndParentDirectories(file_path, temp_arena)) {
            return false;
        }

        c_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(&fs); });

        return SerializeSound(&fs, snd_data);
    }

    t_b8 UnpackSound(const s_str_rdonly file_path, s_arena *const snd_data_arena, s_arena *const temp_arena, s_sound_data_mut *const o_snd_data) {
        c_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_read, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(&fs); });

        if (!fs.ReadItem(&o_snd_data->meta)) {
            return false;
        }

        o_snd_data->pcm = PushArray<t_f32>(snd_data_arena, SampleCount(o_snd_data->meta));

        if (!fs.ReadItemsIntoArray(o_snd_data->pcm, o_snd_data->pcm.len)) {
            return false;
        }

        return true;
    }

    t_b8 SerializeSound(c_stream *const stream, const s_sound_data_mut snd_data) {
        if (!stream->WriteItem(snd_data.meta)) {
            return false;
        }

        if (!SerializeArray(stream, snd_data.pcm)) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeSound(c_stream *const stream, s_arena *const snd_data_arena, s_sound_data_mut *const o_snd_data) {
        if (!stream->ReadItem(&o_snd_data->meta)) {
            return false;
        }

        if (!DeserializeArray(stream, snd_data_arena, &o_snd_data->pcm)) {
            return false;
        }

        return true;
    }
}
