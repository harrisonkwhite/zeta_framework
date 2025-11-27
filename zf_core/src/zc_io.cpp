#include <zc/zc_io.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#ifdef _WIN32
    #include <direct.h>
#endif

namespace zf {
    t_b8 OpenFile(const s_str_rdonly file_path, const e_file_access_mode mode, s_mem_arena& temp_mem_arena, s_stream& o_fs) {
        s_str file_path_terminated;

        if (!CloneStrButAddTerminator(file_path, temp_mem_arena, file_path_terminated)) {
            return false;
        }

        o_fs = {
            .type = ek_stream_type_file
        };

        auto& fs_raw = o_fs.type_data.file.fs_raw;

        switch (mode) {
        case ek_file_access_mode_read:
            o_fs.mode = ek_stream_mode_read;
            fs_raw = fopen(StrRaw(file_path_terminated), "rb");
            break;

        case ek_file_access_mode_write:
            o_fs.mode = ek_stream_mode_write;
            fs_raw = fopen(StrRaw(file_path_terminated), "wb");
            break;

        case ek_file_access_mode_append:
            o_fs.mode = ek_stream_mode_write;
            fs_raw = fopen(StrRaw(file_path_terminated), "ab");
            break;
        }

        return fs_raw != nullptr;
    }

    void CloseFile(s_stream& fs) {
        ZF_ASSERT(fs.type == ek_stream_type_file);

        fclose(fs.type_data.file.fs_raw);
        fs = {};
    }

    t_size CalcFileSize(const s_stream& fs) {
        ZF_ASSERT(fs.type == ek_stream_type_file);

        const auto& fs_raw = fs.type_data.file.fs_raw;
        const auto pos_old = ftell(fs_raw);
        fseek(fs_raw, 0, SEEK_END);
        const auto file_size = ftell(fs_raw);
        fseek(fs_raw, pos_old, SEEK_SET);
        return static_cast<t_size>(file_size);
    }

    t_b8 LoadFileContents(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena, s_array<t_u8>& o_contents) {
        s_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_read, temp_mem_arena, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        const t_size file_size = CalcFileSize(fs);

        if (!MakeArray(mem_arena, file_size, o_contents)) {
            return false;
        }

        if (!StreamReadItemsIntoArray(fs, o_contents, file_size)) {
            return false;
        }

        return true;
    }

    t_b8 CreateDirectory(const s_str_rdonly path, s_mem_arena& temp_mem_arena, e_directory_creation_result* const o_creation_res) {
        if (o_creation_res) {
            *o_creation_res = ek_directory_creation_result_success;
        }

        s_str path_terminated;

        if (!CloneStrButAddTerminator(path, temp_mem_arena, path_terminated)) {
            return false;
        }

#ifdef ZF_PLATFORM_WINDOWS
        const t_s32 res = _mkdir(StrRaw(path_terminated));
#else
        const t_s32 res = mkdir(StrRaw(path_terminated), 0755);
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

    t_b8 CreateDirectoryAndParents(const s_str_rdonly path, s_mem_arena& temp_mem_arena, e_directory_creation_result* const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = ek_directory_creation_result_success;
        }

        const auto create_dir_if_nonexistent = [&temp_mem_arena, o_dir_creation_res](const s_str_rdonly path) {
            e_path_type path_type;

            if (!CheckPathType(path, temp_mem_arena, path_type)) {
                return false;
            }

            if (path_type == ek_path_type_not_found) {
                if (!CreateDirectory(path, temp_mem_arena, o_dir_creation_res)) {
                    return false;
                }
            }

            return true;
        };

        t_b8 cur_dir_name_is_empty = true;

        ZF_WALK_STR(path, chr_info) {
            if (chr_info.code_pt == '/' || chr_info.code_pt == '\\') {
                if (!cur_dir_name_is_empty) {
                    if (!create_dir_if_nonexistent({Slice(path.bytes, 0, chr_info.byte_index)})) {
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

    t_b8 CreateFileAndParentDirs(const s_str_rdonly path, s_mem_arena& temp_mem_arena, e_directory_creation_result* const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = ek_directory_creation_result_success;
        }

        // Get a substring of all directories and create those.
        ZF_WALK_STR_REVERSE(path, chr_info) {
            if (chr_info.code_pt == '/' || chr_info.code_pt == '\\') {
                if (!CreateDirectoryAndParents({Slice(path.bytes, 0, chr_info.byte_index)}, temp_mem_arena, o_dir_creation_res)) {
                    return false;
                }

                break;
            }
        }

        // Now that directories are created, create the file.
        s_stream fs;

        if (!OpenFile(path, ek_file_access_mode_write, temp_mem_arena, fs)) {
            return false;
        }

        CloseFile(fs);

        return true;
    }

    t_b8 CheckPathType(const s_str_rdonly path, s_mem_arena& temp_mem_arena, e_path_type& o_type) {
        s_str path_terminated;

        if (!CloneStrButAddTerminator(path, temp_mem_arena, path_terminated)) {
            return false;
        }

        struct stat info;

        if (stat(StrRaw(path_terminated), &info) != 0) {
            o_type = ek_path_type_not_found;
        } else if (info.st_mode & S_IFDIR) {
            o_type = ek_path_type_directory;
        } else {
            o_type = ek_path_type_file;
        }

        return true;
    }
}
