#include <zc/zc_io.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#ifdef _WIN32
    #include <direct.h>
#endif

namespace zf {
    t_b8 LoadFileContents(c_mem_arena& mem_arena, const s_str_ro file_path, c_array<t_s8>& o_contents, const t_b8 include_terminating_byte) {
        ZF_ASSERT(IsStrTerminated(file_path));

        s_file_stream fs;

        if (!fs.Open(file_path, ec_file_access_mode::read)) {
            ZF_LOG_ERROR("Failed to open \"%s\"!", file_path.Raw());
            return false;
        }

        const t_b8 success = [&o_contents, &mem_arena, file_path, include_terminating_byte, &fs]() {
            const t_size file_size = fs.CalcSize();

            if (!mem_arena.PushArray(include_terminating_byte ? file_size + 1 : file_size, o_contents)) {
                ZF_LOG_ERROR("Failed to reserve memory for the contents of file \"%s\"!", file_path.Raw());
                return false;
            }

            if (fs.ReadItems(o_contents) < file_size) {
                ZF_LOG_ERROR("Failed to read the contents of \"%s\"!", file_path.Raw());
                return false;
            }

            return true;
        }();

        fs.Close();

        return success;
    }

    t_b8 CreateDirectory(const s_str_ro path) {
        ZF_ASSERT(IsStrTerminated(path));

#ifdef _WIN32
        const t_s32 res = _mkdir(path.Raw());
#else
        const t_s32 res = mkdir(path, 0755);
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

    t_b8 CreateDirectoryAndParents(const s_str_ro path, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(path));

        // @speed: Ideally we'd start at the end of the path and move back.

        s_str_mut path_cloned; // @speed: A clone on every call to this? Yuck!

        if (!temp_mem_arena.CloneArray(path.chrs, path_cloned.chrs)) {
            return false;
        }

        t_b8 cur_dir_name_is_empty = true;

        t_size i = 0;

        while (true) {
            if (path_cloned.chrs[i] == '/' || path_cloned.chrs[i] == '\\' || !path_cloned.chrs[i]) {
                if (!cur_dir_name_is_empty) {
                    const char temp = path_cloned.chrs[i];

                    path_cloned.chrs[i] = '\0'; // Temporarily cut the string off here to form the subpath.

                    if (CheckPathType(path_cloned) == ec_path_type::not_found) {
                        if (!CreateDirectory(path_cloned)) {
                            return false;
                        }
                    }

                    path_cloned.chrs[i] = temp;

                    cur_dir_name_is_empty = true;
                }

                if (!path_cloned.chrs[i]) {
                    break;
                }
            } else {
                cur_dir_name_is_empty = false;
            }

            i++;
        }

        return true;
    }

    t_b8 CreateFileAndParentDirs(const s_str_ro path, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(path));

        const t_size path_len = CalcStrLen(path);

        s_str_mut path_cloned; // @speed: A clone on every call to this? Yuck!

        if (!temp_mem_arena.CloneArray(path.chrs.Slice(0, path_len + 1), path_cloned.chrs)) {
            return false;
        }

        for (t_size i = path_len - 1; i >= 0; i--) {
            if (path_cloned.chrs[i] == '/' || path_cloned.chrs[i] == '\\') {
                if (i > 0) {
                    path_cloned.chrs[i] = '\0';

                    if (!CreateDirectoryAndParents(path_cloned, temp_mem_arena)) {
                        return false;
                    }
                }

                const auto fs = fopen(path.Raw(), "wb");

                if (!fs) {
                    return false;
                }

                fclose(fs);

                break;
            }
        }

        return true;
    }

    ec_path_type CheckPathType(const s_str_ro path) {
        struct stat info;

        if (stat(path.Raw(), &info) != 0) {
            return ec_path_type::not_found;
        }

        if (info.st_mode & S_IFDIR) {
            return ec_path_type::directory;
        }

        return ec_path_type::file;
    }
}
