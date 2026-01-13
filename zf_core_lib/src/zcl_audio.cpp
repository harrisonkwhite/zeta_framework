#include <zcl/zcl_audio.h>

#include <miniaudio.h>
#include <zcl/zcl_file_sys.h>

namespace zcl {
    t_b8 sound_load_from_raw(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, t_sound_data_mut *const o_snd_data) {
        const t_str_rdonly file_path_terminated = str_clone_but_add_terminator(file_path, temp_arena);

        ma_decoder decoder;
        ma_decoder_config decoder_config = ma_decoder_config_init(ma_format_f32, 0, 0);

        if (ma_decoder_init_file(str_to_cstr(file_path_terminated), &decoder_config, &decoder) != MA_SUCCESS) {
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

        o_snd_data->pcm = arena_push_array<t_f32>(snd_data_arena, sound_get_sample_cnt(o_snd_data->meta));

        if (ma_decoder_read_pcm_frames(&decoder, o_snd_data->pcm.raw, frame_cnt, nullptr) != MA_SUCCESS) {
            return false;
        }

        return true;
    }

    t_b8 sound_pack(const t_str_rdonly file_path, const t_sound_data_rdonly snd_data, t_arena *const temp_arena) {
        if (!file_sys::create_file_and_parent_directories(file_path, temp_arena)) {
            return false;
        }

        file_sys::t_file_stream fs;

        if (!file_sys::file_open(file_path, file_sys::ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ file_sys::file_close(&fs); });

        return sound_serialize(snd_data, fs);
    }

    t_b8 sound_unpack(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, t_sound_data_mut *const o_snd_data) {
        file_sys::t_file_stream fs;

        if (!file_sys::file_open(file_path, file_sys::ek_file_access_mode_read, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ file_sys::file_close(&fs); });

        if (!stream_read_item(fs, &o_snd_data->meta)) {
            return false;
        }

        o_snd_data->pcm = arena_push_array<t_f32>(snd_data_arena, sound_get_sample_cnt(o_snd_data->meta));

        if (!stream_read_items_into_array(fs, o_snd_data->pcm, o_snd_data->pcm.len)) {
            return false;
        }

        return true;
    }

    t_b8 sound_serialize(const t_sound_data_rdonly snd_data, const t_stream stream) {
        if (!stream_write_item(stream, snd_data.meta)) {
            return false;
        }

        if (!stream_serialize_array(stream, snd_data.pcm)) {
            return false;
        }

        return true;
    }

    t_b8 sound_deserialize(const t_stream stream, t_arena *const snd_data_arena, t_sound_data_mut *const o_snd_data) {
        if (!stream_read_item(stream, &o_snd_data->meta)) {
            return false;
        }

        if (!stream_deserialize_array(stream, snd_data_arena, &o_snd_data->pcm)) {
            return false;
        }

        return true;
    }
}
