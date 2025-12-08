#include <zcl/zcl_audio.h>

#include <miniaudio.h>
#include <zcl/zcl_io.h>

namespace zf {
    t_b8 LoadSoundFromRaw(const s_str_rdonly file_path, s_sound_data *const snd_data,
                          s_mem_arena *const snd_data_mem_arena,
                          s_mem_arena *const temp_mem_arena) {
        s_str file_path_terminated;

        if (!CloneStrButAddTerminator(file_path, *temp_mem_arena, file_path_terminated)) {
            return false;
        }

        ma_decoder decoder;
        ma_decoder_config decoder_config = ma_decoder_config_init(ma_format_f32, 0, 0);

        if (ma_decoder_init_file(StrRaw(file_path_terminated), &decoder_config, &decoder) !=
            MA_SUCCESS) {
            return false;
        }

        ZF_DEFER({ ma_decoder_uninit(&decoder); });

        ma_uint64 frame_cnt;

        if (ma_decoder_get_length_in_pcm_frames(&decoder, &frame_cnt) != MA_SUCCESS) {
            return false;
        }

        ZeroOut(snd_data);

        snd_data->meta.channel_cnt = static_cast<t_s32>(decoder.outputChannels);
        snd_data->meta.sample_rate = static_cast<t_s32>(decoder.outputSampleRate);
        snd_data->meta.frame_cnt = static_cast<t_s64>(frame_cnt);

        if (!InitArray(&snd_data->pcm, CalcSampleCount(snd_data->meta), snd_data_mem_arena)) {
            return false;
        }

        if (ma_decoder_read_pcm_frames(&decoder, snd_data->pcm.buf_raw, frame_cnt, nullptr) !=
            MA_SUCCESS) {
            return false;
        }

        return true;
    }

    t_b8 PackSound(const s_str_rdonly file_path, const s_sound_data snd_data,
                   s_mem_arena *const temp_mem_arena) {
        if (!CreateFileAndParentDirs(file_path, *temp_mem_arena)) {
            return false;
        }

        s_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_write, *temp_mem_arena, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        return SerializeSound(&fs, snd_data);
    }

    t_b8 UnpackSound(const s_str_rdonly file_path, s_sound_data *const snd_data,
                     s_mem_arena *const mem_arena, s_mem_arena *const temp_mem_arena) {
        s_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_read, *temp_mem_arena, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!StreamReadItem(fs, snd_data->meta)) {
            return false;
        }

        if (!InitArray(&snd_data->pcm, CalcSampleCount(snd_data->meta), mem_arena)) {
            return false;
        }

        if (!StreamReadItemsIntoArray(fs, snd_data->pcm, snd_data->pcm.len)) {
            return false;
        }

        return true;
    }

    t_b8 SerializeSound(s_stream *const stream, const s_sound_data snd_data) {
        if (!StreamWriteItem(*stream, snd_data.meta)) {
            return false;
        }

        if (!SerializeArray(*stream, snd_data.pcm)) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeSound(s_stream *const stream, s_sound_data *const snd_data,
                          s_mem_arena *const snd_data_mem_arena) {
        ZeroOut(snd_data);

        if (!StreamReadItem(*stream, snd_data->meta)) {
            return false;
        }

        if (!DeserializeArray(*stream, *snd_data_mem_arena, snd_data->pcm)) {
            return false;
        }

        return true;
    }
}
