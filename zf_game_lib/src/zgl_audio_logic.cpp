#include <zgl/zgl_audio_private.h>

namespace zgl {
    struct t_sound_type_group {
        zcl::t_b8 valid;

        zcl::t_arena *arena;

        t_sound_type *head;
        t_sound_type *tail;
    };

    t_sound_type_group *SoundTypeGroupCreate(const t_audio_ticket_mut audio_ticket, zcl::t_arena *const arena) {
        ZCL_ASSERT(TicketCheckValid(audio_ticket));

        const auto result = zcl::ArenaPush<t_sound_type_group>(arena);
        result->valid = true;
        result->arena = arena;

        return result;
    }

    void SoundTypeGroupDestroy(const t_audio_ticket_mut audio_ticket, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        ZCL_ASSERT(group->valid);

        // All sound instances of any of the sound types in the group need to be destroyed.
        t_sound_type *snd_type = group->head;

        while (snd_type) {
            SoundsDestroyAllOfType(audio_ticket, snd_type, temp_arena);

            t_sound_type *const snd_type_next = snd_type->next;
            *snd_type = {};
            snd_type = snd_type_next;
        }

        *group = {};
    }

    static t_sound_type *SoundTypeGroupAdd(t_sound_type_group *const group) {
        const auto result = zcl::ArenaPush<t_sound_type>(group->arena);
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

    t_sound_type *SoundTypeCreateFromBuilt(const t_audio_ticket_mut audio_ticket, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        ZCL_ASSERT(group->valid);

        zcl::t_file_stream file_stream;

        if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
            ZCL_FATAL();
        }

        zcl::t_sound_data_mut snd_data;

        if (!zcl::DeserializeSound(zcl::FileStreamGetView(&file_stream), group->arena, &snd_data)) {
            ZCL_FATAL();
        }

        zcl::FileClose(&file_stream);

        t_sound_type *const result = SoundTypeGroupAdd(group);
        result->streamable = false;
        result->nonstream_snd_data = snd_data;

        return result;
    }

    t_sound_type *SoundTypeCreateFromUnbuilt(const t_audio_ticket_mut audio_ticket, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        ZCL_ASSERT(group->valid);

        zcl::t_sound_data_mut snd_data;

        if (!zcl::SoundLoadFromUnbuilt(file_path, group->arena, temp_arena, &snd_data)) {
            ZCL_FATAL();
        }

        t_sound_type *const result = SoundTypeGroupAdd(group);
        result->streamable = false;
        result->nonstream_snd_data = snd_data;

        return result;
    }

    t_sound_type *SoundTypeCreateStreamable(const t_audio_ticket_mut audio_ticket, const zcl::t_str_rdonly external_file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        ZCL_ASSERT(group->valid);

        t_sound_type *const result = SoundTypeGroupAdd(group);
        result->streamable = true;
        result->stream_unbuilt_file_path_terminated = zcl::StrCloneButAddTerminator(external_file_path, temp_arena);

        return result;
    }

    zcl::t_b8 SoundTypeCheckStreamable(const t_audio_ticket_rdonly audio_ticket, const t_sound_type *const type) {
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        ZCL_ASSERT(type->valid);

        return type->streamable;
    }

    void SoundsDestroyAll(const t_audio_ticket_mut audio_ticket, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(audio_ticket));

        const auto snd_ids = SoundsGetExisting(audio_ticket, temp_arena);

        for (zcl::t_i32 i = 0; i < snd_ids.len; i++) {
            SoundDestroy(audio_ticket, snd_ids[i]);
        }
    }

    void SoundsDestroyAllOfType(const t_audio_ticket_mut audio_ticket, const t_sound_type *const snd_type, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(audio_ticket));

        const auto snd_ids = SoundsGetExisting(audio_ticket, temp_arena);

        for (zcl::t_i32 i = 0; i < snd_ids.len; i++) {
            if (SoundGetType(audio_ticket, snd_ids[i]) == snd_type) {
                SoundDestroy(audio_ticket, snd_ids[i]);
            }
        }
    }

    void SoundsPauseAll(const t_audio_ticket_mut audio_ticket, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(audio_ticket));

        const auto snd_ids = SoundsGetExisting(audio_ticket, temp_arena);

        for (zcl::t_i32 i = 0; i < snd_ids.len; i++) {
            if (SoundGetState(audio_ticket, snd_ids[i]) == ek_sound_state_playing) {
                SoundPause(audio_ticket, snd_ids[i]);
            }
        }
    }

    void SoundsPauseAllOfType(const t_audio_ticket_mut audio_ticket, const t_sound_type *const snd_type, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(audio_ticket));

        const auto snd_ids = SoundsGetExisting(audio_ticket, temp_arena);

        for (zcl::t_i32 i = 0; i < snd_ids.len; i++) {
            if (SoundGetType(audio_ticket, snd_ids[i]) == snd_type && SoundGetState(audio_ticket, snd_ids[i]) == ek_sound_state_playing) {
                SoundPause(audio_ticket, snd_ids[i]);
            }
        }
    }

    void SoundsResumeAll(const t_audio_ticket_mut audio_ticket, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(audio_ticket));

        const auto snd_ids = SoundsGetExisting(audio_ticket, temp_arena);

        for (zcl::t_i32 i = 0; i < snd_ids.len; i++) {
            if (SoundGetState(audio_ticket, snd_ids[i]) == ek_sound_state_paused) {
                SoundResume(audio_ticket, snd_ids[i]);
            }
        }
    }

    void SoundsResumeAllOfType(const t_audio_ticket_mut audio_ticket, const t_sound_type *const snd_type, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(TicketCheckValid(audio_ticket));

        const auto snd_ids = SoundsGetExisting(audio_ticket, temp_arena);

        for (zcl::t_i32 i = 0; i < snd_ids.len; i++) {
            if (SoundGetType(audio_ticket, snd_ids[i]) == snd_type && SoundGetState(audio_ticket, snd_ids[i]) == ek_sound_state_paused) {
                SoundResume(audio_ticket, snd_ids[i]);
            }
        }
    }
}
