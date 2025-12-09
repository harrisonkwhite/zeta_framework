#include <zcl/zcl_io.h>

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
    #include <direct.h>
#endif

namespace zf {
    t_b8 OpenFile(const s_str_rdonly path, const e_file_access_mode mode, s_stream *const o_stream) {
        ZF_ASSERT(IsStrTerminated(path));

        MarkUninitted(o_stream);
        o_stream->type = ek_stream_type_file;

        const auto raw = &o_stream->type_data.file.raw;

        switch (mode) {
        case ek_file_access_mode_read:
            o_stream->mode = ek_stream_mode_read;
            *raw = fopen(StrRaw(path), "rb");
            break;

        case ek_file_access_mode_write:
            o_stream->mode = ek_stream_mode_write;
            *raw = fopen(StrRaw(path), "wb");
            break;

        case ek_file_access_mode_append:
            o_stream->mode = ek_stream_mode_write;
            *raw = fopen(StrRaw(path), "ab");
            break;
        }

        return raw != nullptr;
    }

    void CloseFile(s_stream *const stream) {
        ZF_ASSERT(stream->type == ek_stream_type_file);

        fclose(stream->type_data.file.raw);
        MarkFreed(stream);
    }

    t_len CalcFileSize(s_stream *const stream) {
        ZF_ASSERT(stream->type == ek_stream_type_file);

        const auto &stream_raw = stream->type_data.file.raw;
        const auto pos_old = ftell(stream_raw);
        fseek(stream_raw, 0, SEEK_END);
        const auto file_size = ftell(stream_raw);
        fseek(stream_raw, pos_old, SEEK_SET);
        return static_cast<t_len>(file_size);
    }

    t_b8 LoadFileContents(const s_str_rdonly path, s_mem_arena *const mem_arena, s_array<t_u8> *const o_contents, const t_b8 add_terminator) {
        ZF_ASSERT(IsStrTerminated(path));

        MarkUninitted(o_contents);

        s_stream stream;

        if (!OpenFile(path, ek_file_access_mode_read, &stream)) {
            return false;
        }

        ZF_DEFER({ CloseFile(&stream); });

        const t_len file_size = CalcFileSize(&stream);

        if (!AllocArray(add_terminator ? file_size + 1 : file_size, mem_arena, o_contents)) {
            return false;
        }

        if (!StreamReadItemsIntoArray(&stream, *o_contents, file_size)) {
            return false;
        }

        if (add_terminator) {
            (*o_contents)[file_size] = 0;
        }

        return true;
    }

    t_b8 CreateDirectory(const s_str_rdonly path, e_directory_creation_result *const o_creation_res) {
        ZF_ASSERT(IsStrTerminated(path));

        if (o_creation_res) {
            *o_creation_res = ek_directory_creation_result_success;
        }

#ifdef ZF_PLATFORM_WINDOWS
        const t_i32 res = _mkdir(StrRaw(path));
#else
        const t_s32 res = mkdir(StrRaw(path), 0755);
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

    t_b8 CreateDirectoryAndParents(const s_str_rdonly path, s_mem_arena *const temp_mem_arena, e_directory_creation_result *const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = ek_directory_creation_result_success;
        }

        s_str path_clone = {};

        if (!AllocArray(path.bytes.len + 1, temp_mem_arena, &path_clone.bytes)) {
            return false;
        }

        Copy(path_clone.bytes, path.bytes);

        const auto create_dir_if_nonexistent = [o_dir_creation_res, path_clone]() {
            e_path_type path_type;

            if (!CheckPathType(path_clone, &path_type)) {
                return false;
            }

            if (path_type == ek_path_type_not_found) {
                if (!CreateDirectory(path_clone, o_dir_creation_res)) {
                    return false;
                }
            }

            return true;
        };

        t_b8 cur_dir_name_is_empty = true;

        ZF_WALK_STR(path, chr_info) {
            if (chr_info.code_pt == '/' || chr_info.code_pt == '\\') {
                if (!cur_dir_name_is_empty) {
                    path_clone.bytes[chr_info.byte_index] = '\0';

                    if (!create_dir_if_nonexistent()) {
                        return false;
                    }

                    path_clone.bytes[chr_info.byte_index] = path.bytes[chr_info.byte_index];

                    cur_dir_name_is_empty = true;
                }
            } else {
                cur_dir_name_is_empty = false;
            }
        }

        if (!cur_dir_name_is_empty) {
            if (!create_dir_if_nonexistent()) {
                return false;
            }
        }

        return true;
    }

    t_b8 CreateFileAndParentDirs(const s_str_rdonly path, s_mem_arena *const temp_mem_arena, e_directory_creation_result *const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = ek_directory_creation_result_success;
        }

        // Get a substring of all directories and create those.
        ZF_WALK_STR(path, chr_info) {
            if (chr_info.code_pt == '/' || chr_info.code_pt == '\\') {
                if (!CreateDirectoryAndParents({Slice(path.bytes, 0, chr_info.byte_index)}, temp_mem_arena, o_dir_creation_res)) {
                    return false;
                }

                break;
            }
        }

        // Now that directories are created, create the file.
        s_stream fs;

        if (!OpenFile(path, ek_file_access_mode_write, &fs)) {
            return false;
        }

        CloseFile(&fs);

        return true;
    }

    t_b8 CheckPathType(const s_str_rdonly path, e_path_type *const o_type) {
        ZF_ASSERT(IsStrTerminated(path));

        struct stat info;

        if (stat(StrRaw(path), &info) != 0) {
            *o_type = ek_path_type_not_found;
        } else if (info.st_mode & S_IFDIR) {
            *o_type = ek_path_type_directory;
        } else {
            *o_type = ek_path_type_file;
        }

        return true;
    }
}
