#pragma once

#include <zcl/zcl_basic.h>
#include <zcl/zcl_bits.h>
#include <zcl/zcl_hash_maps.h>

namespace zcl {
    // ============================================================
    // @section: Core

    enum t_stream_mode : t_i32 {
        ek_stream_mode_read,
        ek_stream_mode_write
    };

    struct t_stream_view {
        void *data;

        t_b8 (*read_func)(const t_stream_view stream, const t_array_mut<t_u8> dest_bytes);
        t_b8 (*write_func)(const t_stream_view stream, const t_array_rdonly<t_u8> src_bytes);

        t_stream_mode mode;
    };

    template <c_simple tp_type>
    [[nodiscard]] t_b8 stream_read_item(const t_stream_view stream_view, tp_type *const o_item) {
        ZCL_ASSERT(stream_view.mode == ek_stream_mode_read);
        return stream_view.read_func(stream_view, to_bytes(o_item));
    }

    template <c_simple tp_type>
    [[nodiscard]] t_b8 stream_write_item(const t_stream_view stream_view, const tp_type &item) {
        ZCL_ASSERT(stream_view.mode == ek_stream_mode_write);
        return stream_view.write_func(stream_view, to_bytes(&item));
    }

    template <c_array_mut tp_arr_type>
    [[nodiscard]] t_b8 stream_read_items_into_array(const t_stream_view stream_view, const tp_arr_type arr, const t_i32 cnt) {
        ZCL_ASSERT(stream_view.mode == ek_stream_mode_read);
        ZCL_ASSERT(cnt >= 0 && cnt <= arr.len);

        if (cnt == 0) {
            return true;
        }

        return stream_view.read_func(stream_view, array_to_byte_array(array_slice(arr, 0, cnt)));
    }

    template <c_array tp_arr_type>
    [[nodiscard]] t_b8 stream_write_items_of_array(const t_stream_view stream_view, const tp_arr_type arr) {
        ZCL_ASSERT(stream_view.mode == ek_stream_mode_write);

        if (arr.len == 0) {
            return true;
        }

        return stream_view.write_func(stream_view, array_to_byte_array(arr));
    }

    // ============================================================


    // ============================================================
    // @section: Memory Stream

    struct t_mem_stream {
        t_array_mut<t_u8> bytes;
        t_i32 byte_pos;

        t_stream_mode mode;

        operator t_stream_view() {
            const auto read_func = [](const t_stream_view stream_view, const t_array_mut<t_u8> dest_bytes) {
                ZCL_ASSERT(stream_view.mode == ek_stream_mode_read);

                const auto mem_stream = static_cast<t_mem_stream *>(stream_view.data);

                if (mem_stream->byte_pos + dest_bytes.len > mem_stream->bytes.len) {
                    return false;
                }

                const t_array_rdonly<t_u8> src_bytes = array_slice_from(mem_stream->bytes, mem_stream->byte_pos);
                array_copy(src_bytes, dest_bytes, true);

                mem_stream->byte_pos += dest_bytes.len;

                return true;
            };

            const auto write_func = [](const t_stream_view stream_view, const t_array_rdonly<t_u8> src_bytes) {
                ZCL_ASSERT(stream_view.mode == ek_stream_mode_write);

                const auto mem_stream = static_cast<t_mem_stream *>(stream_view.data);

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
        ZCL_ASSERT(stream->mode == ek_stream_mode_write);
        return array_slice(stream->bytes, 0, stream->byte_pos);
    }

    // ============================================================
}
