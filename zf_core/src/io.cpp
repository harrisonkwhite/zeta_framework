#include <zc/io.h>

namespace zf {
    bool LoadFileContents(c_array<t_u8>& contents, c_mem_arena& mem_arena, const s_str_view file_path, const bool include_terminating_byte) {
        assert(!contents.IsInitted());
        assert(file_path.IsTerminated());

        c_file_stream fs;
        fs.DeferClose();

        if (!fs.Open(file_path, false)) {
            ZF_LOG_ERROR("Failed to open \"%s\"!", file_path.Raw());
            return false;
        }

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
    }
}
