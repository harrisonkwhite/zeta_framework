#pragma once

#include <cstdio>
#include "zc_mem.h"

#define ZF_ANSI_ESC "\x1b"

#define ZF_ANSI_RESET ZF_ANSI_ESC "[0m"

#define ZF_ANSI_BOLD ZF_ANSI_ESC "[1m"
#define ZF_ANSI_DIM ZF_ANSI_ESC "[2m"
#define ZF_ANSI_UNDERLINE ZF_ANSI_ESC "[4m"
#define ZF_ANSI_REVERSED ZF_ANSI_ESC "[7m"

#define ZF_ANSI_FG_BLACK ZF_ANSI_ESC "[30m"
#define ZF_ANSI_FG_BBLACK ZF_ANSI_ESC "[90m"
#define ZF_ANSI_FG_RED ZF_ANSI_ESC "[31m"
#define ZF_ANSI_FG_BRED ZF_ANSI_ESC "[91m"
#define ZF_ANSI_FG_GREEN ZF_ANSI_ESC "[32m"
#define ZF_ANSI_FG_BGREEN ZF_ANSI_ESC "[92m"
#define ZF_ANSI_FG_YELLOW ZF_ANSI_ESC "[33m"
#define ZF_ANSI_FG_BYELLOW ZF_ANSI_ESC "[93m"
#define ZF_ANSI_FG_BLUE ZF_ANSI_ESC "[34m"
#define ZF_ANSI_FG_BBLUE ZF_ANSI_ESC "[94m"
#define ZF_ANSI_FG_MAGENTA ZF_ANSI_ESC "[35m"
#define ZF_ANSI_FG_BMAGENTA ZF_ANSI_ESC "[95m"
#define ZF_ANSI_FG_CYAN ZF_ANSI_ESC "[36m"
#define ZF_ANSI_FG_BCYAN ZF_ANSI_ESC "[96m"
#define ZF_ANSI_FG_WHITE ZF_ANSI_ESC "[37m"
#define ZF_ANSI_FG_BWHITE ZF_ANSI_ESC "[97m"

#define ZF_ANSI_BG_BLACK ZF_ANSI_ESC "[40m"
#define ZF_ANSI_BG_BBLACK ZF_ANSI_ESC "[100m"
#define ZF_ANSI_BG_RED ZF_ANSI_ESC "[41m"
#define ZF_ANSI_BG_BRED ZF_ANSI_ESC "[101m"
#define ZF_ANSI_BG_GREEN ZF_ANSI_ESC "[42m"
#define ZF_ANSI_BG_BGREEN ZF_ANSI_ESC "[102m"
#define ZF_ANSI_BG_YELLOW ZF_ANSI_ESC "[43m"
#define ZF_ANSI_BG_BYELLOW ZF_ANSI_ESC "[103m"
#define ZF_ANSI_BG_BLUE ZF_ANSI_ESC "[44m"
#define ZF_ANSI_BG_BBLUE ZF_ANSI_ESC "[104m"
#define ZF_ANSI_BG_MAGENTA ZF_ANSI_ESC "[45m"
#define ZF_ANSI_BG_BMAGENTA ZF_ANSI_ESC "[105m"
#define ZF_ANSI_BG_CYAN ZF_ANSI_ESC "[46m"
#define ZF_ANSI_BG_BCYAN ZF_ANSI_ESC "[106m"
#define ZF_ANSI_BG_WHITE ZF_ANSI_ESC "[47m"
#define ZF_ANSI_BG_BWHITE ZF_ANSI_ESC "[107m"

#define ZF_LOG(format, ...) printf(format "\n", ##__VA_ARGS__)
#define ZF_LOG_WARNING(format, ...) fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_YELLOW "Warning: " ZF_ANSI_RESET format "\n", ##__VA_ARGS__)
#define ZF_LOG_ERROR(format, ...) fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_RED "Error: " ZF_ANSI_RESET format "\n", ##__VA_ARGS__)
#define ZF_LOG_ERROR_SPECIAL(prefix, format, ...) fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_BRED prefix " Error: " ZF_ANSI_RESET); \
    fprintf(stderr, format "\n", ##__VA_ARGS__)
#define ZF_LOG_SUCCESS(format, ...) fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_GREEN "Success: " ZF_ANSI_RESET format "\n", ##__VA_ARGS__)

namespace zf {
    constexpr char g_ascii_printable_min = ' ';
    constexpr char g_ascii_printable_max = '~';
    constexpr int g_ascii_printable_range_len = g_ascii_printable_max - g_ascii_printable_min + 1;

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
