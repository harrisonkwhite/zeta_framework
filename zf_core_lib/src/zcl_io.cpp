#include <zcl/zcl_io.h>

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef ZF_PLATFORM_WINDOWS
    #include <windows.h>
    #include <direct.h>
#endif

namespace zf::io {
    t_b8 file_open(const strs::t_str_rdonly path, const t_file_access_mode mode, mem::t_arena *const temp_arena, t_stream *const o_stream) {
        const strs::t_str_rdonly path_terminated = strs::str_clone_but_add_terminator(path, temp_arena);

        FILE *file;
        t_stream_mode stream_mode;

        switch (mode) {
        case ec_file_access_mode_read:
            file = fopen(strs::str_to_cstr(path_terminated), "rb");
            stream_mode = ec_stream_mode_read;
            break;

        case ec_file_access_mode_write:
            file = fopen(strs::str_to_cstr(path_terminated), "wb");
            stream_mode = ec_stream_mode_write;
            break;

        case ec_file_access_mode_append:
            file = fopen(strs::str_to_cstr(path_terminated), "ab");
            stream_mode = ec_stream_mode_write;
            break;

        default:
            ZF_UNREACHABLE();
        }

        if (!file) {
            return false;
        }

        *o_stream = file_stream_create(file, stream_mode);

        return true;
    }

    void file_close(t_stream *const stream) {
        fclose(stream->type_data.file.file);
        *stream = {};
    }

    t_i32 file_calc_size(t_stream *const stream) {
        FILE *const file = stream->type_data.file.file;
        const auto pos_old = ftell(file);
        fseek(file, 0, SEEK_END);
        const auto file_size = ftell(file);
        fseek(file, pos_old, SEEK_SET);
        return static_cast<t_i32>(file_size);
    }

    t_b8 file_load_contents(const strs::t_str_rdonly path, mem::t_arena *const contents_arena, mem::t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator) {
        t_stream stream;

        if (!file_open(path, ec_file_access_mode_read, temp_arena, &stream)) {
            return false;
        }

        ZF_DEFER({ file_close(&stream); });

        const t_i32 file_size = file_calc_size(&stream);

        if (add_terminator) {
            *o_contents = mem::arena_push_array<t_u8>(contents_arena, file_size + 1);
            (*o_contents)[file_size] = 0;
        } else {
            *o_contents = mem::arena_push_array<t_u8>(contents_arena, file_size);
        }

        if (!stream_read_items_into_array(&stream, *o_contents, file_size)) {
            return false;
        }

        return true;
    }

    t_b8 create_directory(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_creation_res) {
        if (o_creation_res) {
            *o_creation_res = ec_directory_creation_result_success;
        }

        const strs::t_str_rdonly path_terminated = strs::str_clone_but_add_terminator(path, temp_arena);

#ifdef ZF_PLATFORM_WINDOWS
        const t_i32 result = _mkdir(strs::str_to_cstr(path_terminated));
#else
        const t_s32 result = mkdir(AsCstr(path_terminated), 0755);
#endif

        if (result == 0) {
            return true;
        }

        if (o_creation_res) {
            switch (errno) {
            case EEXIST:
                *o_creation_res = ec_directory_creation_result_already_exists;
                break;

            case EACCES:
            case EPERM:
                *o_creation_res = ec_directory_creation_result_permission_denied;
                break;

            case ENOENT:
                *o_creation_res = ec_directory_creation_result_path_not_found;
                break;

            default:
                *o_creation_res = ec_directory_creation_result_unknown_err;
                break;
            }
        }

        return false;
    }

    t_b8 create_directory_and_parents(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = ec_directory_creation_result_success;
        }

        const auto create_dir_if_nonexistent = [o_dir_creation_res, &temp_arena](const strs::t_str_rdonly path) {
            if (path_get_type(path, temp_arena) == ec_path_type_not_found) {
                if (!create_directory(path, temp_arena, o_dir_creation_res)) {
                    return false;
                }
            }

            return true;
        };

        t_b8 cur_dir_name_is_empty = true;

        ZF_WALK_STR (path, step) {
            if (step.code_pt == '/' || step.code_pt == '\\') {
                if (!cur_dir_name_is_empty) {
                    if (!create_dir_if_nonexistent({array_slice(path.bytes, 0, step.byte_index)})) {
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

    t_b8 create_file_and_parent_directories(const strs::t_str_rdonly path, mem::t_arena *const temp_arena, t_directory_creation_result *const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = ec_directory_creation_result_success;
        }

        // Get the substring containing all directories and create them.
        ZF_WALK_STR_REVERSE (path, step) {
            if (step.code_pt == '/' || step.code_pt == '\\') {
                if (!create_directory_and_parents({array_slice(path.bytes, 0, step.byte_index)}, temp_arena, o_dir_creation_res)) {
                    return false;
                }

                break;
            }
        }

        // Now that directories are created, create the file.
        t_stream fs;

        if (!file_open(path, ec_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        file_close(&fs);

        return true;
    }

    t_path_type path_get_type(const strs::t_str_rdonly path, mem::t_arena *const temp_arena) {
        const strs::t_str_rdonly path_terminated = strs::str_clone_but_add_terminator(path, temp_arena);

        struct stat info;

        if (stat(strs::str_to_cstr(path_terminated), &info) != 0) {
            return ec_path_type_not_found;
        }

        if (info.st_mode & S_IFDIR) {
            return ec_path_type_directory;
        }

        return ec_path_type_file;
    }

    strs::t_str_mut get_executable_directory(mem::t_arena *const arena) {
#if defined(ZF_PLATFORM_WINDOWS)
        t_static_array<char, MAX_PATH> buf;

        auto len = static_cast<t_i32>(GetModuleFileNameA(nullptr, buf.raw, MAX_PATH));
        ZF_REQUIRE(len != 0);

        for (; len > 0; len--) {
            if (buf[len - 1] == '\\') {
                break;
            }
        }

        const auto result_bytes = mem::arena_push_array<t_u8>(arena, len);
        array_copy(mem::array_to_byte_array(array_slice(array_to_nonstatic(buf), 0, len)), result_bytes);
        return {result_bytes};
#elif defined(ZF_PLATFORM_MACOS)
    #error "Platform-specific implementation not yet done!"
#elif defined(ZF_PLATFORM_LINUX)
    #error "Platform-specific implementation not yet done!"
#endif
    }
}
