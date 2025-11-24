#pragma once

#include <zc/zc_io.h>

namespace zf {
    template<c_array tp_type>
    [[nodiscard]] inline t_b8 SerializeArray(s_stream& stream, tp_type& arr) {
        if (!SerializeItem(stream, arr.len)) {
            return false;
        }

        if (!SerializeItems(stream, arr)) {
            return false;
        }

        return true;
    }

    template<typename tp_type>
    [[nodiscard]] inline t_b8 DeserializeArray(s_mem_arena& mem_arena, s_stream& stream, s_array<tp_type>& o_arr) {
        if (!DeserializeItem(stream, o_arr.len)) {
            return false;
        }

        if (!MakeArray(mem_arena, o_arr.len, o_arr)) {
            return false;
        }

        if (!DeserializeItems(stream, o_arr, o_arr.len)) {
            return false;
        }

        return true;
    }
}
