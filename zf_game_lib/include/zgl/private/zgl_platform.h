#pragma once

#include <zcl.h>

namespace zf
{
    struct s_input_state;
    struct s_platform_layer_info;

    [[nodiscard]] s_platform_layer_info* InitPlatformLayer(
        s_mem_arena* const mem_arena, s_input_state* const input_state);

    void ShutdownPlatformLayer(const s_platform_layer_info* const pli);

    void PollOSEvents();

    void ShowWindow(const s_platform_layer_info* const pli);
    t_b8 ShouldWindowClose(const s_platform_layer_info* const pli);
    void SwapWindowBuffers(const s_platform_layer_info* const pli);
}
