#include <zgl/zgl_audio_private.h>

namespace zgl {
#ifdef ZCL_DEBUG
    static zcl::t_u8 g_audio_ticket_identity;

    t_audio_ticket_mut TicketCreate() {
        return {reinterpret_cast<zcl::t_uintptr>(&g_audio_ticket_identity)};
    }

    zcl::t_b8 TicketCheckValid(const t_audio_ticket_rdonly ticket) {
        return ticket.val == reinterpret_cast<zcl::t_uintptr>(&g_audio_ticket_identity);
    }
#else
    t_audio_ticket_mut TicketCreate() { return {}; }
    zcl::t_b8 TicketCheckValid(const t_audio_ticket_rdonly ticket) { return true; }
#endif
}
