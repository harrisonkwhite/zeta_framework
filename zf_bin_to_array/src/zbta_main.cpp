#include <zcl.h>

namespace zf {
    static t_b8 OutputCode(const t_str_rdonly input_file_path, const t_str_rdonly output_file_path, const t_str_rdonly arr_var_subname) {
        mem::t_arena arena = mem::f_arena_create();
        ZF_DEFER({ mem::f_arena_destroy(&arena); });

        t_stream input_file_stream;

        if (!f_io_open_file(input_file_path, ec_file_access_mode_read, &arena, &input_file_stream)) {
            return false;
        }

        ZF_DEFER({ f_io_close_file(&input_file_stream); });

        t_stream output_file_stream;

        if (!f_io_open_file(output_file_path, ec_file_access_mode_write, &arena, &output_file_stream)) {
            return false;
        }

        ZF_DEFER({ f_io_close_file(&output_file_stream); });

        f_io_print(&output_file_stream, ZF_STR_LITERAL("#include <zcl/zcl_mem.h>\n"));
        f_io_print(&output_file_stream, ZF_STR_LITERAL("\n"));

        f_io_print(&output_file_stream, ZF_STR_LITERAL("namespace zf {\n"));
        f_io_print_fmt(&output_file_stream, ZF_STR_LITERAL("    extern const t_u8 g_%_raw[] = {"), arr_var_subname);

        t_u8 byte_read;
        t_i32 byte_read_cnt = 0;

        while (f_io_read_item(&input_file_stream, &byte_read)) {
            if (byte_read_cnt > 0) {
                f_io_print(&output_file_stream, ZF_STR_LITERAL(", "));
            }

            f_io_print_fmt(&output_file_stream, ZF_STR_LITERAL("%"), f_io_fmt_hex(byte_read));

            byte_read_cnt++;
        }

        f_io_print(&output_file_stream, ZF_STR_LITERAL("};\n"));

        f_io_print_fmt(&output_file_stream, ZF_STR_LITERAL("    extern const t_i32 g_%_len = %;\n"), arr_var_subname, byte_read_cnt);

        f_io_print(&output_file_stream, ZF_STR_LITERAL("}\n"));

        return true;
    }
}

int main(const int arg_cnt, const char *const *const args) {
    if (arg_cnt != 4) {
        zf::t_stream std_err = zf::f_io_get_std_error();
        zf::f_io_print_fmt(&std_err, ZF_STR_LITERAL("Invalid command-line argument count! Usage: zf_bin_to_array <input_file_path> <output_file_path> <array_variable_subname>"));
        return EXIT_FAILURE;
    }

    return zf::OutputCode(zf::f_strs_convert_cstr(args[1]), zf::f_strs_convert_cstr(args[2]), zf::f_strs_convert_cstr(args[3])) ? EXIT_SUCCESS : EXIT_FAILURE;
}
