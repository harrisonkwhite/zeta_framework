#include <zc/zc_audio.h>

#include <zc/zc_io.h>
#include <miniaudio.h>

namespace zf {
    t_b8 LoadSoundFromRaw(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_sound_meta& o_snd_meta, s_array<t_f32>& o_snd_pcm) {
        ZF_ASSERT(IsStrTerminated(file_path));

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

        o_snd_meta.channel_cnt = static_cast<t_s32>(decoder.outputChannels);
        o_snd_meta.sample_rate = static_cast<t_s32>(decoder.outputSampleRate);
        o_snd_meta.frame_cnt = static_cast<t_s64>(frame_cnt);

        if (!MakeArray(mem_arena, CalcSampleCount(o_snd_meta), o_snd_pcm)) {
            return false;
        }

        if (ma_decoder_read_pcm_frames(&decoder, o_snd_pcm.buf_raw, frame_cnt, nullptr) != MA_SUCCESS) {
            return false;
        }

        return true;
    }

    t_b8 PackSound(const s_sound_data_rdonly& snd_data, const s_str_rdonly file_path, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(file_path));

        ZF_DEFER_MEM_ARENA_REWIND(temp_mem_arena);

        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_file_stream fs;

        if (!OpenFile(file_path, e_file_access_mode::write, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!WriteItemToFile(fs, snd_data.meta)) {
            return false;
        }

        if (WriteItemArrayToFile(fs, snd_data.pcm) < snd_data.pcm.len) {
            return false;
        }

        return true;
    }

    t_b8 UnpackSound(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_sound_data& o_snd_data) {
        ZF_ASSERT(IsStrTerminated(file_path));

        s_file_stream fs;

        if (!OpenFile(file_path, e_file_access_mode::read, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!ReadItemFromFile(fs, o_snd_data.meta)) {
            return false;
        }

        if (!MakeArray(mem_arena, CalcSampleCount(o_snd_data.meta), o_snd_data.pcm)) {
            return false;
        }

        if (ReadItemArrayFromFile(fs, o_snd_data.pcm) < o_snd_data.pcm.len) {
            return false;
        }

        return true;
    }
}
