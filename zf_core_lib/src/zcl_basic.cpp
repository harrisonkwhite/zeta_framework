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
    static void PrintStackTrace() {
        fprintf(stderr, "\nStack Trace:\n");

#ifdef ZF_PLATFORM_WINDOWS
        const HANDLE proc = GetCurrentProcess();
        SymInitialize(proc, nullptr, TRUE);

        constexpr t_i32 stack_len = 32;
        void *stack[stack_len];
        const t_i32 frame_cnt = CaptureStackBackTrace(0, stack_len, stack, nullptr);

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
            }
        }

        SymCleanup(proc);
#else
        fprintf(stderr, "Stack trace printing not yet supported on this platform.\n");
#endif
    }

    // Returns true if successfully managed to break into debugger.
    static t_b8 BreakIntoDebugger() {
#ifdef ZF_PLATFORM_WINDOWS
        if (IsDebuggerPresent()) {
            __debugbreak();
            return true;
        }
#endif

        return false;
    }

    void internal::AssertError(const char *const cond, const char *const func_name, const char *const file_name, const t_i32 line) {
        fprintf(stderr, "==================== ASSERTION ERROR ====================\n");
        fprintf(stderr, "Condition: %s\n", cond);
        fprintf(stderr, "Function:  %s\n", func_name);
        fprintf(stderr, "File:      %s\n", file_name);
        fprintf(stderr, "Line:      %d\n", line);

        PrintStackTrace();

        fprintf(stderr, "=========================================================\n");

        if (!BreakIntoDebugger()) {
            abort();
        }
    }

    void internal::FatalError(const char *const func_name, const char *const file_name, const t_i32 line, const char *const cond) {
        fprintf(stderr, "==================== FATAL ERROR ====================\n");

        if (cond) {
            fprintf(stderr, "Function:  %s\n", func_name);
            fprintf(stderr, "File:      %s\n", file_name);
            fprintf(stderr, "Line:      %d\n", line);
            fprintf(stderr, "Condition: %s\n", cond);
        } else {
            fprintf(stderr, "Function: %s\n", func_name);
            fprintf(stderr, "File:     %s\n", file_name);
            fprintf(stderr, "Line:     %d\n", line);
        }

        PrintStackTrace();

        fprintf(stderr, "=====================================================\n");

        fflush(stderr);

#ifdef ZF_DEBUG
        if (!BreakIntoDebugger()) {
            getchar();
        }
#endif

        abort();
    }

    void ShowErrorBox(const char *const title, const char *const contents) {
#ifdef ZF_PLATFORM_WINDOWS
        MessageBoxA(nullptr, contents, title, MB_OK | MB_ICONERROR | MB_TOPMOST);
#endif
    }
}
