#include <zcl/zcl_io.h>

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef ZF_PLATFORM_WINDOWS
    #include <windows.h>
    #include <direct.h>
#endif

namespace zf {
    t_b8 FileOpen(const strs::StrRdonly path, const e_file_access_mode mode, t_arena *const temp_arena, s_stream *const o_stream) {
        const strs::StrRdonly path_terminated = clone_str_but_add_terminator(path, temp_arena);

        FILE *file;
        e_stream_mode stream_mode;

        switch (mode) {
        case ek_file_access_mode_read:
            file = fopen(get_as_cstr(path_terminated), "rb");
            stream_mode = ek_stream_mode_read;
            break;

        case ek_file_access_mode_write:
            file = fopen(get_as_cstr(path_terminated), "wb");
            stream_mode = ek_stream_mode_write;
            break;

        case ek_file_access_mode_append:
            file = fopen(get_as_cstr(path_terminated), "ab");
            stream_mode = ek_stream_mode_write;
            break;

        default:
            ZF_UNREACHABLE();
        }

        if (!file) {
            return false;
        }

        *o_stream = CreateFileStream(file, stream_mode);

        return true;
    }

    void FileClose(s_stream *const stream) {
        fclose(stream->type_data.file.file);
        *stream = {};
    }

    t_i32 FileCalcSize(s_stream *const stream) {
        FILE *const file = stream->type_data.file.file;
        const auto pos_old = ftell(file);
        fseek(file, 0, SEEK_END);
        const auto file_size = ftell(file);
        fseek(file, pos_old, SEEK_SET);
        return static_cast<t_i32>(file_size);
    }

    t_b8 LoadFileContents(const strs::StrRdonly path, t_arena *const contents_arena, t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator) {
        s_stream stream;

        if (!FileOpen(path, ek_file_access_mode_read, temp_arena, &stream)) {
            return false;
        }

        ZF_DEFER({ FileClose(&stream); });

        const t_i32 file_size = FileCalcSize(&stream);

        if (add_terminator) {
            *o_contents = f_mem_push_array<t_u8>(contents_arena, file_size + 1);
            (*o_contents)[file_size] = 0;
        } else {
            *o_contents = f_mem_push_array<t_u8>(contents_arena, file_size);
        }

        if (!ReadItemsIntoArray(&stream, *o_contents, file_size)) {
            return false;
        }

        return true;
    }

    t_b8 CreateDirectoryAssumingParentsExist(const strs::StrRdonly path, t_arena *const temp_arena, e_directory_creation_result *const o_creation_res) {
        if (o_creation_res) {
            *o_creation_res = ek_directory_creation_result_success;
        }

        const strs::StrRdonly path_terminated = clone_str_but_add_terminator(path, temp_arena);

#ifdef ZF_PLATFORM_WINDOWS
        const t_i32 result = _mkdir(get_as_cstr(path_terminated));
#else
        const t_s32 result = mkdir(AsCstr(path_terminated), 0755);
#endif

        if (result == 0) {
            return true;
        }

        if (o_creation_res) {
            switch (errno) {
            case EEXIST:
                *o_creation_res = ek_directory_creation_result_already_exists;
                break;

            case EACCES:
            case EPERM:
                *o_creation_res = ek_directory_creation_result_permission_denied;
                break;

            case ENOENT:
                *o_creation_res = ek_directory_creation_result_path_not_found;
                break;

            default:
                *o_creation_res = ek_directory_creation_result_unknown_err;
                break;
            }
        }

        return false;
    }

    t_b8 CreateDirectoryAndParents(const strs::StrRdonly path, t_arena *const temp_arena, e_directory_creation_result *const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = ek_directory_creation_result_success;
        }

        const auto create_dir_if_nonexistent = [o_dir_creation_res, &temp_arena](const strs::StrRdonly path) {
            if (DeterminePathType(path, temp_arena) == ek_path_type_not_found) {
                if (!CreateDirectoryAssumingParentsExist(path, temp_arena, o_dir_creation_res)) {
                    return false;
                }
            }

            return true;
        };

        t_b8 cur_dir_name_is_empty = true;

        ZF_WALK_STR (path, step) {
            if (step.code_pt == '/' || step.code_pt == '\\') {
                if (!cur_dir_name_is_empty) {
                    if (!create_dir_if_nonexistent({f_mem_slice_array(path.bytes, 0, step.byte_index)})) {
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

    t_b8 CreateFileAndParentDirectories(const strs::StrRdonly path, t_arena *const temp_arena, e_directory_creation_result *const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = ek_directory_creation_result_success;
        }

        // Get the substring containing all directories and create them.
        ZF_WALK_STR_REVERSE (path, step) {
            if (step.code_pt == '/' || step.code_pt == '\\') {
                if (!CreateDirectoryAndParents({f_mem_slice_array(path.bytes, 0, step.byte_index)}, temp_arena, o_dir_creation_res)) {
                    return false;
                }

                break;
            }
        }

        // Now that directories are created, create the file.
        s_stream fs;

        if (!FileOpen(path, ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        FileClose(&fs);

        return true;
    }

    e_path_type DeterminePathType(const strs::StrRdonly path, t_arena *const temp_arena) {
        const strs::StrRdonly path_terminated = clone_str_but_add_terminator(path, temp_arena);

        struct stat info;

        if (stat(get_as_cstr(path_terminated), &info) != 0) {
            return ek_path_type_not_found;
        }

        if (info.st_mode & S_IFDIR) {
            return ek_path_type_directory;
        }

        return ek_path_type_file;
    }

    strs::StrMut LoadExecutableDirectory(t_arena *const arena) {
#if defined(ZF_PLATFORM_WINDOWS)
        t_static_array<char, MAX_PATH> buf;

        auto len = static_cast<t_i32>(GetModuleFileNameA(nullptr, buf.raw, MAX_PATH));
        ZF_REQUIRE(len != 0);

        for (; len > 0; len--) {
            if (buf[len - 1] == '\\') {
                break;
            }
        }

        const auto result_bytes = f_mem_push_array<t_u8>(arena, len);
        CopyAll(f_mem_array_as_byte_array(f_mem_slice_array(f_mem_as_nonstatic_array(buf), 0, len)), result_bytes);
        return {result_bytes};
#elif defined(ZF_PLATFORM_MACOS)
    #error "Platform-specific implementation not yet done!"
#elif defined(ZF_PLATFORM_LINUX)
    #error "Platform-specific implementation not yet done!"
#endif
    }
}
