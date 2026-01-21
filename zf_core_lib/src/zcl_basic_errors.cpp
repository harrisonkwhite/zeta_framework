#include <zcl/zcl_basic.h>

#ifdef ZCL_PLATFORM_WINDOWS
    #include <windows.h>
    #include <dbghelp.h>
#endif

namespace zcl {
    static void ErrorBoxShow(const char *const msg_c_str) {
#if defined(ZCL_PLATFORM_WINDOWS)
        MessageBoxA(nullptr, msg_c_str, "Fatal Error", MB_OK | MB_ICONERROR);
#elif defined(ZCL_PLATFORM_MACOS)
        static_assert(false); // @todo
#elif defined(ZCL_PLATFORM_LINUX)
        static_assert(false); // @todo
#else
        static_assert(false, "Platform not supported!");
#endif
    }

    void (*g_error_box_show_func)() = []() {
        ErrorBoxShow("A fatal error occurred. The program will now exit.");
    };

    void ConfigureErrorLogFile() {
        constexpr const char *k_error_log_file_name_c_str = "error.log";

        FILE *const file = freopen(k_error_log_file_name_c_str, "w", stderr);

        if (!file) {
            ZCL_FATAL();
        }

        g_error_box_show_func = []() {
            char msg_buf_raw[256];
            snprintf(msg_buf_raw, sizeof(msg_buf_raw), "A fatal error occurred - please check \"%s\" for details.\nThe program will now exit.", k_error_log_file_name_c_str);

            ErrorBoxShow(msg_buf_raw);
        };
    }

#ifdef ZCL_DEBUG
    void internal::TryBreakingIntoDebugger() {
    #if defined(ZCL_PLATFORM_WINDOWS)
        if (IsDebuggerPresent()) {
            __debugbreak();
        }
    #elif defined(ZCL_PLATFORM_MACOS)
        static_assert(false); // @todo
    #elif defined(ZCL_PLATFORM_LINUX)
        static_assert(false); // @todo
    #else
        static_assert(false, "Platform not supported!");
    #endif
    }
#endif

#ifdef ZCL_DEBUG
    static void PrintStackTrace() {
    #if defined(ZCL_PLATFORM_WINDOWS)
        constexpr t_i32 k_stack_len = 32;
        void *stack_raw[k_stack_len];
        const t_i32 frame_cnt = CaptureStackBackTrace(0, k_stack_len, stack_raw, nullptr);

        fprintf(stderr, "Stack Trace:\n");

        const HANDLE proc = GetCurrentProcess();
        SymInitialize(proc, nullptr, true);

        const t_i32 func_name_buf_size = 256;
        char symbol_buf_raw[ZCL_SIZE_OF(SYMBOL_INFO) + func_name_buf_size];
        const auto symbol = reinterpret_cast<SYMBOL_INFO *>(symbol_buf_raw);
        symbol->MaxNameLen = func_name_buf_size - 1;
        symbol->SizeOfStruct = ZCL_SIZE_OF(SYMBOL_INFO);

        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = ZCL_SIZE_OF(IMAGEHLP_LINE64);

        for (t_i32 i = 0; i < frame_cnt; i++) {
            const auto addr = static_cast<DWORD64>(reinterpret_cast<uintptr_t>(stack_raw[i]));

            if (SymFromAddr(proc, addr, 0, symbol)) {
                DWORD displacement;

                if (SymGetLineFromAddr64(proc, addr, &displacement, &line)) {
                    fprintf(stderr, "- %s (%s:%lu)\n", symbol->Name, line.FileName, line.LineNumber);
                } else {
                    fprintf(stderr, "- %s\n", symbol->Name);
                }
            } else {
                fprintf(stderr, "- 0x%p\n", stack_raw[i]);
            }
        }

        SymCleanup(proc);
    #elif defined(ZCL_PLATFORM_MACOS)
        static_assert(false); // @todo
    #elif defined(ZCL_PLATFORM_LINUX)
        static_assert(false); // @todo
    #else
        static_assert(false, "Platform not supported!");
    #endif
    }
#endif

    [[noreturn]] static void Terminate() {
#if defined(ZCL_PLATFORM_WINDOWS)
        TerminateProcess(GetCurrentProcess(), 0);
        __assume(false);
#elif defined(ZF_PLATFORM_MACOS) || defined(ZF_PLATFORM_LINUX)
        _Exit(0);
#else
        static_assert(false, "Platform not supported!");
#endif
    }

#ifdef ZCL_DEBUG
    void internal::TriggerAssertionError(const char *const cond_c_str, const char *const func_name_c_str, const char *const file_name_c_str, const t_i32 line) {
        fprintf(stderr, "==================== ASSERTION ERROR ====================\n");

        fprintf(stderr, "Function:  %s\n", func_name_c_str);
        fprintf(stderr, "File:      %s\n", file_name_c_str);
        fprintf(stderr, "Line:      %d\n", line);
        fprintf(stderr, "Condition: %s\n\n", cond_c_str);

        PrintStackTrace();

        fprintf(stderr, "=========================================================\n");

        fflush(stderr);

        TryBreakingIntoDebugger();

        g_error_box_show_func();

        Terminate();
    }
#endif

    void internal::TriggerFatalError(const char *const func_name_c_str, const char *const file_name_c_str, const t_i32 line, const char *const cond_c_str) {
        fprintf(stderr, "==================== FATAL ERROR ====================\n");

#ifdef ZCL_DEBUG
        if (cond_c_str) {
            fprintf(stderr, "Function:  %s\n", func_name_c_str);
            fprintf(stderr, "File:      %s\n", file_name_c_str);
            fprintf(stderr, "Line:      %d\n", line);
            fprintf(stderr, "Condition: %s\n\n", cond_c_str);
        } else {
            fprintf(stderr, "Function: %s\n", func_name_c_str);
            fprintf(stderr, "File:     %s\n", file_name_c_str);
            fprintf(stderr, "Line:     %d\n\n", line);
        }

        PrintStackTrace();

        fprintf(stderr, "\n");
#else
        if (cond_c_str) {
            fprintf(stderr, "Function:  %s\n", func_name_c_str);
            fprintf(stderr, "Line:      %d\n", line);
            fprintf(stderr, "Condition: %s\n\n", cond_c_str);
        } else {
            fprintf(stderr, "Function: %s\n", func_name_c_str);
            fprintf(stderr, "Line:     %d\n\n", line);
        }
#endif

        fprintf(stderr, "=====================================================\n");

        fflush(stderr);

#ifdef ZCL_DEBUG
        TryBreakingIntoDebugger();
#endif

        g_error_box_show_func();

        Terminate();
    }
}
