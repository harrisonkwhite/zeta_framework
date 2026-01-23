#pragma once

#include <type_traits>
#include <limits>
#include <concepts>
#include <cmath>

namespace zcl {
#ifdef _WIN32
    #define ZCL_PLATFORM_WINDOWS
#endif

#if defined(__APPLE__) && defined(__MACH__)
    #define ZCL_PLATFORM_MACOS
#endif

#ifdef __linux__
    #define ZCL_PLATFORM_LINUX
#endif

#ifndef NDEBUG
    #define ZCL_DEBUG
#endif

#define ZCL_CHECK_CONSTEXPR() std::is_constant_evaluated()

#define ZCL_CONCAT_IMPL(a, b) a##b
#define ZCL_CONCAT(a, b) ZCL_CONCAT_IMPL(a, b)

    namespace internal {
        template <typename tp_type>
        struct t_defer {
            tp_type func;

            t_defer(const tp_type f) : func(f) {}

            ~t_defer() {
                func();
            }
        };
    }

#define ZCL_DEFER(x) const auto ZCL_CONCAT(defer_, ZCL_CONCAT(l, ZCL_CURRENT_LINE)) = zcl::internal::t_defer([&]() x)

    namespace internal {
        constexpr const char *FindBaseOfFilenameCStr(const char *const c_str) {
            int pen = 0;

            while (c_str[pen]) {
                pen++;
            }

            while (pen > 0 && (c_str[pen - 1] != '/' && c_str[pen - 1] != '\\')) {
                pen--;
            }

            return c_str + pen;
        }
    }

#define ZCL_CURRENT_FILE_PATH __FILE__
#define ZCL_CURRENT_FILE_NAME zcl::internal::FindBaseOfFilenameCStr(__FILE__)
#define ZCL_CURRENT_LINE __LINE__
#define ZCL_CURRENT_FUNC __func__

    static_assert(CHAR_BIT == 8);

    using t_b8 = bool;
    static_assert(sizeof(t_b8) == 1);

    using t_u8 = unsigned char;
    static_assert(sizeof(t_u8) == 1);
    constexpr t_u8 k_u8_min = std::numeric_limits<t_u8>::min();
    constexpr t_u8 k_u8_max = std::numeric_limits<t_u8>::max();

    using t_i8 = signed char;
    static_assert(sizeof(t_i8) == 1);
    constexpr t_i8 k_i8_min = std::numeric_limits<t_i8>::min();
    constexpr t_i8 k_i8_max = std::numeric_limits<t_i8>::max();

    using t_u16 = unsigned short;
    static_assert(sizeof(t_u16) == 2);
    constexpr t_u16 k_u16_min = std::numeric_limits<t_u16>::min();
    constexpr t_u16 k_u16_max = std::numeric_limits<t_u16>::max();

    using t_i16 = signed short;
    static_assert(sizeof(t_i16) == 2);
    constexpr t_i16 k_i16_min = std::numeric_limits<t_i16>::min();
    constexpr t_i16 k_i16_max = std::numeric_limits<t_i16>::max();

    using t_u32 = unsigned int;
    static_assert(sizeof(t_u32) == 4);
    constexpr t_u32 k_u32_min = std::numeric_limits<t_u32>::min();
    constexpr t_u32 k_u32_max = std::numeric_limits<t_u32>::max();

    using t_i32 = signed int;
    static_assert(sizeof(t_i32) == 4);
    constexpr t_i32 k_i32_min = std::numeric_limits<t_i32>::min();
    constexpr t_i32 k_i32_max = std::numeric_limits<t_i32>::max();

    using t_u64 = unsigned long long;
    static_assert(sizeof(t_u64) == 8);
    constexpr t_u64 k_u64_min = std::numeric_limits<t_u64>::min();
    constexpr t_u64 k_u64_max = std::numeric_limits<t_u64>::max();

    using t_i64 = signed long long;
    static_assert(sizeof(t_i64) == 8);
    constexpr t_i64 k_i64_min = std::numeric_limits<t_i64>::min();
    constexpr t_i64 k_i64_max = std::numeric_limits<t_i64>::max();

    using t_f32 = float;
    static_assert(sizeof(t_f32) == 4);
    constexpr t_f32 k_f32_min = std::numeric_limits<t_f32>::min();
    constexpr t_f32 k_f32_max = std::numeric_limits<t_f32>::max();
    constexpr t_f32 k_f32_inf_pos = std::numeric_limits<t_f32>::infinity();
    constexpr t_f32 k_f32_inf_neg = -std::numeric_limits<t_f32>::infinity();
    constexpr t_f32 k_f32_nan_quiet = std::numeric_limits<t_f32>::quiet_NaN();
    constexpr t_f32 k_f32_nan_signalling = std::numeric_limits<t_f32>::signaling_NaN();

    using t_f64 = double;
    static_assert(sizeof(t_f64) == 8);
    constexpr t_f64 k_f64_min = std::numeric_limits<t_f64>::min();
    constexpr t_f64 k_f64_max = std::numeric_limits<t_f64>::max();
    constexpr t_f64 k_f64_inf_pos = std::numeric_limits<t_f64>::infinity();
    constexpr t_f64 k_f64_inf_neg = -std::numeric_limits<t_f64>::infinity();
    constexpr t_f64 k_f64_nan_quiet = std::numeric_limits<t_f64>::quiet_NaN();
    constexpr t_f64 k_f64_nan_signalling = std::numeric_limits<t_f64>::signaling_NaN();

    constexpr t_b8 CheckNaN(const t_f32 val) { return isnan(val); }
    constexpr t_b8 CheckNaN(const t_f64 val) { return isnan(val); }

    using t_uintptr = uintptr_t;
    static_assert(sizeof(t_uintptr) == 8);

    template <typename tp_type> using t_without_extent = typename std::remove_extent<tp_type>::type;
    template <typename tp_type> using t_without_const = typename std::remove_const<tp_type>::type;
    template <typename tp_type> using t_without_volatile = typename std::remove_volatile<tp_type>::type;
    template <typename tp_type> using t_without_ref = typename std::remove_reference<tp_type>::type;
    template <typename tp_type> using t_without_cv = typename std::remove_cv<tp_type>::type;
    template <typename tp_type> using t_without_cvref = typename std::remove_cvref<tp_type>::type;

    // "Simple" meaning that it's safe to use with arenas and C-style memory operations.
    template <typename tp_type>
    concept c_simple = std::is_trivially_default_constructible_v<tp_type> && std::is_trivially_destructible_v<tp_type> && std::is_trivially_copyable_v<tp_type> && std::is_standard_layout_v<tp_type>;

    template <typename tp_type>
    concept c_integral_unsigned = std::is_same_v<tp_type, t_u8> || std::is_same_v<tp_type, t_u16> || std::is_same_v<tp_type, t_u32> || std::is_same_v<tp_type, t_u64>;

    template <typename tp_type>
    concept c_integral_signed = std::is_same_v<tp_type, t_i8> || std::is_same_v<tp_type, t_i16> || std::is_same_v<tp_type, t_i32> || std::is_same_v<tp_type, t_i64>;

    template <typename tp_type>
    concept c_integral = c_integral_unsigned<tp_type> || c_integral_signed<tp_type>;

    template <typename tp_type>
    concept c_floating_point = std::is_floating_point_v<tp_type>;

    template <typename tp_type>
    concept c_numeric = c_integral<tp_type> || c_floating_point<tp_type>;

    template <typename tp_type> concept c_ptr = std::is_pointer_v<tp_type>;
    template <typename tp_type> concept c_const = std::is_const_v<tp_type>;
    template <typename tp_type> concept c_union = std::is_union_v<tp_type>;
    template <typename tp_type> concept c_enum = std::is_enum_v<tp_type>;
    template <typename tp_type> concept c_scalar = std::is_scalar_v<tp_type>;

    template <typename tp_type_a, typename tp_type_b>
    concept c_same = std::same_as<tp_type_a, tp_type_b>;

    template <typename tp_type>
    concept c_c_str = c_same<t_without_cvref<tp_type>, char *>
        || c_same<t_without_cvref<tp_type>, const char *>
        || (c_same<t_without_cv<t_without_extent<t_without_ref<tp_type>>>, char> && !c_same<t_without_cvref<tp_type>, char>);

    // Should return true iff a and b are equal.
    template <c_simple tp_type>
    using t_comparator_bin = t_b8 (*)(const tp_type &a, const tp_type &b);

    template <c_simple tp_type>
    constexpr t_comparator_bin<tp_type> k_comparator_bin_default =
        [](const tp_type &a, const tp_type &b) {
            return a == b;
        };

    // Should return a negative result if a < b, 0 if a == b, and a positive result if a > b.
    template <c_simple tp_type>
    using t_comparator_ord = t_i32 (*)(const tp_type &a, const tp_type &b);

    template <c_simple tp_type>
    constexpr t_comparator_ord<tp_type> k_comparator_ord_default =
        [](const tp_type &a, const tp_type &b) {
            if (a == b) {
                return 0;
            } else if (a < b) {
                return -1;
            } else {
                return 1;
            }
        };


    // ============================================================
    // @section: Errors

    // Redirects stderr to an error log file and updates the message of the fatal error popup.
    void ConfigureErrorLogFile();

    namespace internal {
#ifdef ZCL_DEBUG
        void TryBreakingIntoDebugger();

        // Returns true iff the debugger was broken into.
        inline void TryBreakingIntoDebuggerIf(const t_b8 cond) {
            if (cond) {
                TryBreakingIntoDebugger();
            }
        }

        [[noreturn]] void TriggerAssertionError(const char *const cond_c_str, const char *const func_name_c_str, const char *const file_name_c_str, const t_i32 line);

    #define ZCL_DEBUG_BREAK() internal::TryBreakingIntoDebugger()
    #define ZCL_DEBUG_BREAK_IF(cond) internal::TryBreakingIntoDebuggerIf(cond)

    #define ZCL_ASSERT(cond)                                                                                                \
        do {                                                                                                                \
            if (!ZCL_CHECK_CONSTEXPR()) {                                                                                   \
                if (!(cond)) {                                                                                              \
                    zcl::internal::TriggerAssertionError(#cond, ZCL_CURRENT_FUNC, ZCL_CURRENT_FILE_NAME, ZCL_CURRENT_LINE); \
                }                                                                                                           \
            }                                                                                                               \
        } while (0)
#else
    #define ZCL_DEBUG_BREAK() static_cast<void>(0)
    #define ZCL_DEBUG_BREAK_IF(cond) static_cast<void>(0)
    #define ZCL_ASSERT(cond) static_cast<void>(0)
#endif

        [[noreturn]] void TriggerFatalError(const char *const func_name_c_str, const char *const file_name_c_str, const t_i32 line, const char *const cond_c_str = nullptr);

#define ZCL_FATAL() zcl::internal::TriggerFatalError(ZCL_CURRENT_FUNC, ZCL_CURRENT_FILE_NAME, ZCL_CURRENT_LINE)
#define ZCL_UNREACHABLE() ZCL_FATAL()

#define ZCL_REQUIRE(cond)                                                                                           \
    do {                                                                                                            \
        if (!ZCL_CHECK_CONSTEXPR()) {                                                                               \
            if (!(cond)) {                                                                                          \
                zcl::internal::TriggerFatalError(ZCL_CURRENT_FUNC, ZCL_CURRENT_FILE_NAME, ZCL_CURRENT_LINE, #cond); \
            }                                                                                                       \
        }                                                                                                           \
    } while (0)
    }

    // ============================================================


    // ============================================================
    // @section: Helpers

    template <c_simple tp_type>
    constexpr void Swap(tp_type *const a, tp_type *const b) {
        const tp_type temp = *a;
        *a = *b;
        *b = temp;
    }

    template <c_numeric tp_type>
    constexpr tp_type CalcMin(const tp_type a, const tp_type b) {
        return a <= b ? a : b;
    }

    template <c_numeric tp_type>
    constexpr tp_type CalcMax(const tp_type a, const tp_type b) {
        return a >= b ? a : b;
    }

    template <c_numeric tp_type>
    constexpr tp_type CalcAbs(const tp_type n) {
        return n < 0 ? -n : n;
    }

    template <c_numeric tp_type>
    constexpr tp_type Clamp(const tp_type n, const tp_type min, const tp_type max) {
        ZCL_ASSERT(min <= max);

        if (n < min) {
            return min;
        }

        if (n > max) {
            return max;
        }

        return n;
    }

    template <c_numeric tp_type>
    constexpr t_i32 CalcSign(const tp_type n) {
        if (n > 0) {
            return 1;
        } else if (n < 0) {
            return -1;
        }

        return 0;
    }

    template <c_integral tp_type>
    constexpr tp_type Wrap(const tp_type val, const tp_type max_excl) {
        return ((val % max_excl) + max_excl) % max_excl;
    }

    template <c_integral tp_type>
    constexpr tp_type Wrap(const tp_type val, const tp_type min, const tp_type max_excl) {
        return min + Wrap(val - min, max_excl - min);
    }

#define ZCL_SIZE_OF(x) static_cast<zcl::t_i32>(sizeof(x))
#define ZCL_SIZE_IN_BITS(x) (8 * ZCL_SIZE_OF(x))

#define ZCL_ALIGN_OF(x) static_cast<zcl::t_i32>(alignof(x))

    constexpr t_i32 KilobytesToBytes(const t_i32 n) { return (1 << 10) * n; }
    constexpr t_i32 MegabytesToBytes(const t_i32 n) { return (1 << 20) * n; }
    constexpr t_i32 GigabytesToBytes(const t_i32 n) { return (1 << 30) * n; }
    constexpr t_i32 BitsToBytes(const t_i32 n) { return (n + 7) / 8; }
    constexpr t_i32 BytesToBits(const t_i32 n) { return n * 8; }

    // Is n a power of 2?
    constexpr t_b8 AlignmentCheckValid(const t_i32 n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    // Take n up to the next multiple of the alignment.
    constexpr t_i32 AlignForward(const t_i32 n, const t_i32 alignment) {
        ZCL_ASSERT(AlignmentCheckValid(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

    inline void ZeroClear(void *const buf, const t_i32 buf_size) {
        ZCL_ASSERT(buf_size >= 0);
        memset(buf, 0, static_cast<size_t>(buf_size));
    }

    template <c_simple tp_type>
    void ZeroClearItem(tp_type *const item) {
        ZeroClear(item, ZCL_SIZE_OF(tp_type));
    }

    constexpr t_b8 ZeroCheck(void *const buf, const t_i32 buf_size) {
        ZCL_ASSERT(buf_size >= 0);

        const auto buf_bytes = static_cast<t_u8 *>(buf);

        for (t_i32 i = 0; i < buf_size; i++) {
            if (buf_bytes[i]) {
                return false;
            }
        }

        return true;
    }

    template <c_simple tp_type>
    constexpr t_b8 ZeroCheckItem(tp_type *const item) {
        return ZeroCheck(item, ZCL_SIZE_OF(tp_type));
    }

    // ============================================================


    // ============================================================
    // @section: Arrays

    template <typename tp_type>
    concept c_array_elem = c_simple<tp_type> && c_same<tp_type, t_without_cvref<tp_type>>;

    template <c_array_elem tp_elem_type>
    struct t_array_rdonly {
        using t_elem = tp_elem_type;

        const tp_elem_type *raw;
        t_i32 len;

        constexpr const tp_elem_type &operator[](const t_i32 index) const {
#ifndef ZCL_ENABLE_NO_BOUNDS_CHECKING_IN_RELEASE
            ZCL_REQUIRE(index >= 0 && index < len);
#else
            ZCL_ASSERT(index >= 0 && index < len);
#endif
            return raw[index];
        }
    };

    template <c_array_elem tp_elem_type>
    struct t_array_mut {
        using t_elem = tp_elem_type;

        tp_elem_type *raw;
        t_i32 len;

        constexpr tp_elem_type &operator[](const t_i32 index) const {
#ifndef ZCL_ENABLE_NO_BOUNDS_CHECKING_IN_RELEASE
            ZCL_REQUIRE(index >= 0 && index < len);
#else
            ZCL_ASSERT(index >= 0 && index < len);
#endif
            return raw[index];
        }

        constexpr operator t_array_rdonly<tp_elem_type>() const {
            return {raw, len};
        }
    };

    template <c_array_elem tp_elem_type, t_i32 tp_len>
    struct t_static_array {
        using t_elem = tp_elem_type;

        static constexpr t_i32 k_len = tp_len;

        tp_elem_type raw[tp_len];

        constexpr tp_elem_type &operator[](const t_i32 index) {
#ifndef ZCL_ENABLE_NO_BOUNDS_CHECKING_IN_RELEASE
            ZCL_REQUIRE(index >= 0 && index < tp_len);
#else
            ZCL_ASSERT(index >= 0 && index < tp_len);
#endif
            return raw[index];
        }

        constexpr const tp_elem_type &operator[](const t_i32 index) const {
#ifndef ZCL_ENABLE_NO_BOUNDS_CHECKING_IN_RELEASE
            ZCL_REQUIRE(index >= 0 && index < tp_len);
#else
            ZCL_ASSERT(index >= 0 && index < tp_len);
#endif
            return raw[index];
        }

        constexpr operator t_array_mut<tp_elem_type>() {
            return {raw, k_len};
        }

        constexpr operator t_array_rdonly<tp_elem_type>() const {
            return {raw, k_len};
        }
    };

    template <typename tp_type>
    concept c_array_mut = requires { typename tp_type::t_elem; } && c_same<t_without_cvref<tp_type>, t_array_mut<typename tp_type::t_elem>>;

    template <typename tp_type>
    concept c_array_rdonly = requires { typename tp_type::t_elem; } && c_same<t_without_cvref<tp_type>, t_array_rdonly<typename tp_type::t_elem>>;

    template <typename tp_type>
    concept c_array = c_array_mut<tp_type> || c_array_rdonly<tp_type>;

    template <c_array tp_arr_type>
    constexpr t_comparator_bin<tp_arr_type> k_array_comparator_bin =
        [](const tp_arr_type &a, const tp_arr_type &b) {
            if (a.len != b.len) {
                return false;
            }

            for (t_i32 i = 0; i < a.len; i++) {
                if (a[i] != b[i]) {
                    return false;
                }
            }

            return true;
        };

    template <c_array_elem tp_elem_type, t_i32 tp_len>
    constexpr t_array_mut<tp_elem_type> ArrayToNonstatic(t_static_array<tp_elem_type, tp_len> *const arr) {
        return {arr->raw, arr->k_len};
    }

    template <c_array_elem tp_elem_type, t_i32 tp_len>
    constexpr t_array_rdonly<tp_elem_type> ArrayToNonstatic(const t_static_array<tp_elem_type, tp_len> *const arr) {
        return {arr->raw, arr->k_len};
    }

    template <c_array_elem tp_elem_type>
    constexpr t_array_mut<tp_elem_type> ArraySlice(const t_array_mut<tp_elem_type> arr, const t_i32 beg, const t_i32 end) {
        ZCL_ASSERT(beg >= 0 && beg <= arr.len);
        ZCL_ASSERT(end >= beg && end <= arr.len);

        return {arr.raw + beg, end - beg};
    }

    template <c_array_elem tp_elem_type>
    constexpr t_array_rdonly<tp_elem_type> ArraySlice(const t_array_rdonly<tp_elem_type> arr, const t_i32 beg, const t_i32 end) {
        ZCL_ASSERT(beg >= 0 && beg <= arr.len);
        ZCL_ASSERT(end >= beg && end <= arr.len);

        return {arr.raw + beg, end - beg};
    }

    template <c_array_elem tp_elem_type>
    constexpr t_array_mut<tp_elem_type> ArraySliceFrom(const t_array_mut<tp_elem_type> arr, const t_i32 beg) {
        ZCL_ASSERT(beg >= 0 && beg <= arr.len);
        return {arr.raw + beg, arr.len - beg};
    }

    template <c_array_elem tp_elem_type>
    constexpr t_array_rdonly<tp_elem_type> ArraySliceFrom(const t_array_rdonly<tp_elem_type> arr, const t_i32 beg) {
        ZCL_ASSERT(beg >= 0 && beg <= arr.len);
        return {arr.raw + beg, arr.len - beg};
    }

    template <c_array tp_arr_type>
    constexpr t_i32 ArrayGetSizeInBytes(const tp_arr_type arr) {
        return ZCL_SIZE_OF(typename tp_arr_type::t_elem) * arr.len;
    }

    template <c_array tp_src_arr_type, c_array_mut tp_dest_arr_type>
        requires c_same<typename tp_src_arr_type::t_elem, typename tp_dest_arr_type::t_elem>
    constexpr void ArrayCopy(const tp_src_arr_type src, const tp_dest_arr_type dest, const t_b8 allow_truncation = false) {
        if (!allow_truncation) {
            ZCL_ASSERT(dest.len >= src.len);

            for (t_i32 i = 0; i < src.len; i++) {
                dest[i] = src[i];
            }
        } else {
            const auto min_len = CalcMin(src.len, dest.len);

            for (t_i32 i = 0; i < min_len; i++) {
                dest[i] = src[i];
            }
        }
    }

    template <c_array_elem tp_elem_type>
    t_array_mut<t_u8> ArrayToByteArray(const t_array_mut<tp_elem_type> arr) {
        return {reinterpret_cast<t_u8 *>(arr.raw), ArrayGetSizeInBytes(arr)};
    }

    template <c_array_elem tp_elem_type>
    t_array_rdonly<t_u8> ArrayToByteArray(const t_array_rdonly<tp_elem_type> arr) {
        return {reinterpret_cast<const t_u8 *>(arr.raw), ArrayGetSizeInBytes(arr)};
    }

    template <c_simple tp_type>
    t_array_mut<t_u8> ToBytes(tp_type *const val) {
        return {reinterpret_cast<t_u8 *>(val), ZCL_SIZE_OF(*val)};
    }

    template <c_simple tp_type>
    t_array_rdonly<t_u8> ToBytes(const tp_type *const val) {
        return {reinterpret_cast<const t_u8 *>(val), ZCL_SIZE_OF(*val)};
    }

    // ============================================================


    // ============================================================
    // @section: Arenas

    struct t_arena_block {
        void *buf;
        t_i32 buf_size;

        t_arena_block *next;
    };

    enum t_arena_type : t_i32 {
        ek_arena_type_invalid,
        ek_arena_type_block_based, // Owns its memory, which is organised as a linked list of dynamically allocated blocks. New blocks are allocated as needed.
        ek_arena_type_wrapping     // Non-owning and non-reallocating. Useful if you want to leverage a stack-allocated buffer for example.
    };

    struct t_arena {
        t_arena_type type;

        union {
            struct {
                t_arena_block *blocks_head;
                t_arena_block *block_cur;
                t_i32 block_cur_offs;
                t_i32 block_min_size;
            } block_based;

            struct {
                void *buf;
                t_i32 buf_size;
                t_i32 buf_offs;
            } wrapping;
        } type_data;
    };

#ifdef ZCL_DEBUG
    constexpr t_u8 k_arena_poison = 0xCD; // Memory outside the arena's valid "scope" is set to this for easier debugging.
#endif

    // Does not allocate any arena memory (blocks) upfront.
    inline t_arena ArenaCreateBlockBased(const t_i32 block_min_size = MegabytesToBytes(1)) {
        ZCL_ASSERT(block_min_size > 0);

        return {
            .type = ek_arena_type_block_based,
            .type_data = {.block_based = {.block_min_size = block_min_size}},
        };
    }

    inline t_arena ArenaCreateWrapping(const t_array_mut<t_u8> bytes) {
        ZeroClear(bytes.raw, bytes.len);

        return {
            .type = ek_arena_type_wrapping,
            .type_data = {.wrapping = {.buf = bytes.raw, .buf_size = bytes.len}},
        };
    }

    // Frees all arena memory. Only valid for block-based arenas. This can be called even if no pushing was done.
    void ArenaDestroy(t_arena *const arena);

    // Will lazily allocate memory as needed. Allocation failure is treated as fatal and causes an abort - you don't need to check for nullptr.
    // The returned buffer is guaranteed to be zeroed.
    void *ArenaPushRaw(t_arena *const arena, const t_i32 size, const t_i32 alignment);

    // Will lazily allocate memory as needed. Allocation failure is treated as fatal and causes an abort - you don't need to check for nullptr.
    // The returned item is guaranteed to be zeroed.
    template <c_simple tp_type>
    tp_type *ArenaPush(t_arena *const arena) {
        return static_cast<tp_type *>(ArenaPushRaw(arena, ZCL_SIZE_OF(tp_type), ZCL_ALIGN_OF(tp_type)));
    }

    template <c_array_elem tp_elem_type>
    t_array_mut<tp_elem_type> ArenaPushArray(t_arena *const arena, const t_i32 len) {
        ZCL_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const t_i32 size = ZCL_SIZE_OF(tp_elem_type) * len;
        return {static_cast<tp_elem_type *>(ArenaPushRaw(arena, size, ZCL_ALIGN_OF(tp_elem_type))), len};
    }

    template <c_array tp_arr_type>
    auto ArenaPushArrayClone(t_arena *const arena, const tp_arr_type arr_to_clone) {
        const auto arr = ArenaPushArray<typename tp_arr_type::t_elem>(arena, arr_to_clone.len);
        ArrayCopy(arr, arr_to_clone);
        return arr;
    }

    // Takes the arena offset to the beginning of its memory (if any) to overwrite from there.
    void ArenaRewind(t_arena *const arena);

    // ============================================================
}
