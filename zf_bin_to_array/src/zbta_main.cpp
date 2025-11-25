#include <cstdio>
#include <cstdlib>
#include <zc.h>

namespace zf {
    t_b8 OutputCode(const s_str_ascii_rdonly input_file_path, const s_str_ascii_rdonly output_file_path, const s_str_ascii_rdonly arr_subname) {
        ZF_ASSERT(IsStrTerminated(arr_subname));

        s_file_stream input_reader;

        if (!input_reader.Open(input_file_path, e_file_access_mode::read)) {
            return false;
        }

        s_file_stream output_writer;

        if (!output_writer.Open(output_file_path, e_file_access_mode::write)) {
            input_reader.Close();
            return false;
        }

        fprintf(output_writer.raw, "#include <zc/mem/mem.h>\n");
        fprintf(output_writer.raw, "\n");
        fprintf(output_writer.raw, "namespace zf {\n");

        fprintf(output_writer.raw, "    extern const t_byte g_%s_raw[] = {", arr_subname.Raw());

        t_s8 read_byte;
        t_size read_cnt = 0;

        while (input_reader.ReadItem(read_byte)) {
            if (read_cnt > 0) {
                fprintf(output_writer.raw, ", ");
            }

            fprintf(output_writer.raw, "0x%02X", read_byte);

            read_cnt++;
        }

        fprintf(output_writer.raw, "};\n");

        fprintf(output_writer.raw, "    extern const t_size g_%s_size = %zu;\n", arr_subname.Raw(), read_cnt);

        fprintf(output_writer.raw, "}\n");

        output_writer.Close();
        input_reader.Close();

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
        zf::StrFromRawTerminated(args[1]),
        zf::StrFromRawTerminated(args[2]),
        zf::StrFromRawTerminated(args[3])
    ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
