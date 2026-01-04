#include <zcl.h>

namespace zf {
    static B8 OutputCode(const strs::StrRdonly input_file_path, const strs::StrRdonly output_file_path, const strs::StrRdonly arr_var_subname) {
        s_arena arena = CreateArena();
        ZF_DEFER({ ArenaDestroy(&arena); });

        s_stream input_file_stream;

        if (!FileOpen(input_file_path, ek_file_access_mode_read, &arena, &input_file_stream)) {
            return false;
        }

        ZF_DEFER({ FileClose(&input_file_stream); });

        s_stream output_file_stream;

        if (!FileOpen(output_file_path, ek_file_access_mode_write, &arena, &output_file_stream)) {
            return false;
        }

        ZF_DEFER({ FileClose(&output_file_stream); });

        Print(&output_file_stream, ZF_STR_LITERAL("#include <zcl/zcl_mem.h>\n"));
        Print(&output_file_stream, ZF_STR_LITERAL("\n"));

        Print(&output_file_stream, ZF_STR_LITERAL("namespace zf {\n"));
        PrintFormat(&output_file_stream, ZF_STR_LITERAL("    extern const U8 g_%_raw[] = {"), arr_var_subname);

        U8 byte_read;
        I32 byte_read_cnt = 0;

        while (ReadItem(&input_file_stream, &byte_read)) {
            if (byte_read_cnt > 0) {
                Print(&output_file_stream, ZF_STR_LITERAL(", "));
            }

            PrintFormat(&output_file_stream, ZF_STR_LITERAL("%"), FormatHex(byte_read));

            byte_read_cnt++;
        }

        Print(&output_file_stream, ZF_STR_LITERAL("};\n"));

        PrintFormat(&output_file_stream, ZF_STR_LITERAL("    extern const I32 g_%_len = %;\n"), arr_var_subname, byte_read_cnt);

        Print(&output_file_stream, ZF_STR_LITERAL("}\n"));

        return true;
    }
}

int main(const int arg_cnt, const char *const *const args) {
    if (arg_cnt != 4) {
        zf::s_stream std_err = zf::StdError();
        zf::PrintFormat(&std_err, ZF_STR_LITERAL("Invalid command-line argument count! Usage: zf_bin_to_array <input_file_path> <output_file_path> <array_variable_subname>"));
        return EXIT_FAILURE;
    }

    return zf::OutputCode(zf::strs::convert_cstr(args[1]), zf::strs::convert_cstr(args[2]), zf::strs::convert_cstr(args[3])) ? EXIT_SUCCESS : EXIT_FAILURE;
}
