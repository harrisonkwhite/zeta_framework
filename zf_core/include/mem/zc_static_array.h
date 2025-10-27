#pragma once

#include "zc_mem.h"

namespace zf {
    template<typename tp_type, int tp_len>
    struct s_static_array {
        tp_type buf_raw[tp_len] = {};

        s_static_array() = default;

        s_static_array(const tp_type (&arr)[tp_len]) {
            for (int i = 0; i < tp_len; i++) {
                buf_raw[i] = arr[i];
            }
        }

        int Len() const {
            return tp_len;
        }

        tp_type& operator[](const int index) {
            assert(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        const tp_type& operator[](const int index) const {
            assert(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        c_array<tp_type> Nonstatic() {
            return {buf_raw, tp_len};
        }

        c_array<const tp_type> Nonstatic() const {
            return {buf_raw, tp_len};
        }
    };
}
