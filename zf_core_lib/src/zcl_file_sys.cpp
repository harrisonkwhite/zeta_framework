#include <zcl/zcl_file_sys.h>

#ifdef ZCL_PLATFORM_WINDOWS
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

    t_b8 DirectoryCreate(const t_str_rdonly path, t_arena *const temp_arena, t_directory_create_result *const o_create_result) {
        if (o_create_result) {
            *o_create_result = ek_directory_create_result_success;
        }

        const t_str_rdonly path_terminated = StrCloneButAddTerminator(path, temp_arena);

#ifdef ZCL_PLATFORM_WINDOWS
        const t_i32 result = _mkdir(StrToCStr(path_terminated));
#elif defined(ZCL_PLATFORM_MACOS)
        static_assert(false); // @todo
#elif defined(ZCL_PLATFORM_LINUX)
        static_assert(false); // @todo
#else
        static_assert(false, "Platform not supported!");
#endif

        if (result == 0) {
            return true;
        }

        if (o_create_result) {
            switch (errno) {
            case EEXIST:
                *o_create_result = ek_directory_create_result_already_exists;
                break;

            case EACCES:
            case EPERM:
                *o_create_result = ek_directory_create_result_permission_denied;
                break;

            case ENOENT:
                *o_create_result = ek_directory_create_result_path_not_found;
                break;

            default:
                *o_create_result = ek_directory_create_result_unknown_error;
                break;
            }
        }

        return false;
    }

    t_b8 DirectoryCreateRecursive(const t_str_rdonly path, t_arena *const temp_arena, t_directory_create_result *const o_create_result) {
        if (o_create_result) {
            *o_create_result = ek_directory_create_result_success;
        }

        const auto create_dir_if_nonexistent = [o_create_result, &temp_arena](const t_str_rdonly path) {
            if (PathGetType(path, temp_arena) == ek_path_type_not_found) {
                if (!DirectoryCreate(path, temp_arena, o_create_result)) {
                    return false;
                }
            }

            return true;
        };

        t_b8 cur_dir_name_is_empty = true;

        ZCL_STR_WALK (path, step) {
            if (step.code_pt == '/' || step.code_pt == '\\') {
                if (!cur_dir_name_is_empty) {
                    if (!create_dir_if_nonexistent({ArraySlice(path.bytes, 0, step.byte_index)})) {
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

        const auto result_bytes = ArenaPushArray<t_u8>(arena, len);
        ArrayCopy(ArrayToByteArray(ArraySlice(ArrayToNonstatic(&buf), 0, len)), result_bytes);
        return {result_bytes};
#elif defined(ZCL_PLATFORM_MACOS)
        static_assert(false); // @todo
#elif defined(ZCL_PLATFORM_LINUX)
        static_assert(false); // @todo
#else
        static_assert(false, "Platform not supported!");
#endif
    }

    t_b8 FileCreate(const t_str_rdonly path, t_arena *const temp_arena) {
        t_file_stream stream;

        if (!FileOpen(path, ek_file_access_mode_write, temp_arena, &stream)) {
            return false;
        }

        FileClose(&stream);

        return true;
    }

    t_b8 FileCreateRecursive(const t_str_rdonly path, t_arena *const temp_arena, t_directory_create_result *const o_dir_create_result) {
        t_file_stream stream;

        if (!FileOpenRecursive(path, ek_file_access_mode_write, temp_arena, &stream, o_dir_create_result)) {
            return false;
        }

        FileClose(&stream);

        return true;
    }

    static t_stream_mode FileAccessModeToStreamMode(const t_file_access_mode access_mode) {
        switch (access_mode) {
        case ek_file_access_mode_read: return ek_stream_mode_read;
        case ek_file_access_mode_write: return ek_stream_mode_write;
        case ek_file_access_mode_append: return ek_stream_mode_write;
        }

        ZCL_UNREACHABLE();
    }

    t_b8 FileOpen(const t_str_rdonly path, const t_file_access_mode mode, t_arena *const temp_arena, t_file_stream *const o_stream) {
        const t_str_rdonly path_terminated = StrCloneButAddTerminator(path, temp_arena);

        const auto file = [mode, path_terminated]() -> FILE * {
            switch (mode) {
            case ek_file_access_mode_read:
                return fopen(StrToCStr(path_terminated), "rb");

            case ek_file_access_mode_write:
                return fopen(StrToCStr(path_terminated), "wb");

            case ek_file_access_mode_append:
                return fopen(StrToCStr(path_terminated), "ab");
            }

            ZCL_UNREACHABLE();
        }();

        if (!file) {
            return false;
        }

        *o_stream = FileStreamCreateOpen(file, FileAccessModeToStreamMode(mode));

        return true;
    }

    t_b8 FileOpenRecursive(const t_str_rdonly path, const t_file_access_mode mode, t_arena *const temp_arena, t_file_stream *const o_stream, t_directory_create_result *const o_dir_create_result) {
        if (o_dir_create_result) {
            *o_dir_create_result = ek_directory_create_result_success;
        }

        // Get the substring containing all directories and create them.
        ZCL_STR_WALK_REVERSE (path, step) {
            if (step.code_pt == '/' || step.code_pt == '\\') {
                if (!DirectoryCreateRecursive({ArraySlice(path.bytes, 0, step.byte_index)}, temp_arena, o_dir_create_result)) {
                    return false;
                }

                break;
            }
        }

        // Now that directories are created, open the file.
        return FileOpen(path, mode, temp_arena, o_stream);
    }

    t_b8 FileReopen(t_file_stream *const stream_current, const t_str_rdonly path, const t_file_access_mode mode, t_arena *const temp_arena, t_file_stream *const o_stream_new) {
        const t_str_rdonly path_terminated = StrCloneButAddTerminator(path, temp_arena);

        const auto file = [mode, path_terminated, stream_current]() -> FILE * {
            switch (mode) {
            case ek_file_access_mode_read:
                return freopen(StrToCStr(path_terminated), "rb", stream_current->file_raw);

            case ek_file_access_mode_write:
                return freopen(StrToCStr(path_terminated), "wb", stream_current->file_raw);

            case ek_file_access_mode_append:
                return freopen(StrToCStr(path_terminated), "ab", stream_current->file_raw);
            }

            ZCL_UNREACHABLE();
        }();

        if (!file) {
            return false;
        }

        if (o_stream_new) {
            *o_stream_new = FileStreamCreateOpen(file, FileAccessModeToStreamMode(mode));
        }

        return true;
    }

    void FileClose(t_file_stream *const stream) {
        ZCL_ASSERT(stream->open);

        fclose(stream->file_raw);

        *stream = {
            .open = false, // Good to be explicit!
        };
    }

    void FileFlush(t_file_stream *const stream) {
        ZCL_ASSERT(stream->open);
        ZCL_ASSERT(stream->mode == ek_stream_mode_write);

        fflush(stream->file_raw);
    }

    t_i32 FileCalcSize(t_file_stream *const stream) {
        ZCL_ASSERT(stream->open);

        const auto pos_old = ftell(stream->file_raw);
        fseek(stream->file_raw, 0, SEEK_END);
        const auto file_size = ftell(stream->file_raw);
        fseek(stream->file_raw, pos_old, SEEK_SET);
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
            *o_contents = ArenaPushArray<t_u8>(contents_arena, file_size + 1);
            (*o_contents)[file_size] = 0;
        } else {
            *o_contents = ArenaPushArray<t_u8>(contents_arena, file_size);
        }

        if (!StreamReadItemsIntoArray(FileStreamGetView(&stream), *o_contents, file_size)) {
            return false;
        }

        return true;
    }
}
