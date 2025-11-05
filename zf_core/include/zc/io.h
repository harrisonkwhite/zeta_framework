#pragma once

#include <cstdio>
#include <zc/mem/strs.h>

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

    struct s_file_stream {
        FILE* raw = nullptr;

        [[nodiscard]]
        bool Open(const s_str_view file_path, const bool is_write) {
            assert(file_path.IsTerminated());
            raw = fopen(file_path.Raw(), is_write ? "wb" : "rb");
            return raw;
        }

        void Close() {
            fclose(raw);
            raw = nullptr;
        }

        size_t CalcSize() {
            const auto pos_old = ftell(raw);
            fseek(raw, 0, SEEK_END);
            const size_t file_size = ftell(raw);
            fseek(raw, pos_old, SEEK_SET);
            return file_size;
        }

        template<typename tp_type>
        [[nodiscard]]
        bool ReadItem(tp_type& item) {
            return fread(&item, sizeof(tp_type), 1, raw) == 1;
        }

        template<typename tp_type>
        [[nodiscard]]
        int ReadItems(const c_array<tp_type> arr) {
            return fread(arr.Raw(), sizeof(tp_type), arr.Len(), raw);
        }

        template<typename tp_type>
        [[nodiscard]]
        bool WriteItem(const tp_type& item) {
            return fwrite(&item, sizeof(tp_type), 1, raw) == 1;
        }

        template<typename tp_type>
        [[nodiscard]]
        int WriteItems(const c_array<const tp_type> arr) {
            return fwrite(arr.Raw(), sizeof(tp_type), arr.Len(), raw);
        }
    };

    bool LoadFileContents(c_array<t_u8>& contents, c_mem_arena& mem_arena, const s_str_view file_path, const bool include_terminating_byte = false);

    inline bool LoadFileContentsAsStr(s_str& contents, c_mem_arena& mem_arena, const s_str_view file_path) {
        assert(contents.chrs.IsEmpty());

        c_array<t_u8> contents_default;

        if (!LoadFileContents(contents_default, mem_arena, file_path, true)) {
            return false;
        }

        contents = s_str::FromRawTerminated(reinterpret_cast<char*>(contents_default.Raw()), contents_default.Len() - 1);

        return true;
    }
}
