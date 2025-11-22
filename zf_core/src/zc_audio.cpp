#include <zc/zc_audio.h>

#include <zc/zc_io.h>
#include <miniaudio.h>

namespace zf {
    t_b8 LoadSoundFromRaw(const s_str_rdonly file_path, s_sound_meta& o_snd_meta, s_array<t_f32>& o_snd_pcm, const s_allocator allocator) {
        ZF_ASSERT(IsStrTerminated(file_path));

        ma_decoder decoder;
        ma_decoder_config decoder_config = ma_decoder_config_init(ma_format_f32, 0, 0);

        if (ma_decoder_init_file(StrRaw(file_path), &decoder_config, &decoder) != MA_SUCCESS) {
            return false;
        }

        const t_b8 success = [&decoder, &allocator, &o_snd_meta, &o_snd_pcm]() {
            ma_uint64 frame_cnt;

            if (ma_decoder_get_length_in_pcm_frames(&decoder, &frame_cnt) != MA_SUCCESS) {
                return false;
            }

            o_snd_meta.channel_cnt = static_cast<t_s32>(decoder.outputChannels);
            o_snd_meta.sample_rate = static_cast<t_s32>(decoder.outputSampleRate);
            o_snd_meta.frame_cnt = static_cast<t_s64>(frame_cnt);

            if (!AllocArray(CalcSampleCount(o_snd_meta), o_snd_pcm, allocator)) {
                return false;
            }

            if (ma_decoder_read_pcm_frames(&decoder, o_snd_pcm.buf_raw, frame_cnt, nullptr) != MA_SUCCESS) {
                return false;
            }

            return true;
        }();

        ma_decoder_uninit(&decoder);

        return success;
    }

    t_b8 PackSound(const s_sound_data_rdonly& snd_data, const s_str_rdonly file_path, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(file_path));

        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_file_stream fs;

        if (!OpenFile(file_path, ec_file_access_mode::write, fs)) {
            return false;
        }

        const t_b8 success = [fs, &snd_data]() {
            if (!WriteItemToFile(fs, snd_data.meta)) {
                return false;
            }

            if (WriteItemArrayToFile(fs, snd_data.pcm) < snd_data.pcm.len) {
                return false;
            }

            return true;
        }();

        CloseFile(fs);

        return success;
    }

    t_b8 UnpackSound(const s_str_rdonly file_path, s_sound_data& o_snd_data, const s_allocator allocator) {
        ZF_ASSERT(IsStrTerminated(file_path));

        s_file_stream fs;

        if (!OpenFile(file_path, ec_file_access_mode::read, fs)) {
            return false;
        }

        const t_b8 success = [fs, &o_snd_data, &allocator]() {
            if (!ReadItemFromFile(fs, o_snd_data.meta)) {
                return false;
            }

            if (!AllocArray(CalcSampleCount(o_snd_data.meta), o_snd_data.pcm, allocator)) {
                return false;
            }

            if (ReadItemArrayFromFile(fs, o_snd_data.pcm) < o_snd_data.pcm.len) {
                return false;
            }

            return true;
        }();

        CloseFile(fs);

        return success;
    }
}
