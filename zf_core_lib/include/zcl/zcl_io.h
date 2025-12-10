#pragma once

#include <zcl/zcl_math.h>
#include <zcl/zcl_mem.h>
#include <zcl/zcl_strs.h>

#include <cstdio>

namespace zf {
    // ============================================================
    // @section: Streams
    // ============================================================
    enum class e_stream_type : t_i32 {
        invalid,
        mem,
        file
    };

    enum class e_stream_mode : t_i32 {
        read,
        write
    };

    struct s_stream {
    public:
        s_stream() = default;

        s_stream(const s_array<t_u8> bytes, const e_stream_mode mode, const t_len pos = 0)
            : m_type(e_stream_type::mem), m_type_data({.mem = {.bytes = bytes, .pos = pos}}), m_mode(mode) {
            ZF_ASSERT(pos >= 0 && pos <= bytes.Len());
        }

        s_stream(FILE *const file, const e_stream_mode mode)
            : m_type(e_stream_type::file), m_type_data({.file = {.file = file}}), m_mode(mode) {
            ZF_ASSERT(file);
        }

        e_stream_type Type() const {
            return m_type;
        }

        e_stream_mode Mode() const {
            return m_mode;
        }

        t_len Pos() const {
            ZF_ASSERT(m_type == e_stream_type::mem);
            return m_type_data.mem.pos;
        }

        FILE *File() const {
            ZF_ASSERT(m_type == e_stream_type::file);
            return m_type_data.file.file;
        }

        template <typename tp_type>
        [[nodiscard]] t_b8 ReadItem(tp_type *const o_item) {
            ZF_ASSERT(m_mode == e_stream_mode::read);

            constexpr t_len size = ZF_SIZE_OF(tp_type);

            switch (m_type) {
            case e_stream_type::mem: {
                if (m_type_data.mem.pos + size > m_type_data.mem.bytes.Len()) {
                    return false;
                }

                const auto dest = ToBytes(*o_item);
                const auto src = m_type_data.mem.bytes.Slice(m_type_data.mem.pos, m_type_data.mem.pos + size);
                src.CopyTo(dest);

                m_type_data.mem.pos += size;

                return true;
            }

            case e_stream_type::file:
                return fread(o_item, size, 1, m_type_data.file.file) == 1;
            }

            ZF_ASSERT(false);
            return false;
        }

        template <typename tp_type>
        [[nodiscard]] t_b8 WriteItem(const tp_type &item) {
            ZF_ASSERT(m_mode == e_stream_mode::write);

            constexpr t_len size = ZF_SIZE_OF(tp_type);

            switch (m_type) {
            case e_stream_type::mem: {
                if (m_type_data.mem.pos + size > m_type_data.mem.bytes.Len()) {
                    return false;
                }

                const auto dest = m_type_data.mem.bytes.Slice(m_type_data.mem.pos, m_type_data.mem.pos + size);
                const auto src = ToBytes(item);
                src.CopyTo(dest);

                m_type_data.mem.pos += size;

                return true;
            }

            case e_stream_type::file:
                return fwrite(&item, size, 1, m_type_data.file.file) == 1;
            }

            ZF_ASSERT(false);
            return false;
        }

        template <c_nonstatic_mut_array tp_type>
        [[nodiscard]] t_b8 ReadItemsIntoArray(const tp_type arr, const t_len cnt) {
            ZF_ASSERT(m_mode == e_stream_mode::read);
            ZF_ASSERT(cnt >= 0 && cnt <= arr.Len());

            if (cnt == 0) {
                return true;
            }

            switch (m_type) {
            case e_stream_type::mem: {
                const t_len size = ZF_SIZE_OF(arr[0]) * cnt;

                if (m_type_data.mem.pos + size > m_type_data.mem.bytes.Len()) {
                    return false;
                }

                const auto dest = arr.ToBytes();
                const auto src = m_type_data.mem.bytes.Slice(m_type_data.mem.pos, m_type_data.mem.pos + size);
                src.CopyTo(dest);

                m_type_data.mem.pos += size;

                return true;
            }

            case e_stream_type::file:
                return static_cast<t_len>(fread(arr.Raw(), sizeof(arr[0]), static_cast<size_t>(cnt), m_type_data.file.file)) == cnt;
            }

            ZF_ASSERT(false);
            return false;
        }

        template <c_nonstatic_array tp_type>
        [[nodiscard]] t_b8 WriteItemsOfArray(const tp_type arr) {
            ZF_ASSERT(m_mode == e_stream_mode::write);

            if (arr.IsEmpty()) {
                return true;
            }

            switch (m_type) {
            case e_stream_type::mem: {
                const t_len size = arr.SizeInBytes();

                if (m_type_data.mem.pos + size > m_type_data.mem.bytes.Len()) {
                    return false;
                }

                const auto dest = m_type_data.mem.bytes.Slice(m_type_data.mem.pos, m_type_data.mem.pos + size);
                const auto src = arr.ToBytes();
                src.CopyTo(dest);

                m_type_data.mem.pos += size;

                return true;
            }

            case e_stream_type::file:
                return static_cast<t_len>(fwrite(arr.Raw(), sizeof(arr[0]), static_cast<size_t>(arr.Len()), m_type_data.file.file)) == arr.Len();
            }

            ZF_ASSERT(false);
            return false;
        }

    private:
        e_stream_type m_type = e_stream_type::invalid;

        union {
            struct {
                s_array<t_u8> bytes;
                t_len pos;
            } mem;

            struct {
                FILE *file;
            } file;
        } m_type_data = {};

        e_stream_mode m_mode = {};
    };

    inline s_stream StdIn() {
        return {stdin, e_stream_mode::read};
    }

    inline s_stream StdOut() {
        return {stdout, e_stream_mode::write};
    }

    inline s_stream StdError() {
        return {stderr, e_stream_mode::write};
    }

    template <c_nonstatic_array tp_type>
    [[nodiscard]] t_b8 SerializeArray(s_stream *const stream, const tp_type arr) {
        if (!stream->WriteItem(arr.Len())) {
            return false;
        }

        if (!stream->WriteItemsOfArray(arr)) {
            return false;
        }

        return true;
    }

    template <typename tp_type>
    [[nodiscard]] t_b8 DeserializeArray(s_stream *const stream, s_mem_arena *const arr_mem_arena, s_array<tp_type> *const o_arr) {
        t_len len;

        if (!stream->ReadItem(&len)) {
            return false;
        }

        if (len > 0) {
            if (!AllocArray(len, arr_mem_arena, o_arr)) {
                return false;
            }

            if (!stream->ReadItemsIntoArray(*o_arr, len)) {
                return false;
            }
        } else {
            *o_arr = {};
        }

        return true;
    }

    [[nodiscard]] inline t_b8 SerializeBitVec(s_stream *const stream, const s_bit_vec_rdonly bv) {
        if (!stream->WriteItem(bv.BitCount())) {
            return false;
        }

        if (!stream->WriteItemsOfArray(bv.Bytes())) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 DeserializeBitVec(s_stream *const stream, s_mem_arena *const bv_mem_arena, s_bit_vec *const o_bv) {
        t_len bit_cnt;

        if (!stream->ReadItem(&bit_cnt)) {
            return false;
        }

        if (bit_cnt > 0) {
            if (!CreateBitVec(bit_cnt, bv_mem_arena, o_bv)) {
                return false;
            }

            if (!stream->ReadItemsIntoArray(o_bv->Bytes(), o_bv->Bytes().Len())) {
                return false;
            }
        } else {
            *o_bv = {};
        }

        return true;
    }

    // ============================================================
    // @section: Files and Directories
    // ============================================================
    enum class e_file_access_mode : t_i32 {
        read,
        write,
        append
    };

    [[nodiscard]] t_b8 OpenFile(const s_str_rdonly file_path, const e_file_access_mode mode, s_stream *const o_stream);
    void CloseFile(s_stream *const stream);
    t_len CalcFileSize(const s_stream *const stream);
    [[nodiscard]] t_b8 LoadFileContents(const s_str_rdonly file_path, s_mem_arena *const contents_mem_arena, s_array<t_u8> *const o_contents, const t_b8 add_terminator = false);

    enum class e_directory_creation_result : t_i32 {
        success,
        already_exists,
        permission_denied,
        path_not_found,
        unknown_err
    };

    [[nodiscard]] t_b8 CreateDirectory(const s_str_rdonly path, e_directory_creation_result *const o_creation_res = nullptr); // This DOES NOT create non-existent parent directories.
    [[nodiscard]] t_b8 CreateDirectoryAndParents(const s_str_rdonly path, s_mem_arena *const temp_mem_arena, e_directory_creation_result *const o_dir_creation_res = nullptr);
    [[nodiscard]] t_b8 CreateFileAndParentDirs(const s_str_rdonly path, s_mem_arena *const temp_mem_arena, e_directory_creation_result *const o_dir_creation_res = nullptr);

    enum class e_path_type : t_i32 {
        not_found,
        file,
        directory
    };

    [[nodiscard]] t_b8 CheckPathType(const s_str_rdonly path, e_path_type *const o_type);
}
