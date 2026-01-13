#pragma once

#include <zcl/zcl_basic.h>
#include <zcl/zcl_arenas.h>

namespace zcl {
    enum t_stream_mode : t_i32 {
        ek_stream_mode_read,
        ek_stream_mode_write
    };

    struct t_stream {
        void *data;

        t_b8 (*read_func)(const t_stream stream, const t_array_mut<t_u8> dest_bytes);
        t_b8 (*write_func)(const t_stream stream, const t_array_rdonly<t_u8> src_bytes);

        t_stream_mode mode;
    };

    template <c_simple tp_type>
    [[nodiscard]] t_b8 stream_read_item(const t_stream stream, tp_type *const o_item) {
        ZF_ASSERT(stream.mode == ek_stream_mode_read);
        return stream.read_func(stream, to_bytes(o_item));
    }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 stream_write_item(const t_stream stream, const tp_type &item) {
        ZF_ASSERT(stream.mode == ek_stream_mode_write);
        return stream.write_func(stream, to_bytes(&item));
    }

    template <c_array_mut tp_arr_type>
    [[nodiscard]] t_b8 stream_read_items_into_array(const t_stream stream, const tp_arr_type arr, const t_i32 cnt) {
        ZF_ASSERT(stream.mode == ek_stream_mode_read);
        ZF_ASSERT(cnt >= 0 && cnt <= arr.len);

        if (cnt == 0) {
            return true;
        }

        return stream.read_func(stream, array_to_byte_array(array_slice(arr, 0, cnt)));
    }

    template <c_array tp_arr_type>
    [[nodiscard]] t_b8 stream_write_items_of_array(const t_stream stream, const tp_arr_type arr) {
        ZF_ASSERT(stream.mode == ek_stream_mode_write);

        if (arr.len == 0) {
            return true;
        }

        return stream.write_func(stream, array_to_byte_array(arr));
    }

    struct t_mem_stream {
        t_array_mut<t_u8> bytes;
        t_i32 byte_pos;

        t_stream_mode mode;

        operator t_stream() {
            const auto read_func = [](const t_stream stream, const t_array_mut<t_u8> dest_bytes) {
                ZF_ASSERT(stream.mode == ek_stream_mode_read);

                const auto mem_stream = static_cast<t_mem_stream *>(stream.data);

                if (mem_stream->byte_pos + dest_bytes.len > mem_stream->bytes.len) {
                    return false;
                }

                const t_array_rdonly<t_u8> src_bytes = array_slice_from(mem_stream->bytes, mem_stream->byte_pos);
                array_copy(src_bytes, dest_bytes, true);

                mem_stream->byte_pos += dest_bytes.len;

                return true;
            };

            const auto write_func = [](const t_stream stream, const t_array_rdonly<t_u8> src_bytes) {
                ZF_ASSERT(stream.mode == ek_stream_mode_write);

                const auto mem_stream = static_cast<t_mem_stream *>(stream.data);

                if (mem_stream->byte_pos + src_bytes.len > mem_stream->bytes.len) {
                    return false;
                }

                const t_array_mut<t_u8> dest_bytes = array_slice_from(mem_stream->bytes, mem_stream->byte_pos);
                array_copy(src_bytes, dest_bytes);

                mem_stream->byte_pos += src_bytes.len;

                return true;
            };

            return {
                .data = this,
                .read_func = read_func,
                .write_func = write_func,
                .mode = mode,
            };
        }
    };

    inline t_mem_stream mem_stream_create(const t_array_mut<t_u8> bytes, const t_stream_mode mode, const t_i32 pos = 0) {
        return {.bytes = bytes, .byte_pos = pos, .mode = mode};
    }

    inline t_array_mut<t_u8> mem_stream_get_bytes_written(const t_mem_stream *const stream) {
        ZF_ASSERT(stream->mode == ek_stream_mode_write);
        return array_slice(stream->bytes, 0, stream->byte_pos);
    }

    template <c_array tp_arr_type>
    [[nodiscard]] t_b8 stream_serialize_array(const t_stream stream, const tp_arr_type arr) {
        if (!stream_write_item(stream, arr.len)) {
            return false;
        }

        if (!stream_write_items_of_array(stream, arr)) {
            return false;
        }

        return true;
    }

    template <c_array_elem tp_elem_type>
    [[nodiscard]] t_b8 stream_deserialize_array(const t_stream stream, t_arena *const arr_arena, t_array_mut<tp_elem_type> *const o_arr) {
        t_i32 len;

        if (!stream_read_item(stream, &len)) {
            return false;
        }

        *o_arr = arena_push_array<tp_elem_type>(arr_arena, len);

        if (!stream_read_items_into_array(stream, *o_arr, len)) {
            return false;
        }

        return true;
    }
}
