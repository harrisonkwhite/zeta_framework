#include <zcl.h>

namespace zf {
    static t_b8 OutputCode(const t_str_rdonly input_file_path, const t_str_rdonly output_file_path, const t_str_rdonly arr_var_subname) {
        mem::t_arena arena = mem::f_arena_create();
        ZF_DEFER({ mem::f_arena_destroy(&arena); });

        io::t_stream input_file_stream;

        if (!io::f_open_file(input_file_path, io::ec_file_access_mode_read, &arena, &input_file_stream)) {
            return false;
        }

        ZF_DEFER({ io::f_close_file(&input_file_stream); });

        io::t_stream output_file_stream;

        if (!io::f_open_file(output_file_path, io::ec_file_access_mode_write, &arena, &output_file_stream)) {
            return false;
        }

        ZF_DEFER({ io::f_close_file(&output_file_stream); });

        io::f_print(&output_file_stream, ZF_STR_LITERAL("#include <zcl/zcl_mem.h>\n"));
        io::f_print(&output_file_stream, ZF_STR_LITERAL("\n"));

        io::f_print(&output_file_stream, ZF_STR_LITERAL("namespace zf {\n"));
        io::f_print_fmt(&output_file_stream, ZF_STR_LITERAL("    extern const t_u8 g_%_raw[] = {"), arr_var_subname);

        t_u8 byte_read;
        t_i32 byte_read_cnt = 0;

        while (io::f_read_item(&input_file_stream, &byte_read)) {
            if (byte_read_cnt > 0) {
                io::f_print(&output_file_stream, ZF_STR_LITERAL(", "));
            }

            io::f_print_fmt(&output_file_stream, ZF_STR_LITERAL("%"), io::f_fmt_hex(byte_read));

            byte_read_cnt++;
        }

        io::f_print(&output_file_stream, ZF_STR_LITERAL("};\n"));

        io::f_print_fmt(&output_file_stream, ZF_STR_LITERAL("    extern const t_i32 g_%_len = %;\n"), arr_var_subname, byte_read_cnt);

        io::f_print(&output_file_stream, ZF_STR_LITERAL("}\n"));

        return true;
    }
}

int main(const int arg_cnt, const char *const *const args) {
    if (arg_cnt != 4) {
        zf::io::t_stream std_err = zf::io::f_get_std_error();
        zf::io::f_print_fmt(&std_err, ZF_STR_LITERAL("Invalid command-line argument count! Usage: zf_bin_to_array <input_file_path> <output_file_path> <array_variable_subname>"));
        return EXIT_FAILURE;
    }

    return zf::OutputCode(zf::f_strs_convert_cstr(args[1]), zf::f_strs_convert_cstr(args[2]), zf::f_strs_convert_cstr(args[3])) ? EXIT_SUCCESS : EXIT_FAILURE;
}
