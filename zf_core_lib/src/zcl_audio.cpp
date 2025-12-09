#include <zcl/zcl_audio.h>

#include <miniaudio.h>

namespace zf {
    t_b8 LoadSoundFromRaw(const s_str_rdonly file_path, s_mem_arena *const snd_data_mem_arena, s_mem_arena *const temp_mem_arena, s_sound_data *const o_snd_data) {
        ZF_ASSERT(IsStrTerminated(file_path));

        MarkUninitted(o_snd_data);

        ma_decoder decoder;
        ma_decoder_config decoder_config = ma_decoder_config_init(ma_format_f32, 0, 0);

        if (ma_decoder_init_file(StrRaw(file_path), &decoder_config, &decoder) != MA_SUCCESS) {
            return false;
        }

        ZF_DEFER({ ma_decoder_uninit(&decoder); });

        ma_uint64 frame_cnt;

        if (ma_decoder_get_length_in_pcm_frames(&decoder, &frame_cnt) != MA_SUCCESS) {
            return false;
        }

        o_snd_data->meta.channel_cnt = static_cast<t_i32>(decoder.outputChannels);
        o_snd_data->meta.sample_rate = static_cast<t_i32>(decoder.outputSampleRate);
        o_snd_data->meta.frame_cnt = static_cast<t_i64>(frame_cnt);

        if (!AllocArray(CalcSampleCount(o_snd_data->meta), snd_data_mem_arena, &o_snd_data->pcm)) {
            return false;
        }

        if (ma_decoder_read_pcm_frames(&decoder, o_snd_data->pcm.buf, frame_cnt, nullptr) != MA_SUCCESS) {
            return false;
        }

        return true;
    }

    t_b8 PackSound(const s_str_rdonly file_path, const s_sound_data snd_data, s_mem_arena *const temp_mem_arena) {
        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_write, &fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(&fs); });

        return SerializeSound(&fs, snd_data);
    }

    t_b8 UnpackSound(const s_str_rdonly file_path, s_mem_arena *const snd_data_mem_arena, s_mem_arena *const temp_mem_arena, s_sound_data *const o_snd_data) {
        MarkUninitted(o_snd_data);

        s_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_read, &fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(&fs); });

        if (!StreamReadItem(&fs, &o_snd_data->meta)) {
            return false;
        }

        if (!AllocArray(CalcSampleCount(o_snd_data->meta), snd_data_mem_arena, &o_snd_data->pcm)) {
            return false;
        }

        if (!StreamReadItemsIntoArray(&fs, o_snd_data->pcm, o_snd_data->pcm.len)) {
            return false;
        }

        return true;
    }

    t_b8 SerializeSound(s_stream *const stream, const s_sound_data snd_data) {
        if (!StreamWriteItem(stream, snd_data.meta)) {
            return false;
        }

        if (!SerializeArray(stream, snd_data.pcm)) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeSound(s_stream *const stream, s_mem_arena *const snd_data_mem_arena, s_sound_data *const o_snd_data) {
        MarkUninitted(o_snd_data);

        if (!StreamReadItem(stream, &o_snd_data->meta)) {
            return false;
        }

        if (!DeserializeArray(stream, snd_data_mem_arena, &o_snd_data->pcm)) {
            return false;
        }

        return true;
    }
}
