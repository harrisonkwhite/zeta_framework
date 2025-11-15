#include <zf/zf_debug.h>

#include <cstdio>

namespace zf {
    void ConfigErrorOutput() {
#ifndef ZF_DEBUG
        // Redirect stderr to crash log file.
        freopen("error.log", "w", stderr);
#endif
    }
}
