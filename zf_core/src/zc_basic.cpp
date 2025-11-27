#include <zc/zc_basic.h>

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

    void HandleAssertFailure(const char* const cond_raw, const char* const func_name_raw, const char* const file_name_raw, const t_s32 line, const char* const msg_raw) {
        fprintf(stderr, "==================== ASSERTION FAILED ====================\n");
        fprintf(stderr, "Condition: %s\n", cond_raw);
        fprintf(stderr, "Function:  %s\n", func_name_raw);
        fprintf(stderr, "File:      %s\n", file_name_raw);
        fprintf(stderr, "Line:      %d\n", line);

        if (msg_raw) {
            fprintf(stderr, "Message:   \"%s\"\n", msg_raw);
        }

        PrintStackTrace();

        fprintf(stderr, "==========================================================\n");

        if (!BreakIntoDebugger()) {
            abort();
        }
    }

    void ReportFailure(const char* const func_name_raw, const char* const file_name_raw, const t_s32 line, const char* const msg_raw) {
        fprintf(stderr, "==================== FAILURE ====================\n");

        fprintf(stderr, "Function: %s\n", func_name_raw);
        fprintf(stderr, "File:     %s\n", file_name_raw);
        fprintf(stderr, "Line:     %d\n", line);

        if (msg_raw) {
            fprintf(stderr, "Message:  \"%s\"\n", msg_raw);
        }

        PrintStackTrace();

        fprintf(stderr, "=================================================\n");

#ifdef ZF_DEBUG
        BreakIntoDebugger();
#endif
    }

    void ShowErrorBox(const char* const title_raw, const char* const contents_raw) {
#ifdef ZF_PLATFORM_WINDOWS
        MessageBoxA(nullptr, contents_raw, title_raw, MB_OK | MB_ICONERROR | MB_TOPMOST);
#endif
    }
}
