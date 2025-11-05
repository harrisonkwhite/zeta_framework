#include <cstdio>
#include <cstdlib>
#include <zc/io.h>

namespace zf {
    bool OutputCode(const s_str_view input_file_path, const s_str_view output_file_path, const s_str_view arr_subname) {
        assert(arr_subname.IsTerminated());

        c_file_reader input_reader;
        input_reader.DeferClose();

        if (!input_reader.Open(input_file_path)) {
            return false;
        }

        c_file_writer output_writer;
        output_writer.DeferClose();

        if (!output_writer.Open(output_file_path)) {
            return false;
        }

        fprintf(output_writer.Raw(), "#include <zc/mem/mem.h>\n");
        fprintf(output_writer.Raw(), "\n");
        fprintf(output_writer.Raw(), "namespace zf {\n");

        fprintf(output_writer.Raw(), "    extern const t_u8 g_%s_raw[] = {", arr_subname.Raw());

        t_u8 read_byte;
        size_t read_cnt = 0;

        while (input_reader.ReadItem(read_byte)) {
            if (read_cnt > 0) {
                fprintf(output_writer.Raw(), ", ");
            }

            fprintf(output_writer.Raw(), "0x%02X", read_byte);

            read_cnt++;
        }

        fprintf(output_writer.Raw(), "};\n");

        fprintf(output_writer.Raw(), "    extern const size_t g_%s_size = %zu;\n", arr_subname.Raw(), read_cnt);

        fprintf(output_writer.Raw(), "}\n");

        return true;
    }
}

int main(const int arg_cnt, const char* const* args) {
    if (arg_cnt != 4) {
        ZF_LOG_ERROR("Invalid command-line argument count!");
        ZF_LOG_ERROR("Usage: zf_bin_to_array <input_file_path> <output_file_path> <arr_subname>");
        return EXIT_FAILURE;
    }

    return zf::OutputCode(
        zf::s_str_view::FromRawTerminated(args[1]),
        zf::s_str_view::FromRawTerminated(args[2]),
        zf::s_str_view::FromRawTerminated(args[3])
    ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
