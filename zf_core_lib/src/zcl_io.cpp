#include <zcl/zcl_io.h>

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
    #include <direct.h>
#endif

namespace zf {
    t_b8 OpenFile(const s_str_rdonly path, const e_file_access_mode mode, s_stream &o_stream) {
        ZF_ASSERT(path.IsValid());

        FILE *file = nullptr;
        e_stream_mode stream_mode;

        switch (mode) {
        case e_file_access_mode::read:
            file = fopen(path.Raw(), "rb");
            stream_mode = e_stream_mode::read;
            break;

        case e_file_access_mode::write:
            file = fopen(path.Raw(), "wb");
            stream_mode = e_stream_mode::write;
            break;

        case e_file_access_mode::append:
            file = fopen(path.Raw(), "ab");
            stream_mode = e_stream_mode::write;
            break;
        }

        if (!file) {
            return false;
        }

        o_stream = {file, stream_mode};

        return true;
    }

    void CloseFile(s_stream *const stream) {
        ZF_ASSERT(stream->Type() == e_stream_type::file);
        fclose(stream->File());
        *stream = {};
    }

    t_len CalcFileSize(s_stream *const stream) {
        ZF_ASSERT(stream->Type() == e_stream_type::file);

        const auto &file = stream->File();
        const auto pos_old = ftell(file);
        fseek(file, 0, SEEK_END);
        const auto file_size = ftell(file);
        fseek(file, pos_old, SEEK_SET);
        return static_cast<t_len>(file_size);
    }

    t_b8 LoadFileContents(const s_str_rdonly path, s_mem_arena &contents_mem_arena, s_array<t_u8> &o_contents, const t_b8 add_terminator) {
        ZF_ASSERT(path.IsValid());

        s_stream stream;

        if (!OpenFile(path, e_file_access_mode::read, stream)) {
            return false;
        }

        ZF_DEFER({ CloseFile(&stream); });

        const t_len file_size = CalcFileSize(&stream);

        if (!AllocArray(add_terminator ? file_size + 1 : file_size, contents_mem_arena, o_contents)) {
            return false;
        }

        if (stream.ReadItemsIntoArray(o_contents, file_size)) {
            return false;
        }

        return true;
    }

    t_b8 CreateDirectory(const s_str_rdonly path, e_directory_creation_result *const o_creation_res) {
        ZF_ASSERT(path.IsValid());

        if (o_creation_res) {
            *o_creation_res = e_directory_creation_result::success;
        }

#ifdef ZF_PLATFORM_WINDOWS
        const t_i32 res = _mkdir(path.Raw());
#else
        const t_s32 res = mkdir(path.Raw(), 0755);
#endif

        if (res == 0) {
            return true;
        }

        if (o_creation_res) {
            switch (errno) {
            case EEXIST:
                *o_creation_res = e_directory_creation_result::already_exists;
                break;

            case EACCES:
            case EPERM:
                *o_creation_res = e_directory_creation_result::permission_denied;
                break;

            case ENOENT:
                *o_creation_res = e_directory_creation_result::path_not_found;
                break;

            default:
                *o_creation_res = e_directory_creation_result::unknown_err;
                break;
            }
        }

        return false;
    }

    static t_b8 CreateDirectoryAndParentsHelper(const s_str path_mut, e_directory_creation_result *const o_dir_creation_res) {
        ZF_ASSERT(path_mut.IsValid());

        if (o_dir_creation_res) {
            *o_dir_creation_res = e_directory_creation_result::success;
        }

        const auto create_dir_if_nonexistent = [o_dir_creation_res, path_mut]() {
            e_path_type path_type;

            if (!CheckPathType(path_mut, path_type)) {
                return false;
            }

            if (path_type == e_path_type::not_found) {
                if (!CreateDirectory(path_mut, o_dir_creation_res)) {
                    return false;
                }
            }

            return true;
        };

        t_b8 cur_dir_name_is_empty = true;

        ZF_WALK_STR(path_mut, chr_info) {
            if (chr_info.code_pt == '/' || chr_info.code_pt == '\\') {
                if (!cur_dir_name_is_empty) {
                    const auto byte_old = path_mut.bytes[chr_info.byte_index];
                    path_mut.bytes[chr_info.byte_index] = '\0';

                    if (!create_dir_if_nonexistent()) {
                        return false;
                    }

                    path_mut.bytes[chr_info.byte_index] = byte_old;

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

    t_b8 CreateDirectoryAndParents(const s_str_rdonly path, s_mem_arena &temp_mem_arena, e_directory_creation_result *const o_dir_creation_res) {
        ZF_ASSERT(path.IsValid());

        // Create a mutable copy of the path (so terminators can be put in at different points) and call the helper.
        s_str path_clone = {};

        if (!AllocArray(path.bytes.Len(), temp_mem_arena, path_clone.bytes)) {
            return false;
        }

        path.bytes.CopyTo(path_clone.bytes);

        return CreateDirectoryAndParentsHelper(path_clone, o_dir_creation_res);
    }

    t_b8 CreateFileAndParentDirs(const s_str_rdonly path, s_mem_arena &temp_mem_arena, e_directory_creation_result *const o_dir_creation_res) {
        if (o_dir_creation_res) {
            *o_dir_creation_res = e_directory_creation_result::success;
        }

        // Create a mutable copy of the path (so a terminator can be put in).
        s_str path_clone = {};

        if (!AllocArray(path.bytes.Len(), temp_mem_arena, path_clone.bytes)) {
            return false;
        }

        path.bytes.CopyTo(path_clone.bytes);

        // Get the substring containing all directories and create them.
        ZF_WALK_STR_REVERSE(path, chr_info) {
            if (chr_info.code_pt == '/' || chr_info.code_pt == '\\') {
                const auto byte_old = path_clone.bytes[chr_info.byte_index];
                path_clone.bytes[chr_info.byte_index] = '\0';

                if (!CreateDirectoryAndParents(path_clone, temp_mem_arena, o_dir_creation_res)) {
                    return false;
                }

                path_clone.bytes[chr_info.byte_index] = byte_old;

                break;
            }
        }

        // Now that directories are created, create the file.
        s_stream fs;

        if (!OpenFile(path, e_file_access_mode::write, fs)) {
            return false;
        }

        CloseFile(&fs);

        return true;
    }

    t_b8 CheckPathType(const s_str_rdonly path, e_path_type &o_type) {
        ZF_ASSERT(path.IsValid());

        struct stat info;

        if (stat(path.Raw(), &info) != 0) {
            o_type = e_path_type::not_found;
        } else if (info.st_mode & S_IFDIR) {
            o_type = e_path_type::directory;
        } else {
            o_type = e_path_type::file;
        }

        return true;
    }
}
