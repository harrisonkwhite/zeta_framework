#include <zcl.h>

namespace zf {
    [[nodiscard]] static t_b8 output_code(const strs::t_str_rdonly input_file_path, const strs::t_str_rdonly output_file_path, const strs::t_str_rdonly arr_var_subname, const strs::t_str_rdonly module_namespace_name) {
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

        if (strs::f_is_empty(module_namespace_name)) {
            io::f_print(&output_file_stream, ZF_STR_LITERAL("namespace zf {\n"));
        } else {
            io::f_print_fmt(&output_file_stream, ZF_STR_LITERAL("namespace zf::% {\n"), module_namespace_name);
        }

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
    if (arg_cnt != 5) {
        zf::io::t_stream std_err = zf::io::f_get_std_error();
        zf::io::f_print_fmt(&std_err, ZF_STR_LITERAL("Invalid command-line argument count!\nUsage: zf_bin_to_array <input_file_path> <output_file_path> <array_variable_subname> <module_namespace_name>\nNote that the given module namespace name can be empty to just use the base namespace.\n"));
        return EXIT_FAILURE;
    }

    return zf::output_code(zf::strs::f_convert_cstr(args[1]), zf::strs::f_convert_cstr(args[2]), zf::strs::f_convert_cstr(args[3]), zf::strs::f_convert_cstr(args[4])) ? EXIT_SUCCESS : EXIT_FAILURE;
}
