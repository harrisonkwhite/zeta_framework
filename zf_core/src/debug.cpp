#include <zc/debug.h>

#ifdef _WIN32
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
#ifdef _WIN32
        const HANDLE proc = GetCurrentProcess();
        SymInitialize(proc, nullptr, TRUE);

        constexpr int stack_len = 32;
        void* stack[stack_len];
        const int frame_cnt = CaptureStackBackTrace(0, stack_len, stack, nullptr);

        constexpr int func_name_buf_size = 256;
        char symbol_buf[sizeof(SYMBOL_INFO) + func_name_buf_size];
        const auto symbol = reinterpret_cast<SYMBOL_INFO*>(symbol_buf);
        symbol->MaxNameLen = func_name_buf_size - 1;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        for (int i = 0; i < frame_cnt; i++) {
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

    void HandleAssertFailure(const char* const condition, const char* const file, const int line, const char* const func, const char* const msg) {
        fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_RED "\n==================== ASSERTION FAILED ====================\n" ZF_ANSI_RESET);
        fprintf(stderr, "Condition: %s\n", condition);
        fprintf(stderr, "Function:  %s\n", func);
        fprintf(stderr, "File:      %s\n", file);
        fprintf(stderr, "Line:      %d\n", line);

        if (msg) {
            fprintf(stderr, "Message:   \"%s\"\n", msg);
        }

        fprintf(stderr, "\nStack Trace:\n");
        PrintStackTrace();

        fprintf(stderr, ZF_ANSI_BOLD ZF_ANSI_FG_RED "==========================================================\n\n" ZF_ANSI_RESET);

        // Break into debugger if attached, otherwise abort.
#ifdef _WIN32
        if (IsDebuggerPresent()) {
            __debugbreak();
        } else {
            abort();
        }
#else
        abort();
#endif
    }
}
