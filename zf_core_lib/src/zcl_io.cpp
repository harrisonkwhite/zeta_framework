#include <zcl/zcl_io.h>

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef ZF_PLATFORM_WINDOWS
    #include <windows.h>
    #include <direct.h>
#endif

namespace zf {
    t_b8 OpenFile(const s_str_rdonly path, const e_file_access_mode mode, c_arena *const temp_arena, c_stream *const o_stream) {
        const s_str_rdonly path_terminated = AllocStrCloneButAddTerminator(path, temp_arena);

        FILE *file;
        e_stream_mode stream_mode;

        switch (mode) {
        case ek_file_access_mode_read:
            file = fopen(path_terminated.AsCstr(), "rb");
            stream_mode = ek_stream_mode_read;
            break;

        case ek_file_access_mode_write:
            file = fopen(path_terminated.AsCstr(), "wb");
            stream_mode = ek_stream_mode_write;
            break;

        case ek_file_access_mode_append:
            file = fopen(path_terminated.AsCstr(), "ab");
            stream_mode = ek_stream_mode_write;
            break;

        default:
            ZF_UNREACHABLE();
        }

        if (!file) {
            return false;
        }

        *o_stream = {file, stream_mode};

        return true;
    }

    void CloseFile(c_stream *const stream) {
        fclose(stream->File());
        *stream = {};
    }

    t_i32 CalcFileSize(c_stream *const stream) {
        FILE *const file = stream->File();
        const auto pos_old = ftell(file);
        fseek(file, 0, SEEK_END);
        const auto file_size = ftell(file);
        fseek(file, pos_old, SEEK_SET);
        return static_cast<t_i32>(file_size);
    }

    t_b8 LoadFileContents(const s_str_rdonly path, c_arena *const contents_arena, c_arena *const temp_arena, s_array_mut<t_u8> *const o_contents, const t_b8 add_terminator) {
        c_stream stream;

        if (!OpenFile(path, ek_file_access_mode_read, temp_arena, &stream)) {
            return false;
        }

        ZF_DEFER({ CloseFile(&stream); });

        const t_i32 file_size = CalcFileSize(&stream);

        if (add_terminator) {
            *o_contents = AllocArray<t_u8>(file_size + 1, contents_arena);
            (*o_contents)[file_size] = 0;
        } else {
            *o_contents = AllocArray<t_u8>(file_size, contents_arena);
        }

        if (!stream.ReadItemsIntoArray(*o_contents, file_size)) {
            return false;
        }

        return true;
    }

    // @todo: Rename.
    t_b8 CreateDirec(const s_str_rdonly path, c_arena *const temp_arena, e_directory_creation_result *const o_creation_res) {
        if (o_creation_res) {
            *o_creation_res = ek_directory_creation_result_success;
        }

        const s_str_rdonly path_terminated = AllocStrCloneButAddTerminator(path, temp_arena);

#ifdef ZF_PLATFORM_WINDOWS
        const t_i32 res = _mkdir(path_terminated.AsCstr());
#else
        const t_s32 res = mkdir(path_terminated.AsCstr(), 0755);
#endif

        if (res == 0) {
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

    t_b8 CreateDirectoryAndParents(const s_str_rdonly path, c_arena *const temp_arena, e_directory_creation_result *const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = ek_directory_creation_result_success;
        }

        const auto create_dir_if_nonexistent = [o_dir_creation_res, &temp_arena](const s_str_rdonly path) {
            if (CheckPathType(path, temp_arena) == ek_path_type_not_found) {
                if (!CreateDirec(path, temp_arena, o_dir_creation_res)) {
                    return false;
                }
            }

            return true;
        };

        t_b8 cur_dir_name_is_empty = true;

        ZF_WALK_STR (path, info) {
            if (info.code_pt == '/' || info.code_pt == '\\') {
                if (!cur_dir_name_is_empty) {
                    if (!create_dir_if_nonexistent({path.bytes.Slice(0, info.byte_index)})) {
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

    t_b8 CreateFileAndParentDirs(const s_str_rdonly path, c_arena *const temp_arena, e_directory_creation_result *const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = ek_directory_creation_result_success;
        }

        // Get the substring containing all directories and create them.
        ZF_WALK_STR_REVERSE (path, info) {
            if (info.code_pt == '/' || info.code_pt == '\\') {
                if (!CreateDirectoryAndParents({path.bytes.Slice(0, info.byte_index)}, temp_arena, o_dir_creation_res)) {
                    return false;
                }

                break;
            }
        }

        // Now that directories are created, create the file.
        c_stream fs;

        if (!OpenFile(path, ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        CloseFile(&fs);

        return true;
    }

    e_path_type CheckPathType(const s_str_rdonly path, c_arena *const temp_arena) {
        const s_str_rdonly path_terminated = AllocStrCloneButAddTerminator(path, temp_arena);

        struct stat info;

        if (stat(path_terminated.AsCstr(), &info) != 0) {
            return ek_path_type_not_found;
        }

        if (info.st_mode & S_IFDIR) {
            return ek_path_type_directory;
        }

        return ek_path_type_file;
    }

    s_str LoadExecutableDirectory(c_arena *const arena) {
#if defined(ZF_PLATFORM_WINDOWS)
        s_static_array<char, MAX_PATH> buf;

        auto len = static_cast<t_i32>(GetModuleFileNameA(nullptr, buf.raw, MAX_PATH));
        ZF_REQUIRE(len != 0);

        for (; len > 0; len--) {
            if (buf[len - 1] == '\\') {
                break;
            }
        }

        const auto res = AllocArray<t_u8>(len, arena);
        Copy(res, buf.AsNonstatic().Slice(0, len).AsByteArray());
        return {res};
#elif defined(ZF_PLATFORM_MACOS)
    #error "Platform-specific implementation not yet done!"
#elif defined(ZF_PLATFORM_LINUX)
    #error "Platform-specific implementation not yet done!"
#endif
    }
}
