#include <zc/audio.h>

#include <miniaudio.h>

namespace zf {
    bool s_audio_data::LoadFromRaw(const s_str_view file_path, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(file_path.IsTerminated());

        ma_decoder decoder;

        if (ma_decoder_init_file(file_path.Raw(), nullptr, &decoder) != MA_SUCCESS) {
            return false;
        }

        const bool success = [this, &decoder, &temp_mem_arena]() {
            ma_uint64 frame_cnt;

            if (ma_decoder_get_length_in_pcm_frames(&decoder, &frame_cnt) != MA_SUCCESS) {
                return false;
            }

            channel_cnt = static_cast<t_s32>(decoder.outputChannels);
            sample_rate = static_cast<t_s32>(decoder.outputSampleRate);

            const t_size total_sample_cnt = static_cast<t_size>(frame_cnt) * channel_cnt;

            if (!pcm.Init(temp_mem_arena, total_sample_cnt)) {
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
};
