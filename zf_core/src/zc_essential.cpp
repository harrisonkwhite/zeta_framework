#include <zc/zc_essential.h>

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

#include <cstdlib> // Reinclude necessary since abort() gets overwritten.

namespace zf {
    static void PrintStackTrace() {
        fprintf(stderr, "\nStack Trace:\n");

#ifdef ZF_PLATFORM_WINDOWS
        const HANDLE proc = GetCurrentProcess();
        SymInitialize(proc, nullptr, TRUE);

        constexpr t_s32 stack_len = 32;
        void* stack[stack_len];
        const t_s32 frame_cnt = CaptureStackBackTrace(0, stack_len, stack, nullptr);

        constexpr t_s32 func_name_buf_size = 256;
        char symbol_buf[ZF_SIZE_OF(SYMBOL_INFO) + func_name_buf_size];
        const auto symbol = reinterpret_cast<SYMBOL_INFO*>(symbol_buf);
        symbol->MaxNameLen = func_name_buf_size - 1;
        symbol->SizeOfStruct = ZF_SIZE_OF(SYMBOL_INFO);

        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = ZF_SIZE_OF(IMAGEHLP_LINE64);

        for (t_s32 i = 0; i < frame_cnt; i++) {
            const auto addr = static_cast<DWORD64>(reinterpret_cast<uintptr_t>(stack[i]));

            if (SymFromAddr(proc, addr, 0, symbol)) {
                DWORD displacement;

                if (SymGetLineFromAddr64(proc, addr, &displacement, &line)) {
                    fprintf(stderr, "- %s (%s:%lu)\n", symbol->Name, line.FileName, line.LineNumber);
                } else {
                    fprintf(stderr, "- %s\n", symbol->Name);
                }
            }
        }

        SymCleanup(proc);
#else
        fprintf(stderr, "Stack trace printing not supported on this platform.\n");
#endif
    }

    static void BreakIntoDebuggerOrAbort() {
#ifdef ZF_PLATFORM_WINDOWS
        if (IsDebuggerPresent()) {
            __debugbreak();
        } else {
            abort();
        }
#else
        abort();
#endif
    }

    void HandleFailureDump(const char* const func, const char* const file, const int line, const char* const msg) {
        fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_RED "\n==================== FAILURE ====================\n" ZF_ANSI_RESET);
        fprintf(stderr, "Function: %s\n", func);
        fprintf(stderr, "File:     %s\n", file);
        fprintf(stderr, "Line:     %d\n", line);

        if (msg) {
            fprintf(stderr, "Message:   \"%s\"\n", msg);
        }

        PrintStackTrace();

        fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_RED "=================================================\n\n" ZF_ANSI_RESET);

        BreakIntoDebuggerOrAbort();
    }

    void HandleAssertFailure(const char* const condition, const char* const file, const int line, const char* const func, const char* const msg) {
        fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_RED "\n==================== ASSERTION FAILED ====================\n" ZF_ANSI_RESET);
        fprintf(stderr, "Condition: %s\n", condition);
        fprintf(stderr, "Function:  %s\n", func);
        fprintf(stderr, "File:      %s\n", file);
        fprintf(stderr, "Line:      %d\n", line);

        if (msg) {
            fprintf(stderr, "Message:   \"%s\"\n", msg);
        }

        PrintStackTrace();

        fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_RED "==========================================================\n\n" ZF_ANSI_RESET);

        BreakIntoDebuggerOrAbort();
    }
}
