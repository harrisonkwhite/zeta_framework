#include <zgl/zgl_gfx_private.h>

namespace zgl {
#ifdef ZCL_DEBUG
    static zcl::t_u8 g_gfx_ticket_identity;
#endif

    t_gfx_ticket_mut TicketCreate() {
        return {reinterpret_cast<zcl::t_uintptr>(&g_gfx_ticket_identity)};
    }

    zcl::t_b8 TicketCheckValid(const t_gfx_ticket_rdonly ticket) {
        return ticket.val == reinterpret_cast<zcl::t_uintptr>(&g_gfx_ticket_identity);
    }
}
