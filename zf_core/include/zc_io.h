#pragma once

#include <cstdio>
#include "zc_mem.h"

namespace zf {
    class c_file_reader {
    public:
        ~c_file_reader() {
            if (m_fs && m_close_deferred) {
                Close();
            }
        }

        [[nodiscard]] bool Open(const c_string_view file_path) {
            assert(!m_fs);
            m_fs = fopen(file_path.Raw(), "rb");
            return m_fs;
        }

        void Close() {
            assert(m_fs);
            fclose(m_fs);
        }

        void DeferClose() {
            m_close_deferred = true;
        }

        size_t CalcSize() {
            assert(m_fs);

            const auto pos_old = ftell(m_fs);
            fseek(m_fs, 0, SEEK_END);
            const size_t file_size = ftell(m_fs);
            fseek(m_fs, pos_old, SEEK_SET);
            return pos_old;
        }

        template<typename tp_type>
        [[nodiscard]] int Read(const c_array<tp_type> arr) {
            assert(m_fs);
            assert(arr.Raw() && arr.Len() > 0);

            return fread(arr.Raw(), sizeof(tp_type), arr.Len(), m_fs);
        }

        template<typename tp_type>
        [[nodiscard]] bool ReadItem(tp_type& item) {
            assert(m_fs);
            return fread(&item, sizeof(tp_type), 1, m_fs) == 1;
        }

    private:
        FILE* m_fs = nullptr;
        bool m_close_deferred = false;
    };

    class c_file_writer {
    public:
        ~c_file_writer() {
            if (m_fs && m_close_deferred) {
                Close();
            }
        }

        [[nodiscard]] bool Open(const c_string_view file_path) {
            assert(!m_fs);
            m_fs = fopen(file_path.Raw(), "wb");
            return m_fs;
        }

        void Close() {
            assert(m_fs);
            fclose(m_fs);
        }

        void DeferClose() {
            m_close_deferred = true;
        }

        size_t CalcSize() {
            assert(m_fs);

            const auto pos_old = ftell(m_fs);
            fseek(m_fs, 0, SEEK_END);
            const size_t file_size = ftell(m_fs);
            fseek(m_fs, pos_old, SEEK_SET);
            return pos_old;
        }

        template<typename tp_type>
        [[nodiscard]] int Write(const c_array<const tp_type> arr) {
            assert(m_fs);
            assert(arr.Raw() && arr.Len() > 0);

            return fwrite(arr.Raw(), sizeof(tp_type), arr.Len(), m_fs);
        }

        template<typename tp_type>
        [[nodiscard]] bool WriteItem(const tp_type& item) {
            assert(m_fs);
            return fwrite(&item, sizeof(tp_type), 1, m_fs) == 1;
        }

    private:
        FILE* m_fs = nullptr;
        bool m_close_deferred = false;
    };

    c_array<t_u8> LoadFileContents(const c_string_view file_path, c_mem_arena& mem_arena, const bool include_terminating_byte = false);
}
