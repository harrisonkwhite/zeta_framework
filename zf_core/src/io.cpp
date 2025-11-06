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

    ec_directory_creation_result CreateDirectory(const s_str_view path) {
        ZF_ASSERT(path.IsTerminated());

#ifdef _WIN32
        const int res = _mkdir(path.Raw());
#else
        const int res = mkdir(path, 0755);
#endif

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
    }

    ec_directory_creation_result CreateDirectoryAndParents(const s_str path) {
        ZF_ASSERT(path.IsTerminated());

        bool cur_dir_name_is_empty = true;

        int i = 0;

        while (true) {
            if (path.chrs[i] == '/' || path.chrs[i] == '\\' || !path.chrs[i]) {
                if (!cur_dir_name_is_empty) {
                    const char temp = path.chrs[i];

                    path.chrs[i] = '\0'; // Temporarily cut the string off here to form the subpath.

                    if (CheckPathType(path) == ec_path_type::not_found) {
                        const auto res = CreateDirectory(path);

                        if (res != ec_directory_creation_result::success) {
                            path.chrs[i] = temp;
                            return res;
                        }
                    }

                    path.chrs[i] = temp;

                    cur_dir_name_is_empty = true;
                }

                if (!path.chrs[i]) {
                    break;
                }
            } else {
                cur_dir_name_is_empty = false;
            }

            i++;
        }

        return ec_directory_creation_result::success;
    }

    bool CreateFileAndParentDirs(const s_str path) {
        ZF_ASSERT(path.IsTerminated());

        const int path_len = path.CalcLen();

        for (int i = path_len - 1; i >= 0; i--) {
            if (path.chrs[i] == '/' || path.chrs[i] == '\\') {
                if (i > 0) {
                    const char temp = path.chrs[i];

                    path.chrs[i] = '\0';

                    const auto res = CreateDirectoryAndParents(path);

                    path.chrs[i] = temp;

                    if (res != ec_directory_creation_result::success) {
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
