#pragma once

#include <zcl.h>
#include <zgl/zgl_platform_public.h>

namespace zgl {
    t_platform_ticket_mut TicketCreate();
    zcl::t_b8 TicketCheckValid(const t_platform_ticket_rdonly ticket);

    enum t_key_code : zcl::t_i32;          // Forward declaration from Input.
    enum t_mouse_button_code : zcl::t_i32; // Forward declaration from Input.

    zcl::t_i32 ToGLFWKey(const t_key_code key_code);
    zcl::t_i32 ToGLFWMouseButton(const t_mouse_button_code btn_code);
}
