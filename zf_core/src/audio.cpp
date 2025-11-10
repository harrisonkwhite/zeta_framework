#include <zc/audio.h>

#include <zc/io.h>
#include <miniaudio.h>

namespace zf {
    bool s_sound_data::LoadFromRaw(c_mem_arena& mem_arena, const s_str_view file_path) {
        ZF_ASSERT(file_path.IsTerminated());

        ma_decoder decoder;

        if (ma_decoder_init_file(file_path.Raw(), nullptr, &decoder) != MA_SUCCESS) {
            return false;
        }

        const bool success = [this, &decoder, &mem_arena]() {
            ma_uint64 frame_cnt;

            if (ma_decoder_get_length_in_pcm_frames(&decoder, &frame_cnt) != MA_SUCCESS) {
                return false;
            }

            channel_cnt = static_cast<t_s32>(decoder.outputChannels);
            sample_rate = static_cast<t_s32>(decoder.outputSampleRate);

            const t_size total_sample_cnt = static_cast<t_size>(frame_cnt) * channel_cnt;

            if (!pcm.Init(mem_arena, total_sample_cnt)) {
                return false;
            }

            if (ma_decoder_read_pcm_frames(&decoder, pcm.Raw(), frame_cnt, nullptr) != MA_SUCCESS) {
                return false;
            }

            return true;
        }();

        ma_decoder_uninit(&decoder);

        return success;
    }

    bool PackSound(const s_str_view file_path, const s_sound_data& snd_data, c_mem_arena& temp_mem_arena) {
        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_file_stream fs;

        if (!fs.Open(file_path, true)) {
            return false;
        }

        const t_b8 success = [snd_data, fs]() {
            if (!fs.WriteItem(snd_data.channel_cnt)) {
                return false;
            }

            if (!fs.WriteItem(snd_data.sample_rate)) {
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

    bool UnpackSound(s_sound_data& snd_data, const s_str_view file_path) {
        return false;
    }
}
