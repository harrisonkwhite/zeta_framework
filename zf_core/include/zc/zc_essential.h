#pragma once

#include <climits>
#include <cstdio>

#ifdef _WIN32
    #define ZF_PLATFORM_WINDOWS 1
#else
    #define ZF_PLATFORM_WINDOWS 0
#endif

#ifdef __linux__
    #define ZF_PLATFORM_LINUX 1
#else
    #define ZF_PLATFORM_LINUX 0
#endif

#if defined(__APPLE__) && defined(__MACH__)
    #define ZF_PLATFORM_MACOS 1
#else
    #define ZF_PLATFORM_MACOS 0
#endif

#ifdef NDEBUG
    #define ZF_DEBUG 0
#else
    #define ZF_DEBUG 1
#endif

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

#ifdef ZF_DEBUG
    #define ZF_FAILURE_DUMP() zf::HandleFailureDump(__FUNCTION__, __FILE__, __LINE__)
    #define ZF_FAILURE_DUMP_MSG(msg) zf::HandleFailureDump(__FUNCTION__, __FILE__, __LINE__, msg)

    #define ZF_ASSERT(condition) \
        do { \
            if (!(condition)) { \
                zf::HandleAssertFailure(#condition, __FUNCTION__, __FILE__, __LINE__); \
            } \
        } while(0)

    #define ZF_ASSERT_MSG(condition, msg) \
        do { \
            if (!(condition)) { \
                zf::HandleAssertFailure(__FUNCTION__, #condition, __FILE__, __LINE__, msg); \
            } \
        } while(0)
#else
    #define ZF_ASSERT(condition) static_cast<void>(0)
    #define ZF_ASSERT_MSG(condition, msg) static_cast<void>(0)
#endif

#if defined(__cpp_lib_is_constant_evaluated)
    #define ZF_IS_CONSTEXPR() (std::is_constant_evaluated())
#elif defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    #define ZF_IS_CONSTEXPR() (__builtin_is_constant_evaluated())
#else
    #define ZF_IS_CONSTEXPR() (false)
#endif

#define ZF_SIZE_OF(x) static_cast<zf::t_size>(sizeof(x))
#define ZF_SIZE_IN_BITS(x) (8 * ZF_SIZE_OF(x))

namespace zf {
    void HandleFailureDump(const char* const func, const char* const file, const int line, const char* const msg = nullptr);
    void HandleAssertFailure(const char* const condition, const char* const func, const char* const file, const int line, const char* const msg = nullptr); // @todo: This feels awkward here, might want to move somewhere else? But where?

    static_assert(CHAR_BIT == 8);

    using t_s8 = signed char;
    static_assert(sizeof(t_s8) == 1);

    using t_u8 = unsigned char;
    static_assert(sizeof(t_u8) == 1);

    using t_s16 = signed short;
    static_assert(sizeof(t_s16) == 2);

    using t_u16 = unsigned short;
    static_assert(sizeof(t_u16) == 2);

    using t_s32 = signed int;
    static_assert(sizeof(t_s32) == 4);

    using t_u32 = unsigned int;
    static_assert(sizeof(t_u32) == 4);

    using t_s64 = signed long long;
    static_assert(sizeof(t_s64) == 8);

    using t_u64 = unsigned long long;
    static_assert(sizeof(t_u64) == 8);

    using t_f32 = float;
    static_assert(sizeof(t_f32) == 4);

    using t_f64 = double;
    static_assert(sizeof(t_f64) == 8);

    using t_b8 = bool;
    static_assert(sizeof(t_b8) == 1);

    // Why signed for this?
    // 1. Mixing signed and unsigned can lead to strange overflow bugs that cannot always be caught by warnings. Better to be consistent and have predictability.
    // 2. The signed 64-bit range is more than sufficient for realistic use cases.
    // 3. If you want a value to be 0 or greater, ASSERT that it is!
    using t_size = t_s64;

    template<typename tp_type> struct s_is_integral { static constexpr t_b8 sm_value = false; };
    template<> struct s_is_integral<t_s8> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_u8> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_s16> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_u16> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_s32> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_u32> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_s64> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_u64> { static constexpr t_b8 sm_value = true; };

    template<typename tp_type> struct s_is_signed_integral { static constexpr t_b8 sm_value = false; };
    template<> struct s_is_signed_integral<t_s8> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_signed_integral<t_s16> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_signed_integral<t_s32> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_signed_integral<t_s64> { static constexpr t_b8 sm_value = true; };

    template<typename tp_type> struct s_is_unsigned_integral { static constexpr t_b8 sm_value = false; };
    template<> struct s_is_unsigned_integral<t_u8> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_unsigned_integral<t_u16> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_unsigned_integral<t_u32> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_unsigned_integral<t_u64> { static constexpr t_b8 sm_value = true; };

    template<typename tp_type> struct s_is_floating_point { static constexpr t_b8 sm_value = false; };
    template<> struct s_is_floating_point<t_f32> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_floating_point<t_f64> { static constexpr t_b8 sm_value = true; };

    template<typename tp_type>
    concept co_integral = s_is_integral<tp_type>::sm_value;

    template<typename tp_type>
    concept co_signed_integral = s_is_signed_integral<tp_type>::sm_value;

    template<typename tp_type>
    concept co_unsigned_integral = s_is_unsigned_integral<tp_type>::sm_value;

    template<typename tp_type>
    concept co_floating_point = s_is_floating_point<tp_type>::sm_value;

    template<typename tp_type>
    concept co_numeric = s_is_integral<tp_type>::sm_value || s_is_floating_point<tp_type>::sm_value;

    // If a < b, return a negative result, if a == b, return 0, and if a > b, return a positive result.
    template<typename tp_type>
    using t_comparator = t_s32 (*)(const tp_type& a, const tp_type& b);

    template<typename tp_type>
    t_s32 DefaultComparator(const tp_type& a, const tp_type& b) {
        if (a == b) {
            return 0;
        } else if (a < b) {
            return -1;
        } else {
            return 1;
        }
    }

    constexpr t_size Kilobytes(const t_size x) { return (static_cast<t_size>(1) << 10) * x; }
    constexpr t_size Megabytes(const t_size x) { return (static_cast<t_size>(1) << 20) * x; }
    constexpr t_size Gigabytes(const t_size x) { return (static_cast<t_size>(1) << 30) * x; }
    constexpr t_size Terabytes(const t_size x) { return (static_cast<t_size>(1) << 40) * x; }

    constexpr t_size BitsToBytes(const t_size x) { return (x + 7) / 8; }
    constexpr t_size BytesToBits(const t_size x) { return x * 8; }

    constexpr t_b8 IsAlignmentValid(const t_size n) {
        // Is it a power of 2?
        return n > 0 && (n & (n - 1)) == 0;
    }

    constexpr t_size AlignForward(const t_size n, const t_size alignment) {
        ZF_ASSERT(n >= 0);
        ZF_ASSERT(IsAlignmentValid(alignment));

        return (n + alignment - 1) & ~(alignment - 1);
    }

    template<typename tp_type>
    constexpr void Swap(tp_type& a, tp_type& b) {
        const tp_type temp = a;
        a = b;
        b = temp;
    }

    template<typename tp_type>
    struct c_array {
        tp_type* buf = nullptr;
        t_size len = 0;

        constexpr c_array() = default;
        constexpr c_array(tp_type* const buf, const t_size len) : buf(buf), len(len) {}

        constexpr t_size SizeInBytes() const {
            return ZF_SIZE_OF(tp_type) * len;
        }

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return buf[index];
        }

        constexpr c_array<const tp_type> View() const {
            return {buf, len};
        }

        constexpr operator c_array<const tp_type>() const {
            return View();
        }

        constexpr c_array Slice(const t_size beg, const t_size end) const {
            ZF_ASSERT(beg >= 0 && beg <= len);
            ZF_ASSERT(end >= beg && end <= len);
            return {buf + beg, end - beg};
        }

        void CopyTo(const c_array<tp_type> dest) const {
            ZF_ASSERT(dest.len >= len);

            for (t_size i = 0; i < len; i++) {
                dest[i] = buf[i];
            }
        }
    };

    template<typename tp_type>
    constexpr c_array<const t_u8> ToBytes(const tp_type& obj) {
        return {reinterpret_cast<const t_u8*>(obj), ZF_SIZE_OF(obj)};
    }

    template<typename tp_type>
    constexpr c_array<t_u8> ToBytes(tp_type& obj) {
        return {reinterpret_cast<t_u8*>(obj), ZF_SIZE_OF(obj)};
    }
}
