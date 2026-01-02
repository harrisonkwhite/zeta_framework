#include <zcl/zcl_basic.h>

#include <cstdio>

#ifdef ZF_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #include <windows.h>
    #include <dbghelp.h>
#endif

#include <cstdlib>

namespace zf {
    void detail::TryBreakingIntoDebuggerIf(const t_b8 cond) {
        if (!cond) {
            return;
        }

#ifdef ZF_DEBUG
    #ifdef ZF_PLATFORM_WINDOWS
        if (IsDebuggerPresent()) {
            __debugbreak();
            return;
        }
    #endif
#endif
    }

    static void PrintStackTrace() {
#ifdef ZF_PLATFORM_WINDOWS
        constexpr t_i32 stack_len = 32;
        void *stack[stack_len];
        const t_i32 frame_cnt = CaptureStackBackTrace(0, stack_len, stack, nullptr);

        fprintf(stderr, "Stack Trace:\n");

    #ifdef ZF_DEBUG
        const HANDLE proc = GetCurrentProcess();
        SymInitialize(proc, nullptr, TRUE);

        constexpr t_i32 func_name_buf_size = 256;
        char symbol_buf[ZF_SIZE_OF(SYMBOL_INFO) + func_name_buf_size];
        const auto symbol = reinterpret_cast<SYMBOL_INFO *>(symbol_buf);
        symbol->MaxNameLen = func_name_buf_size - 1;
        symbol->SizeOfStruct = ZF_SIZE_OF(SYMBOL_INFO);

        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = ZF_SIZE_OF(IMAGEHLP_LINE64);

        for (t_i32 i = 0; i < frame_cnt; i++) {
            const auto addr = static_cast<DWORD64>(reinterpret_cast<uintptr_t>(stack[i]));

            if (SymFromAddr(proc, addr, 0, symbol)) {
                DWORD displacement;

                if (SymGetLineFromAddr64(proc, addr, &displacement, &line)) {
                    fprintf(stderr, "- %s (%s:%lu)\n", symbol->Name, line.FileName, line.LineNumber);
                } else {
                    fprintf(stderr, "- %s\n", symbol->Name);
                }
            } else {
                fprintf(stderr, "- 0x%p\n", stack[i]);
            }
        }

        SymCleanup(proc);
    #else
        for (t_i32 i = 0; i < frame_cnt; i++) {
            fprintf(stderr, "- 0x%p\n", stack[i]);
        }
    #endif

#else
        fprintf(stderr, "Stack trace printing not yet supported on this platform.\n");
#endif
    }

    void detail::AssertError(const char *const cond_cstr, const char *const func_name_cstr, const char *const file_name_cstr, const t_i32 line) {
        fprintf(stderr, "==================== ASSERTION ERROR ====================\n");
        fprintf(stderr, "Condition: %s\n", cond_cstr);
        fprintf(stderr, "Function:  %s\n", func_name_cstr);
        fprintf(stderr, "File:      %s\n", file_name_cstr);
        fprintf(stderr, "Line:      %d\n\n", line);

        PrintStackTrace();

        fprintf(stderr, "=========================================================\n");

        fflush(stderr);

        TryBreakingIntoDebuggerIf(true);

        abort();
    }

    void detail::FatalError(const char *const func_name_cstr, const char *const file_name_cstr, const t_i32 line, const char *const cond_cstr) {
        fprintf(stderr, "==================== FATAL ERROR ====================\n");

#ifdef ZF_DEBUG
        if (cond_cstr) {
            fprintf(stderr, "Function:  %s\n", func_name_cstr);
            fprintf(stderr, "File:      %s\n", file_name_cstr);
            fprintf(stderr, "Line:      %d\n", line);
            fprintf(stderr, "Condition: %s\n\n", cond_cstr);
        } else {
            fprintf(stderr, "Function: %s\n", func_name_cstr);
            fprintf(stderr, "File:     %s\n", file_name_cstr);
            fprintf(stderr, "Line:     %d\n\n", line);
        }
#endif

        PrintStackTrace();

        fprintf(stderr, "=====================================================\n");

        fflush(stderr);

        TryBreakingIntoDebuggerIf(true);

        abort();
    }
}
