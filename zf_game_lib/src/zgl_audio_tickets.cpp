#include <zgl/zgl_audio_private.h>

namespace zgl {
#ifdef ZCL_DEBUG
    static zcl::t_u8 g_audio_ticket_identity;
#endif

    t_audio_ticket_mut TicketCreate() {
        return {reinterpret_cast<zcl::t_uintptr>(&g_audio_ticket_identity)};
    }

    zcl::t_b8 TicketCheckValid(const t_audio_ticket_rdonly ticket) {
        return ticket.val == reinterpret_cast<zcl::t_uintptr>(&g_audio_ticket_identity);
    }
}
