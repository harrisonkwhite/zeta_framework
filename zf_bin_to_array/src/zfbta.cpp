#include <cstdio>
#include <cstdlib>
#include <zc.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace zf {
    void OutputCode(const c_string_view arr_subname) {
        printf("#include <zc.h>\n");
        printf("\n");
        printf("namespace zf {\n");

        printf("    const t_u8 g_%s[] = {", arr_subname.Raw());

        size_t size = 0;
        int c;

        while ((c = getchar()) != EOF) {
            const auto byte = static_cast<t_u8>(c);

            if (size > 0) {
                printf(", ");
            }

            printf("0x%02X", byte);

            size++;
        }

        printf("};\n");

        printf("    const size_t g_%s_size = %zu;\n", arr_subname.Raw(), size);
        printf("};\n");
    }
}

int main(const int arg_cnt, const char* const* args) {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    if (arg_cnt != 2) {
        ZF_LOG_ERROR("Invalid command-line argument count! Expected an array variable subname to be provided.");
        return EXIT_FAILURE;
    }

    zf::OutputCode(args[1]);

    return EXIT_SUCCESS;
}
