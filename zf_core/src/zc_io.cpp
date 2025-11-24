#include <zc/zc_io.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#ifdef _WIN32
    #include <direct.h>
#endif

namespace zf {
    t_b8 OpenFile(const s_str_rdonly file_path, const e_file_access_mode mode, s_file_stream& o_fs) {
        ZF_ASSERT(IsStrTerminated(file_path));

        FILE* fs_raw = nullptr;

        switch (mode) {
        case ek_file_access_mode_read:
            fs_raw = fopen(StrRaw(file_path), "rb");
            break;

        case ek_file_access_mode_write:
            fs_raw = fopen(StrRaw(file_path), "wb");
            break;

        case ek_file_access_mode_append:
            fs_raw = fopen(StrRaw(file_path), "ab");
            break;
        }

        if (!fs_raw) {
            return false;
        }

        o_fs.raw = fs_raw;

        return true;
    }

    void CloseFile(s_file_stream& fs) {
        if (fs.raw) {
            fclose(fs.raw);
            fs.raw = nullptr;
        } else {
            ZF_ASSERT(false);
        }
    }

    t_size CalcFileSize(const s_file_stream& fs) {
        const auto pos_old = ftell(fs.raw);
        fseek(fs.raw, 0, SEEK_END);
        const auto file_size = ftell(fs.raw);
        fseek(fs.raw, pos_old, SEEK_SET);
        return static_cast<t_size>(file_size);
    }

    t_b8 LoadFileContents(s_mem_arena& mem_arena, const s_str_rdonly file_path, s_array<t_u8>& o_contents, const t_b8 include_terminating_byte) {
        ZF_ASSERT(IsStrTerminated(file_path));

        s_file_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_read, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        const t_size file_size = CalcFileSize(fs);

        if (!MakeArray(mem_arena, include_terminating_byte ? file_size + 1 : file_size, o_contents)) {
            return false;
        }

        if (ReadItemArrayFromFile(fs, o_contents) < file_size) {
            return false;
        }

        return true;
    }

    t_b8 CreateDirectory(const s_str_rdonly path) {
        ZF_ASSERT(IsStrTerminated(path));

#ifdef ZF_PLATFORM_WINDOWS
        const t_s32 res = _mkdir(StrRaw(path));
#else
        const t_s32 res = mkdir(StrRaw(path), 0755);
#endif

        return res == 0;

#if 0
        if (res == 0) {
            return ec_directory_creation_result::success;
        }

        switch (errno) {
            case EEXIST:
                return ec_directory_creation_result::already_exists;

            case EACCES:
            case EPERM:
                return ec_directory_creation_result::permission_denied;

            case ENOENT:
                return ec_directory_creation_result::path_not_found;

            default:
                return ec_directory_creation_result::unknown_err;
        }
#endif
    }

    t_b8 CreateDirectoryAndParents(const s_str_rdonly path, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(path));

        ZF_DEFER_MEM_ARENA_REWIND(temp_mem_arena);

        // @speed: Ideally we'd start at the end of the path and move back.

        s_str path_clone; // @speed: A clone on every call to this? Yuck!

        if (!MakeArrayClone(temp_mem_arena, path.chrs, path_clone.chrs)) {
            return false;
        }

        t_b8 cur_dir_name_is_empty = true;

        t_size i = 0;

        while (true) {
            if (path_clone.chrs[i] == '/' || path_clone.chrs[i] == '\\' || !path_clone.chrs[i]) {
                if (!cur_dir_name_is_empty) {
                    const char temp = path_clone.chrs[i];

                    path_clone.chrs[i] = '\0'; // Temporarily cut the string off here to form the subpath.

                    if (CheckPathType(path_clone) == ek_path_type_not_found) {
                        if (!CreateDirectory(path_clone)) {
                            return false;
                        }
                    }

                    path_clone.chrs[i] = temp;

                    cur_dir_name_is_empty = true;
                }

                if (!path_clone.chrs[i]) {
                    break;
                }
            } else {
                cur_dir_name_is_empty = false;
            }

            i++;
        }

        return true;
    }

    t_b8 CreateFileAndParentDirs(const s_str_rdonly path, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(path) && IsValidUTF8Str(path));

        ZF_DEFER_MEM_ARENA_REWIND(temp_mem_arena);

        const t_size path_len = CalcUTF8StrLenFastButUnsafe(path);
        const auto path_relevant = Slice(path.chrs, 0, path_len + 1);

        s_str path_clone; // @speed: A clone on every call to this? Yuck!

        if (!MakeArrayClone(temp_mem_arena, path_relevant, path_clone.chrs)) {
            return false;
        }

        for (t_size i = path_len - 1; i >= 0; i--) {
            if (path_clone.chrs[i] == '/' || path_clone.chrs[i] == '\\') {
                if (i > 0) {
                    path_clone.chrs[i] = '\0';

                    if (!CreateDirectoryAndParents(path_clone, temp_mem_arena)) {
                        return false;
                    }
                }

                s_file_stream fs;

                if (!OpenFile(path, ek_file_access_mode_write, fs)) {
                    return false;
                }

                CloseFile(fs);

                break;
            }
        }

        return true;
    }

    e_path_type CheckPathType(const s_str_rdonly path) {
        struct stat info;

        if (stat(StrRaw(path), &info) != 0) {
            return ek_path_type_not_found;
        }

        if (info.st_mode & S_IFDIR) {
            return ek_path_type_directory;
        }

        return ek_path_type_file;
    }
}
