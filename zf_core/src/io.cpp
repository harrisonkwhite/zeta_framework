#include <zc/io.h>

namespace zf {
    c_array<t_u8> LoadFileContents(const c_string_view file_path, c_mem_arena& mem_arena, const bool include_terminating_byte) {
        c_file_reader fr;
        fr.DeferClose();

        if (!fr.Open(file_path)) {
            ZF_LOG_ERROR("Failed to open \"%s\"!", file_path.Raw());
            return {};
        }

        const auto file_size = fr.CalcSize();

        const auto contents = mem_arena.PushArray<t_u8>(include_terminating_byte ? file_size + 1 : file_size);

        if (contents.IsEmpty()) {
            ZF_LOG_ERROR("Failed to reserve memory for the contents of file \"%s\"!", file_path.Raw());
            return {};
        }

        if (fr.Read(contents) < file_size) {
            ZF_LOG_ERROR("Failed to read the contents of \"%s\"!", file_path.Raw());
            return {};
        }

        return contents;
    }
}
