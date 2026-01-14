#include <zcl/zcl_audio.h>

#include <miniaudio.h>
#include <zcl/zcl_file_sys.h>

namespace zcl {
    t_b8 SoundLoadFromRaw(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, t_sound_data_mut *const o_snd_data) {
        const t_str_rdonly file_path_terminated = StrCloneButAddTerminator(file_path, temp_arena);

        ma_decoder decoder;
        ma_decoder_config decoder_config = ma_decoder_config_init(ma_format_f32, 0, 0);

        if (ma_decoder_init_file(StrToCStr(file_path_terminated), &decoder_config, &decoder) != MA_SUCCESS) {
            return false;
        }

        ZCL_DEFER({ ma_decoder_uninit(&decoder); });

        ma_uint64 frame_cnt;

        if (ma_decoder_get_length_in_pcm_frames(&decoder, &frame_cnt) != MA_SUCCESS) {
            return false;
        }

        o_snd_data->meta = {
            .sample_rate = static_cast<t_i32>(decoder.outputSampleRate),
            .channel_cnt = static_cast<t_i32>(decoder.outputChannels),
            .frame_cnt = static_cast<t_i32>(frame_cnt),
        };

        o_snd_data->pcm = arena_push_array<t_f32>(snd_data_arena, SoundCalcSampleCount(o_snd_data->meta));

        if (ma_decoder_read_pcm_frames(&decoder, o_snd_data->pcm.raw, frame_cnt, nullptr) != MA_SUCCESS) {
            return false;
        }

        return true;
    }

    t_b8 SoundPack(const t_str_rdonly file_path, const t_sound_data_rdonly snd_data, t_arena *const temp_arena) {
        if (!FileCreateRecursive(file_path, temp_arena)) {
            return false;
        }

        t_file_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        ZCL_DEFER({ FileClose(&fs); });

        return SoundSerialize(snd_data, fs);
    }

    t_b8 SoundUnpack(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, t_sound_data_mut *const o_snd_data) {
        t_file_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_read, temp_arena, &fs)) {
            return false;
        }

        ZCL_DEFER({ FileClose(&fs); });

        if (!stream_read_item(fs, &o_snd_data->meta)) {
            return false;
        }

        o_snd_data->pcm = arena_push_array<t_f32>(snd_data_arena, SoundCalcSampleCount(o_snd_data->meta));

        if (!stream_read_items_into_array(fs, o_snd_data->pcm, o_snd_data->pcm.len)) {
            return false;
        }

        return true;
    }

    t_b8 SoundSerialize(const t_sound_data_rdonly snd_data, const t_stream stream) {
        if (!stream_write_item(stream, snd_data.meta)) {
            return false;
        }

        if (!serialize_array(stream, snd_data.pcm)) {
            return false;
        }

        return true;
    }

    t_b8 SoundDeserialize(const t_stream stream, t_arena *const snd_data_arena, t_sound_data_mut *const o_snd_data) {
        if (!stream_read_item(stream, &o_snd_data->meta)) {
            return false;
        }

        if (!deserialize_array(stream, snd_data_arena, &o_snd_data->pcm)) {
            return false;
        }

        return true;
    }
}
