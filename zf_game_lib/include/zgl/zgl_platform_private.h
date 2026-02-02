#pragma once

#include <zcl.h>
#include <zgl/zgl_platform_public.h>

namespace zgl {
    // ============================================================
    // @section: External Forward Declarations

    enum t_key_code : zcl::t_i32;

    enum t_mouse_button_code : zcl::t_i32;

    // ==================================================

    t_platform_ticket_mut TicketCreate();

    zcl::t_b8 TicketCheckValid(const t_platform_ticket_rdonly ticket);

    zcl::t_i32 ToGLFWKey(const t_key_code key_code);

    zcl::t_i32 ToGLFWMouseButton(const t_mouse_button_code btn_code);
}
