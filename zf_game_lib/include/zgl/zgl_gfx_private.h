#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx_public.h>

namespace zgl {
    t_gfx_ticket_mut TicketCreate();
    zcl::t_b8 TicketCheckValid(const t_gfx_ticket_rdonly ticket);
}
