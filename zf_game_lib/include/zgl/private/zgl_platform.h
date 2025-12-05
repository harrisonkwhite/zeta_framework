#pragma once

#include <zcl.h>

namespace zf {
    struct s_input_state;
    struct s_platform_layer;

    [[nodiscard]] t_b8 InitPlatformLayer(s_mem_arena& mem_arena, s_input_state& input_state, s_platform_layer*& o_pl);

    void ShutdownPlatformLayer(const s_platform_layer& pl);

    void PollOSEvents();

    void ShowWindow(const s_platform_layer& pl);
    t_b8 ShouldWindowClose(const s_platform_layer& pl);
    void SwapWindowBuffers(const s_platform_layer& pl);
}
