#include "zc_mem.h"

namespace zf {
    class c_string_view {
    public:
        c_string_view() = default;

        c_string_view(const char* const c_str) {
            m_chrs = c_array<const char>(c_str, strlen(c_str) + 1);
        }

        c_string_view(const char* const c_str, const size_t c_str_len) {
            assert(strnlen(c_str, c_str_len) == c_str_len && "Invalid string length provided!");
            m_chrs = c_array<const char>(c_str, c_str_len + 1);
        }

        c_string_view(const c_array<const char> chrs) {
            const int chrs_str_len = strnlen(chrs.Raw(), chrs.Len());
            assert(chrs_str_len != chrs.Len() && "Unterminated array of characters provided!");
            m_chrs = chrs.Slice(0, chrs_str_len + 1);
        }

        const char* Raw() const {
            return m_chrs.Raw();
        }

        size_t Len() const {
            return m_chrs.IsEmpty() ? 0 : m_chrs.Len() - 1;
        }

        bool IsEmpty() const {
            return m_chrs.Len() <= 1;
        }

    private:
        c_array<const char> m_chrs;
    };

    class c_string {
    public:
        c_string() = default;

        c_string(char* const c_str) {
            assert(c_str);
            m_chrs = c_array<char>(c_str, strlen(c_str) + 1);
        }

        c_string(char* const c_str, const size_t c_str_len) {
            assert(c_str);
            assert(strnlen(c_str, c_str_len) == c_str_len && "Invalid string length provided!");
            m_chrs = c_array<char>(c_str, c_str_len + 1);
        }

        c_string(const c_array<char> chrs) {
            assert(chrs.Len() > 0);
            const int chrs_str_len = strnlen(chrs.Raw(), chrs.Len());
            assert(chrs_str_len != chrs.Len() && "Unterminated array of characters provided!");
            m_chrs = chrs.Slice(0, chrs_str_len + 1);
        }

        char* Raw() const {
            return m_chrs.Raw();
        }

        size_t Len() const {
            return m_chrs.IsEmpty() ? 0 : m_chrs.Len() - 1;
        }

        bool IsEmpty() const {
            return m_chrs.Len() <= 1;
        }

        c_string_view View() const {
            return {m_chrs.Raw(), Len()};
        }

        operator c_string_view() const {
            return View();
        }

    private:
        c_array<char> m_chrs;
    };
}
