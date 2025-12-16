#include <zcl.h>

namespace zf {
    t_b8 OutputCode(const s_str_rdonly input_file_path, const s_str_rdonly output_file_path, const s_str_rdonly arr_subname) {
        s_mem_arena mem_arena = CreateMemArena(Megabytes(4));
        ZF_DEFER({ mem_arena.Release(); });

        s_stream input_file_stream;

        if (!OpenFile(input_file_path, ek_file_access_mode_read, mem_arena, input_file_stream)) {
            return false;
        }

        ZF_DEFER({ CloseFile(input_file_stream); });

        s_stream output_file_stream;

        if (!OpenFile(output_file_path, ek_file_access_mode_write, mem_arena, output_file_stream)) {
            return false;
        }

        ZF_DEFER({ CloseFile(output_file_stream); });

        Print(output_file_stream, s_cstr_literal("#include <zcl/zcl_mem.h>\n"));
        Print(output_file_stream, s_cstr_literal("\n"));
        Print(output_file_stream, s_cstr_literal("namespace zf {\n"));

        PrintFormat(output_file_stream, s_cstr_literal("    extern const t_u8 g_%_raw[] = {"), arr_subname);

        t_u8 byte_read;
        t_b8 is_first_byte = true;

        while (input_file_stream.ReadItem(byte_read)) {
            if (!is_first_byte) {
                Print(output_file_stream, s_cstr_literal(", "));
            }

            PrintFormat(output_file_stream, s_cstr_literal("%"), FormatHex(byte_read));

            is_first_byte = false;
        }

        Print(output_file_stream, s_cstr_literal("};\n"));
        Print(output_file_stream, s_cstr_literal("}\n"));

        return true;
    }
}

int main(const int arg_cnt, const char *const *const args) {
    if (arg_cnt != 4) {
        zf::LogError(zf::s_cstr_literal("Invalid command-line argument count!"));
        return EXIT_FAILURE;
    }

    return zf::OutputCode(zf::ConvertCstr(args[1]), zf::ConvertCstr(args[2]), zf::ConvertCstr(args[3])) ? EXIT_SUCCESS : EXIT_FAILURE;
}
