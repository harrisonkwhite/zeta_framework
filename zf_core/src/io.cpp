#include <zc/io.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#ifdef _WIN32
    #include <direct.h>
#endif

namespace zf {
    bool LoadFileContents(c_array<t_byte>& contents, c_mem_arena& mem_arena, const s_str_view file_path, const bool include_terminating_byte) {
        ZF_ASSERT(contents.IsEmpty());
        ZF_ASSERT(file_path.IsTerminated());

        s_file_stream fs;

        if (!fs.Open(file_path, false)) {
            ZF_LOG_ERROR("Failed to open \"%s\"!", file_path.Raw());
            return false;
        }

        const bool success = [&contents, &mem_arena, file_path, include_terminating_byte, &fs]() {
            const auto file_size = fs.CalcSize();

            if (!contents.Init(mem_arena, include_terminating_byte ? file_size + 1 : file_size)) {
                ZF_LOG_ERROR("Failed to reserve memory for the contents of file \"%s\"!", file_path.Raw());
                return false;
            }

            if (fs.ReadItems(contents) < file_size) {
                ZF_LOG_ERROR("Failed to read the contents of \"%s\"!", file_path.Raw());
                return false;
            }

            return true;
        }();

        fs.Close();

        return success;
    }

    bool CreateDirectory(const s_str_view path) {
        ZF_ASSERT(path.IsTerminated());

#ifdef _WIN32
        const int res = _mkdir(path.Raw());
#else
        const int res = mkdir(path, 0755);
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

    bool CreateDirectoryAndParents(const s_str_view path, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(path.IsTerminated());

        // @speed: Ideally we'd start at the end of the path and move back.

        s_str path_cloned; // @speed: A clone on every call to this? Yuck!

        if (!CloneArray(path_cloned.chrs, temp_mem_arena, path.chrs)) {
            return false;
        }

        bool cur_dir_name_is_empty = true;

        int i = 0;

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

    bool CreateFileAndParentDirs(const s_str_view path, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(path.IsTerminated());

        const int path_len = path.CalcLen();

        s_str path_cloned; // @speed: A clone on every call to this? Yuck!

        if (!CloneArray(path_cloned.chrs, temp_mem_arena, path.chrs.Slice(0, path_len + 1))) {
            return false;
        }

        for (int i = path_len - 1; i >= 0; i--) {
            if (path_cloned.chrs[i] == '/' || path_cloned.chrs[i] == '\\') {
                if (i > 0) {
                    path_cloned.chrs[i] = '\0';

                    if (!CreateDirectoryAndParents(path_cloned, temp_mem_arena)) {
                        return false;
                    }
                }

                const auto fs = fopen(path.Raw(), "w");

                if (!fs) {
                    return false;
                }

                fclose(fs);

                break;
            }
        }

        return true;
    }

    ec_path_type CheckPathType(const s_str_view path) {
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
