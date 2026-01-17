#include <zgl/zgl_audio_private.h>

namespace zgl {
    t_sound_type_group *SoundTypeGroupCreate(t_audio_sys *const audio_sys, zcl::t_arena *const arena) {
        const auto result = zcl::ArenaPushItem<t_sound_type_group>(arena);
        result->valid = true;
        result->arena = arena;
        return result;
    }

    void SoundTypeGroupDestroy(t_audio_sys *const audio_sys, t_sound_type_group *const group) {
        ZCL_ASSERT(group->valid);

        // All sound instances of any of the sound types in the group need to be destroyed.
        t_sound_type *snd_type = group->head;

        while (snd_type) {
            SoundsDestroyAllOfType(audio_sys, snd_type);

            t_sound_type *const snd_type_next = snd_type->next;
            *snd_type = {};
            snd_type = snd_type_next;
        }

        *group = {};
    }

    static t_sound_type *SoundTypeGroupAdd(t_sound_type_group *const group) {
        const auto result = zcl::ArenaPushItem<t_sound_type>(group->arena);
        result->valid = true;

        if (!group->head) {
            group->head = result;
            group->tail = result;
        } else {
            group->tail->next = result;
            group->tail = result;
        }

        return result;
    }

    t_sound_type *SoundTypeCreateFromExternal(t_audio_sys *const audio_sys, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(group->valid);

        zcl::t_sound_data_mut snd_data;

        if (!zcl::SoundLoadFromExternal(file_path, group->arena, temp_arena, &snd_data)) {
            ZCL_FATAL();
        }

        t_sound_type *const result = SoundTypeGroupAdd(group);
        result->streamable = false;
        result->nonstream_snd_data = snd_data;

        return result;
    }

    t_sound_type *SoundTypeCreateFromPacked(t_audio_sys *const audio_sys, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(group->valid);

        zcl::t_file_stream file_stream;

        if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
            ZCL_FATAL();
        }

        zcl::t_sound_data_mut snd_data;

        if (!zcl::DeserializeSound(file_stream, group->arena, &snd_data)) {
            ZCL_FATAL();
        }

        zcl::FileClose(&file_stream);

        t_sound_type *const result = SoundTypeGroupAdd(group);
        result->streamable = false;
        result->nonstream_snd_data = snd_data;

        return result;
    }

    t_sound_type *SoundTypeCreateStreamable(t_audio_sys *const audio_sys, const zcl::t_str_rdonly external_file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(group->valid);

        t_sound_type *const result = SoundTypeGroupAdd(group);
        result->streamable = true;
        result->stream_external_file_path_terminated = zcl::StrCloneButAddTerminator(external_file_path, temp_arena);

        return result;
    }

    zcl::t_b8 SoundTypeCheckStreamable(t_audio_sys *const audio_sys, const t_sound_type *const type) {
        ZCL_ASSERT(type->valid);
        return type->streamable;
    }
}
