#include <zgl/zgl_audio_internal.h>

namespace zgl {
#ifdef ZCL_DEBUG
    constexpr zcl::t_u64 k_audio_sys_magic = 0xCAFEBABE;
    static_assert(k_audio_sys_magic != 0);
#endif

    static zcl::t_b8 g_module_active;

    t_audio_sys *detail::AudioStartup(zcl::t_arena *const arena) {
        ZCL_ASSERT(!g_module_active);

        g_module_active = true;

        const auto sys = zcl::ArenaPushItem<t_audio_sys>(arena);

#ifdef ZCL_DEBUG
        sys->magic = k_audio_sys_magic;
#endif

        if (ma_engine_init(nullptr, &sys->ma_eng) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        return sys;
    }

    void detail::AudioShutdown(t_audio_sys *const sys) {
        ZCL_ASSERT(g_module_active);

        SoundsDestroyAll(sys);

        ma_engine_uninit(&sys->ma_eng);

        *sys = {};

        g_module_active = false;
    }

#ifdef ZCL_DEBUG
    zcl::t_b8 detail::AudioSysCheckValid(const t_audio_sys *const sys) {
        return sys->magic == k_audio_sys_magic;
    }
#endif
}
