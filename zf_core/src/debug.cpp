#include <zc/debug.h>

namespace zf {
    void HandleAssertFailure(const char* const condition, const char* const file, const int line, const char* const func) {
        fprintf(stderr, "\n==================== ASSERTION FAILED ====================\n");
        fprintf(stderr, "Condition: %s\n", condition);
        fprintf(stderr, "Function:  %s\n", func);
        fprintf(stderr, "File:      %s\n", file);
        fprintf(stderr, "Line:      %d\n", line);

        // @todo: Print stack trace!

        fprintf(stderr, "==========================================================\n\n");

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
