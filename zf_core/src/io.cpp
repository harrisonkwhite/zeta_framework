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

    ec_folder_create_result CreateFolder(const s_str_view path) {
        ZF_ASSERT(path.IsTerminated());

#ifdef _WIN32
        const int res = _mkdir(path.Raw());
#else
        const int res = mkdir(path, 0755);
#endif

        if (res == 0) {
            return ec_folder_create_result::success;
        }

        switch (errno) {
            case EEXIST:
                return ec_folder_create_result::already_exists;

            case EACCES:
            case EPERM:
                return ec_folder_create_result::permission_denied;

            case ENOENT:
                return ec_folder_create_result::path_not_found;

            default:
                return ec_folder_create_result::unknown_err;
        }
    }
}
