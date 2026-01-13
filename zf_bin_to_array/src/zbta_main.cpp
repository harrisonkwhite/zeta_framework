#include <zcl.h>

[[nodiscard]] static zcl::t_b8 output_code(const zcl::t_str_rdonly input_file_path, const zcl::t_str_rdonly output_file_path, const zcl::t_str_rdonly arr_var_subname, const zcl::t_str_rdonly namespace_name) {
    zcl::t_arena arena = zcl::arena_create_blockbased();
    ZF_DEFER({ zcl::arena_destroy(&arena); });

    zcl::t_file_stream input_file_stream;

    if (!zcl::file_open(input_file_path, zcl::ek_file_access_mode_read, &arena, &input_file_stream)) {
        return false;
    }

    ZF_DEFER({ zcl::file_close(&input_file_stream); });

    zcl::t_file_stream output_file_stream;

    if (!zcl::file_open(output_file_path, zcl::ek_file_access_mode_write, &arena, &output_file_stream)) {
        return false;
    }

    ZF_DEFER({ zcl::file_close(&output_file_stream); });

    zcl::print(output_file_stream, ZCL_STR_LITERAL("#include <zcl/zcl_basic.h>\n"));
    zcl::print(output_file_stream, ZCL_STR_LITERAL("\n"));

    zcl::t_str_rdonly indent = {};

    if (!zcl::str_check_empty(namespace_name)) {
        zcl::print_format(output_file_stream, ZCL_STR_LITERAL("namespace % {\n"), namespace_name);
        indent = ZCL_STR_LITERAL("    ");
    }

    zcl::print_format(output_file_stream, ZCL_STR_LITERAL("%extern const zcl::t_u8 g_%_raw[] = {"), indent, arr_var_subname);

    zcl::t_u8 byte_read;
    zcl::t_i32 byte_read_cnt = 0;

    while (zcl::stream_read_item(input_file_stream, &byte_read)) {
        if (byte_read_cnt > 0) {
            zcl::print(output_file_stream, ZCL_STR_LITERAL(", "));
        }

        zcl::print_format(output_file_stream, ZCL_STR_LITERAL("%"), zcl::format_hex(byte_read));

        byte_read_cnt++;
    }

    zcl::print(output_file_stream, ZCL_STR_LITERAL("};\n"));

    zcl::print_format(output_file_stream, ZCL_STR_LITERAL("%extern const zcl::t_i32 g_%_len = %;\n"), indent, arr_var_subname, byte_read_cnt);

    if (!zcl::str_check_empty(namespace_name)) {
        zcl::print(output_file_stream, ZCL_STR_LITERAL("}\n"));
    }

    return true;
}

int main(const int arg_cnt, const char *const *const args) {
    if (arg_cnt != 5) {
        zcl::t_file_stream std_err = zcl::file_stream_create_std_error();
        zcl::print_format(std_err, ZCL_STR_LITERAL("Invalid command-line argument count!\nUsage: zf_bin_to_array <input_file_path> <output_file_path> <array_variable_subname> <namespace>\nNote that the given namespace can be empty for no namespace.\n"));
        return EXIT_FAILURE;
    }

    return output_code(zcl::cstr_to_str(args[1]), zcl::cstr_to_str(args[2]), zcl::cstr_to_str(args[3]), zcl::cstr_to_str(args[4])) ? EXIT_SUCCESS : EXIT_FAILURE;
}
