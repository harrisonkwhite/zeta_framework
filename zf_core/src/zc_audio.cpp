#include <zc/zc_audio.h>

#include <zc/zc_io.h>
#include <miniaudio.h>

namespace zf {
    t_b8 LoadSoundFromRaw(const s_str_view file_path, c_mem_arena& mem_arena, s_sound_data& o_snd_data) {
        ZF_ASSERT(file_path.IsTerminated());

        ma_decoder decoder;

        if (ma_decoder_init_file(file_path.Raw(), nullptr, &decoder) != MA_SUCCESS) {
            return false;
        }

        const t_b8 success = [&decoder, &mem_arena, &o_snd_data]() {
            ma_uint64 frame_cnt;

            if (ma_decoder_get_length_in_pcm_frames(&decoder, &frame_cnt) != MA_SUCCESS) {
                return false;
            }

            o_snd_data.meta.channel_cnt = static_cast<t_s32>(decoder.outputChannels);
            o_snd_data.meta.sample_rate = static_cast<t_s32>(decoder.outputSampleRate);
            o_snd_data.meta.frame_cnt = static_cast<t_s64>(frame_cnt);

            if (!mem_arena.PushArray(o_snd_data.meta.SampleCount(), o_snd_data.pcm)) {
                return false;
            }

            if (ma_decoder_read_pcm_frames(&decoder, o_snd_data.pcm.Raw(), frame_cnt, nullptr) != MA_SUCCESS) {
                return false;
            }

            return true;
        }();

        ma_decoder_uninit(&decoder);

        return success;
    }

    t_b8 LoadSoundFromPacked(const s_str_view file_path, c_mem_arena& mem_arena, s_sound_data& o_snd_data) {
        ZF_ASSERT(file_path.IsTerminated());

        s_file_stream fs;

        if (!fs.Open(file_path, ec_file_access_mode::read)) {
            return false;
        }

        const t_b8 success = [fs, &mem_arena, &o_snd_data]() {
            if (!fs.ReadItem(o_snd_data.meta)) {
                return false;
            }

            if (!mem_arena.PushArray(o_snd_data.meta.SampleCount(), o_snd_data.pcm)) {
                return false;
            }

            if (fs.ReadItems(o_snd_data.pcm) < o_snd_data.pcm.Len()) {
                return false;
            }

            return true;
        }();

        fs.Close();

        return success;
    }

    t_b8 PackSound(const s_sound_data_view& snd_data, const s_str_view file_path, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(file_path.IsTerminated());

        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_file_stream fs;

        if (!fs.Open(file_path, ec_file_access_mode::write)) {
            return false;
        }

        const t_b8 success = [fs, snd_data]() {
            if (!fs.WriteItem(snd_data.meta)) {
                return false;
            }

            if (fs.WriteItems(snd_data.pcm.View()) < snd_data.pcm.Len()) {
                return false;
            }

            return true;
        }();

        fs.Close();

        return success;
    }
}
