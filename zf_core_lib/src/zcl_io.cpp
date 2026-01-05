#include <zcl/zcl_io.h>

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef ZF_PLATFORM_WINDOWS
    #include <windows.h>
    #include <direct.h>
#endif

namespace zf {
    t_b8 f_io_open_file(const t_str_rdonly path, const t_io_file_access_mode mode, mem::t_arena *const temp_arena, t_io_stream *const o_stream) {
        const t_str_rdonly path_terminated = f_strs_clone_but_add_terminator(path, temp_arena);

        FILE *file;
        t_io_stream_mode stream_mode;

        switch (mode) {
        case ec_io_file_access_mode_read:
            file = fopen(f_strs_get_as_cstr(path_terminated), "rb");
            stream_mode = ec_stream_mode_read;
            break;

        case ec_io_file_access_mode_write:
            file = fopen(f_strs_get_as_cstr(path_terminated), "wb");
            stream_mode = ec_stream_mode_write;
            break;

        case ec_io_file_access_mode_append:
            file = fopen(f_strs_get_as_cstr(path_terminated), "ab");
            stream_mode = ec_stream_mode_write;
            break;

        default:
            ZF_UNREACHABLE();
        }

        if (!file) {
            return false;
        }

        *o_stream = f_io_create_file_stream(file, stream_mode);

        return true;
    }

    void f_io_close_file(t_io_stream *const stream) {
        fclose(stream->type_data.file.file);
        *stream = {};
    }

    t_i32 f_io_calc_file_size(t_io_stream *const stream) {
        FILE *const file = stream->type_data.file.file;
        const auto pos_old = ftell(file);
        fseek(file, 0, SEEK_END);
        const auto file_size = ftell(file);
        fseek(file, pos_old, SEEK_SET);
        return static_cast<t_i32>(file_size);
    }

    t_b8 f_io_load_file_contents(const t_str_rdonly path, mem::t_arena *const contents_arena, mem::t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator) {
        t_io_stream stream;

        if (!f_io_open_file(path, ec_io_file_access_mode_read, temp_arena, &stream)) {
            return false;
        }

        ZF_DEFER({ f_io_close_file(&stream); });

        const t_i32 file_size = f_io_calc_file_size(&stream);

        if (add_terminator) {
            *o_contents = mem::f_arena_push_array<t_u8>(contents_arena, file_size + 1);
            (*o_contents)[file_size] = 0;
        } else {
            *o_contents = mem::f_arena_push_array<t_u8>(contents_arena, file_size);
        }

        if (!f_io_read_items_into_array(&stream, *o_contents, file_size)) {
            return false;
        }

        return true;
    }

    t_b8 f_io_create_directory(const t_str_rdonly path, mem::t_arena *const temp_arena, t_io_directory_creation_result *const o_creation_res) {
        if (o_creation_res) {
            *o_creation_res = ec_io_directory_creation_result_success;
        }

        const t_str_rdonly path_terminated = f_strs_clone_but_add_terminator(path, temp_arena);

#ifdef ZF_PLATFORM_WINDOWS
        const t_i32 result = _mkdir(f_strs_get_as_cstr(path_terminated));
#else
        const t_s32 result = mkdir(AsCstr(path_terminated), 0755);
#endif

        if (result == 0) {
            return true;
        }

        if (o_creation_res) {
            switch (errno) {
            case EEXIST:
                *o_creation_res = ec_io_directory_creation_result_already_exists;
                break;

            case EACCES:
            case EPERM:
                *o_creation_res = ec_io_directory_creation_result_permission_denied;
                break;

            case ENOENT:
                *o_creation_res = ec_io_directory_creation_result_path_not_found;
                break;

            default:
                *o_creation_res = ec_io_directory_creation_result_unknown_err;
                break;
            }
        }

        return false;
    }

    t_b8 f_io_create_directory_and_parents(const t_str_rdonly path, mem::t_arena *const temp_arena, t_io_directory_creation_result *const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = ec_io_directory_creation_result_success;
        }

        const auto create_dir_if_nonexistent = [o_dir_creation_res, &temp_arena](const t_str_rdonly path) {
            if (f_io_get_path_type(path, temp_arena) == ec_io_path_type_not_found) {
                if (!f_io_create_directory(path, temp_arena, o_dir_creation_res)) {
                    return false;
                }
            }

            return true;
        };

        t_b8 cur_dir_name_is_empty = true;

        ZF_WALK_STR (path, step) {
            if (step.code_pt == '/' || step.code_pt == '\\') {
                if (!cur_dir_name_is_empty) {
                    if (!create_dir_if_nonexistent({f_array_slice(path.bytes, 0, step.byte_index)})) {
                        return false;
                    }

                    cur_dir_name_is_empty = true;
                }
            } else {
                cur_dir_name_is_empty = false;
            }
        }

        if (!cur_dir_name_is_empty) {
            if (!create_dir_if_nonexistent(path)) {
                return false;
            }
        }

        return true;
    }

    t_b8 f_io_create_file_and_parent_directories(const t_str_rdonly path, mem::t_arena *const temp_arena, t_io_directory_creation_result *const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = ec_io_directory_creation_result_success;
        }

        // Get the substring containing all directories and create them.
        ZF_WALK_STR_REVERSE (path, step) {
            if (step.code_pt == '/' || step.code_pt == '\\') {
                if (!f_io_create_directory_and_parents({f_array_slice(path.bytes, 0, step.byte_index)}, temp_arena, o_dir_creation_res)) {
                    return false;
                }

                break;
            }
        }

        // Now that directories are created, create the file.
        t_io_stream fs;

        if (!f_io_open_file(path, ec_io_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        f_io_close_file(&fs);

        return true;
    }

    t_io_path_type f_io_get_path_type(const t_str_rdonly path, mem::t_arena *const temp_arena) {
        const t_str_rdonly path_terminated = f_strs_clone_but_add_terminator(path, temp_arena);

        struct stat info;

        if (stat(f_strs_get_as_cstr(path_terminated), &info) != 0) {
            return ec_io_path_type_not_found;
        }

        if (info.st_mode & S_IFDIR) {
            return ec_io_path_type_directory;
        }

        return ec_io_path_type_file;
    }

    t_str_mut f_io_get_executable_directory(mem::t_arena *const arena) {
#if defined(ZF_PLATFORM_WINDOWS)
        t_static_array<char, MAX_PATH> buf;

        auto len = static_cast<t_i32>(GetModuleFileNameA(nullptr, buf.raw, MAX_PATH));
        ZF_REQUIRE(len != 0);

        for (; len > 0; len--) {
            if (buf[len - 1] == '\\') {
                break;
            }
        }

        const auto result_bytes = mem::f_arena_push_array<t_u8>(arena, len);
        f_algos_copy_all(mem::f_get_array_as_byte_array(f_array_slice(f_array_get_as_nonstatic(buf), 0, len)), result_bytes);
        return {result_bytes};
#elif defined(ZF_PLATFORM_MACOS)
    #error "Platform-specific implementation not yet done!"
#elif defined(ZF_PLATFORM_LINUX)
    #error "Platform-specific implementation not yet done!"
#endif
    }
}
