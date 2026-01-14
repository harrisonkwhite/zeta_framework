#include <zcl/zcl_basic.h>

#include <cstdio>

#ifdef ZCL_PLATFORM_WINDOWS
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

namespace zcl {
    void detail::try_breaking_into_debugger_if(const t_b8 cond) {
        if (!cond) {
            return;
        }

#ifdef ZCL_DEBUG
    #ifdef ZCL_PLATFORM_WINDOWS
        if (IsDebuggerPresent()) {
            __debugbreak();
            return;
        }
    #endif
#endif
    }

    static void print_stack_trace() {
#ifdef ZCL_PLATFORM_WINDOWS
        constexpr t_i32 k_stack_len = 32;
        void *stack[k_stack_len];
        const t_i32 frame_cnt = CaptureStackBackTrace(0, k_stack_len, stack, nullptr);

        fprintf(stderr, "Stack Trace:\n");

    #ifdef ZCL_DEBUG
        const HANDLE proc = GetCurrentProcess();
        SymInitialize(proc, nullptr, true);

        const t_i32 func_name_buf_size = 256;
        char symbol_buf[ZCL_SIZE_OF(SYMBOL_INFO) + func_name_buf_size];
        const auto symbol = reinterpret_cast<SYMBOL_INFO *>(symbol_buf);
        symbol->MaxNameLen = func_name_buf_size - 1;
        symbol->SizeOfStruct = ZCL_SIZE_OF(SYMBOL_INFO);

        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = ZCL_SIZE_OF(IMAGEHLP_LINE64);

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

    void detail::handle_assert_error(const char *const cond_c_str, const char *const func_name_c_str, const char *const file_name_c_str, const t_i32 line) {
        fprintf(stderr, "==================== ASSERTION ERROR ====================\n");
        fprintf(stderr, "Condition: %s\n", cond_c_str);
        fprintf(stderr, "Function:  %s\n", func_name_c_str);
        fprintf(stderr, "File:      %s\n", file_name_c_str);
        fprintf(stderr, "Line:      %d\n\n", line);

        print_stack_trace();

        fprintf(stderr, "=========================================================\n");

        fflush(stderr);

        try_breaking_into_debugger_if(true);

        abort();
    }

    void detail::handle_fatal_error(const char *const func_name_c_str, const char *const file_name_c_str, const t_i32 line, const char *const cond_c_str) {
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
#endif

        print_stack_trace();

        fprintf(stderr, "=====================================================\n");

        fflush(stderr);

        try_breaking_into_debugger_if(true);

        abort();
    }
}
