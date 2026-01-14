#include <zcl/zcl_file_sys.h>

#ifdef ZCL_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #include <windows.h>
    #include <direct.h>
#endif

namespace zcl {
    t_path_type PathGetType(const t_str_rdonly path, t_arena *const temp_arena) {
        const t_str_rdonly path_terminated = StrCloneButAddTerminator(path, temp_arena);

        struct stat info;

        if (stat(StrToCStr(path_terminated), &info) != 0) {
            return ek_path_type_not_found;
        }

        if (info.st_mode & S_IFDIR) {
            return ek_path_type_directory;
        }

        return ek_path_type_file;
    }

    t_b8 DirectoryCreate(const t_str_rdonly path, t_arena *const temp_arena, t_directory_create_result *const o_create_res) {
        if (o_create_res) {
            *o_create_res = ek_directory_create_result_success;
        }

        const t_str_rdonly path_terminated = StrCloneButAddTerminator(path, temp_arena);

#ifdef ZCL_PLATFORM_WINDOWS
        const t_i32 result = _mkdir(StrToCStr(path_terminated));
#elif defined(ZCL_PLATFORM_MACOS)
    #error "Platform-specific implementation not yet done!" // @todo
#elif defined(ZCL_PLATFORM_LINUX)
    #error "Platform-specific implementation not yet done!" // @todo
#endif

        if (result == 0) {
            return true;
        }

        if (o_create_res) {
            switch (errno) {
            case EEXIST:
                *o_create_res = ek_directory_create_result_already_exists;
                break;

            case EACCES:
            case EPERM:
                *o_create_res = ek_directory_create_result_permission_denied;
                break;

            case ENOENT:
                *o_create_res = ek_directory_create_result_path_not_found;
                break;

            default:
                *o_create_res = ek_directory_create_result_unknown_error;
                break;
            }
        }

        return false;
    }

    t_b8 DirectoryCreateRecursive(const t_str_rdonly path, t_arena *const temp_arena, t_directory_create_result *const o_create_res) {
        if (o_create_res) {
            *o_create_res = ek_directory_create_result_success;
        }

        const auto create_dir_if_nonexistent = [o_create_res, &temp_arena](const t_str_rdonly path) {
            if (PathGetType(path, temp_arena) == ek_path_type_not_found) {
                if (!DirectoryCreate(path, temp_arena, o_create_res)) {
                    return false;
                }
            }

            return true;
        };

        t_b8 cur_dir_name_is_empty = true;

        ZCL_STR_WALK (path, step) {
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

    t_str_mut GetExecutableDirectory(t_arena *const arena) {
#if defined(ZCL_PLATFORM_WINDOWS)
        t_static_array<char, MAX_PATH> buf;

        auto len = static_cast<t_i32>(GetModuleFileNameA(nullptr, buf.raw, MAX_PATH));
        ZCL_REQUIRE(len != 0);

        for (; len > 0; len--) {
            if (buf[len - 1] == '\\') {
                break;
            }
        }

        const auto result_bytes = arena_push_array<t_u8>(arena, len);
        array_copy(array_to_byte_array(array_slice(array_to_nonstatic(&buf), 0, len)), result_bytes);
        return {result_bytes};
#elif defined(ZCL_PLATFORM_MACOS)
    #error "Platform-specific implementation not yet done!" // @todo
#elif defined(ZCL_PLATFORM_LINUX)
    #error "Platform-specific implementation not yet done!" // @todo
#endif
    }

    t_b8 FileCreate(const t_str_rdonly path, t_arena *const temp_arena) {
        t_file_stream fs;

        if (!FileOpen(path, ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        FileClose(&fs);

        return true;
    }

    t_b8 FileCreateRecursive(const t_str_rdonly path, t_arena *const temp_arena, t_directory_create_result *const o_dir_create_res) {
        if (o_dir_create_res) {
            *o_dir_create_res = ek_directory_create_result_success;
        }

        // Get the substring containing all directories and create them.
        ZCL_STR_WALK_REVERSE (path, step) {
            if (step.code_pt == '/' || step.code_pt == '\\') {
                if (!DirectoryCreateRecursive({array_slice(path.bytes, 0, step.byte_index)}, temp_arena, o_dir_create_res)) {
                    return false;
                }

                break;
            }
        }

        // Now that directories are created, create the file.
        return FileCreate(path, temp_arena);
    }

    t_b8 FileOpen(const t_str_rdonly path, const t_file_access_mode mode, t_arena *const temp_arena, t_file_stream *const o_stream) {
        const t_str_rdonly path_terminated = StrCloneButAddTerminator(path, temp_arena);

        FILE *file;
        t_stream_mode stream_mode;

        switch (mode) {
        case ek_file_access_mode_read:
            file = fopen(StrToCStr(path_terminated), "rb");
            stream_mode = ek_stream_mode_read;
            break;

        case ek_file_access_mode_write:
            file = fopen(StrToCStr(path_terminated), "wb");
            stream_mode = ek_stream_mode_write;
            break;

        case ek_file_access_mode_append:
            file = fopen(StrToCStr(path_terminated), "ab");
            stream_mode = ek_stream_mode_write;
            break;

        default:
            ZCL_UNREACHABLE();
        }

        if (!file) {
            return false;
        }

        *o_stream = FileStreamCreate(file, stream_mode);

        return true;
    }

    void FileClose(t_file_stream *const stream) {
        fclose(stream->file);
        *stream = {};
    }

    t_i32 FileCalcSize(t_file_stream *const stream) {
        const auto pos_old = ftell(stream->file);
        fseek(stream->file, 0, SEEK_END);
        const auto file_size = ftell(stream->file);
        fseek(stream->file, pos_old, SEEK_SET);
        return static_cast<t_i32>(file_size);
    }

    t_b8 FileLoadContents(const t_str_rdonly path, t_arena *const contents_arena, t_arena *const temp_arena, t_array_mut<t_u8> *const o_contents, const t_b8 add_terminator) {
        t_file_stream stream;

        if (!FileOpen(path, ek_file_access_mode_read, temp_arena, &stream)) {
            return false;
        }

        ZCL_DEFER({ FileClose(&stream); });

        const t_i32 file_size = FileCalcSize(&stream);

        if (add_terminator) {
            *o_contents = arena_push_array<t_u8>(contents_arena, file_size + 1);
            (*o_contents)[file_size] = 0;
        } else {
            *o_contents = arena_push_array<t_u8>(contents_arena, file_size);
        }

        if (!StreamReadItemsIntoArray(stream, *o_contents, file_size)) {
            return false;
        }

        return true;
    }
}
