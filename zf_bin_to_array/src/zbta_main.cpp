#include <zcl.h>

namespace zf {
    static t_b8 OutputCode(const s_str_rdonly input_file_path, const s_str_rdonly output_file_path, const s_str_rdonly arr_subname) {
        s_arena arena = ArenaCreate();
        ZF_DEFER({ ArenaDestroy(&arena); });

        c_stream input_file_stream;

        if (!FileOpen(input_file_path, ek_file_access_mode_read, &arena, &input_file_stream)) {
            return false;
        }

        ZF_DEFER({ FileClose(&input_file_stream); });

        c_stream output_file_stream;

        if (!FileOpen(output_file_path, ek_file_access_mode_write, &arena, &output_file_stream)) {
            return false;
        }

        ZF_DEFER({ FileClose(&output_file_stream); });

        Print(&output_file_stream, s_cstr_literal{"#include <zcl/zcl_mem.h>\n"});
        Print(&output_file_stream, s_cstr_literal{"\n"});
        Print(&output_file_stream, s_cstr_literal{"namespace zf {\n"});

        PrintFormat(&output_file_stream, s_cstr_literal{"    extern const t_u8 g_%_raw[] = {"}, arr_subname);

        t_u8 byte_read;
        t_i32 byte_read_cnt = 0;

        while (input_file_stream.ReadItem(&byte_read)) {
            if (byte_read_cnt > 0) {
                Print(&output_file_stream, s_cstr_literal{", "});
            }

            PrintFormat(&output_file_stream, s_cstr_literal{"%"}, FormatHex(byte_read));

            byte_read_cnt++;
        }

        Print(&output_file_stream, s_cstr_literal{"};\n"});

        PrintFormat(&output_file_stream, s_cstr_literal{"    extern const t_i32 g_%_len = %;\n"}, arr_subname, byte_read_cnt);

        Print(&output_file_stream, s_cstr_literal{"}\n"});

        return true;
    }
}

int main(const int arg_cnt, const char *const *const args) {
    if (arg_cnt != 4) {
        zf::LogError(zf::s_cstr_literal{"Invalid command-line argument count!"});
        return EXIT_FAILURE;
    }

    return zf::OutputCode(zf::CstrToStr(args[1]), zf::CstrToStr(args[2]), zf::CstrToStr(args[3])) ? EXIT_SUCCESS : EXIT_FAILURE;
}
