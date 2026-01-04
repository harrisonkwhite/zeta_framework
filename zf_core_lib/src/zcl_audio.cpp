#include <zcl/zcl_audio.h>

#include <miniaudio.h>

namespace zf::audio {
    B8 load_sound_data_from_raw(const strs::StrRdonly file_path, s_arena *const snd_data_arena, s_arena *const temp_arena, SoundDataMut *const o_snd_data) {
        const strs::StrRdonly file_path_terminated = clone_str_but_add_terminator(file_path, temp_arena);

        ma_decoder decoder;
        ma_decoder_config decoder_config = ma_decoder_config_init(ma_format_f32, 0, 0);

        if (ma_decoder_init_file(get_as_cstr(file_path_terminated), &decoder_config, &decoder) != MA_SUCCESS) {
            return false;
        }

        ZF_DEFER({ ma_decoder_uninit(&decoder); });

        ma_uint64 frame_cnt;

        if (ma_decoder_get_length_in_pcm_frames(&decoder, &frame_cnt) != MA_SUCCESS) {
            return false;
        }

        o_snd_data->meta = {
            .sample_rate = static_cast<I32>(decoder.outputSampleRate),
            .channel_cnt = static_cast<I32>(decoder.outputChannels),
            .frame_cnt = static_cast<I32>(frame_cnt),
        };

        o_snd_data->pcm = ArenaPushArray<F32>(snd_data_arena, get_sample_cnt(o_snd_data->meta));

        if (ma_decoder_read_pcm_frames(&decoder, o_snd_data->pcm.raw, frame_cnt, nullptr) != MA_SUCCESS) {
            return false;
        }

        return true;
    }

    B8 pack_sound(const strs::StrRdonly file_path, const SoundDataMut snd_data, s_arena *const temp_arena) {
        if (!CreateFileAndParentDirectories(file_path, temp_arena)) {
            return false;
        }

        s_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ FileClose(&fs); });

        return serialize_sound(&fs, snd_data);
    }

    B8 unpack_sound(const strs::StrRdonly file_path, s_arena *const snd_data_arena, s_arena *const temp_arena, SoundDataMut *const o_snd_data) {
        s_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_read, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ FileClose(&fs); });

        if (!ReadItem(&fs, &o_snd_data->meta)) {
            return false;
        }

        o_snd_data->pcm = ArenaPushArray<F32>(snd_data_arena, get_sample_cnt(o_snd_data->meta));

        if (!ReadItemsIntoArray(&fs, o_snd_data->pcm, o_snd_data->pcm.len)) {
            return false;
        }

        return true;
    }

    B8 serialize_sound(s_stream *const stream, const SoundDataMut snd_data) {
        if (!WriteItem(stream, snd_data.meta)) {
            return false;
        }

        if (!SerializeArray(stream, snd_data.pcm)) {
            return false;
        }

        return true;
    }

    B8 deserialize_sound(s_stream *const stream, s_arena *const snd_data_arena, SoundDataMut *const o_snd_data) {
        if (!ReadItem(stream, &o_snd_data->meta)) {
            return false;
        }

        if (!DeserializeArray(stream, snd_data_arena, &o_snd_data->pcm)) {
            return false;
        }

        return true;
    }
}
