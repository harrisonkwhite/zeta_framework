#include <zgl/zgl_platform_private.h>

namespace zgl {
#ifdef ZCL_DEBUG
    static zcl::t_u8 g_platform_ticket_identity;

    t_platform_ticket_mut TicketCreate() {
        return {reinterpret_cast<zcl::t_uintptr>(&g_platform_ticket_identity)};
    }

    zcl::t_b8 TicketCheckValid(const t_platform_ticket_rdonly ticket) {
        return ticket.val == reinterpret_cast<zcl::t_uintptr>(&g_platform_ticket_identity);
    }
#else
    t_platform_ticket_mut TicketCreate() {
        return {};
    }

    zcl::t_b8 TicketCheckValid(const t_platform_ticket_rdonly ticket) {
        return true;
    }
#endif
}
