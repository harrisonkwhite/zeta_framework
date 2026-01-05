#include <zcl/zcl_audio.h>

#include <miniaudio.h>

namespace zf {
    t_b8 f_audio_load_sound_data_from_raw(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, t_sound_data_mut *const o_snd_data) {
        const t_str_rdonly file_path_terminated = f_strs_clone_but_add_terminator(file_path, temp_arena);

        ma_decoder decoder;
        ma_decoder_config decoder_config = ma_decoder_config_init(ma_format_f32, 0, 0);

        if (ma_decoder_init_file(f_strs_get_as_cstr(file_path_terminated), &decoder_config, &decoder) != MA_SUCCESS) {
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

        o_snd_data->pcm = f_mem_push_array<t_f32>(snd_data_arena, f_audio_get_sample_cnt(o_snd_data->meta));

        if (ma_decoder_read_pcm_frames(&decoder, o_snd_data->pcm.raw, frame_cnt, nullptr) != MA_SUCCESS) {
            return false;
        }

        return true;
    }

    t_b8 f_audio_pack_sound(const t_str_rdonly file_path, const t_sound_data_mut snd_data, t_arena *const temp_arena) {
        if (!CreateFileAndParentDirectories(file_path, temp_arena)) {
            return false;
        }

        s_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ FileClose(&fs); });

        return f_audio_serialize_sound(&fs, snd_data);
    }

    t_b8 f_audio_unpack_sound(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, t_sound_data_mut *const o_snd_data) {
        s_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_read, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ FileClose(&fs); });

        if (!ReadItem(&fs, &o_snd_data->meta)) {
            return false;
        }

        o_snd_data->pcm = f_mem_push_array<t_f32>(snd_data_arena, f_audio_get_sample_cnt(o_snd_data->meta));

        if (!ReadItemsIntoArray(&fs, o_snd_data->pcm, o_snd_data->pcm.len)) {
            return false;
        }

        return true;
    }

    t_b8 f_audio_serialize_sound(s_stream *const stream, const t_sound_data_mut snd_data) {
        if (!WriteItem(stream, snd_data.meta)) {
            return false;
        }

        if (!SerializeArray(stream, snd_data.pcm)) {
            return false;
        }

        return true;
    }

    t_b8 f_audio_deserialize_sound(s_stream *const stream, t_arena *const snd_data_arena, t_sound_data_mut *const o_snd_data) {
        if (!ReadItem(stream, &o_snd_data->meta)) {
            return false;
        }

        if (!DeserializeArray(stream, snd_data_arena, &o_snd_data->pcm)) {
            return false;
        }

        return true;
    }
}
