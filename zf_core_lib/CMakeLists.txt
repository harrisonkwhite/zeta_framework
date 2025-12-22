add_library(zf_core_lib STATIC
    src/zcl_basic.cpp
    src/zcl_mem.cpp
    src/zcl_audio.cpp
    src/zcl_math.cpp
    src/zcl_strs.cpp
    src/zcl_gfx.cpp
    src/zcl_io.cpp
    ../external/stb_image/src/stb_image.c
    ../external/stb_truetype/src/stb_truetype.c
	../external/miniaudio/src/miniaudio.c

    include/zcl.h
    include/zcl/zcl_mem.h
    include/zcl/zcl_gfx.h
    include/zcl/zcl_audio.h
    include/zcl/zcl_strs.h
    include/zcl/zcl_ds.h
    include/zcl/zcl_math.h
    include/zcl/zcl_io.h
    include/zcl/zcl_algos.h
    include/zcl/zcl_basic.h
)

target_compile_features(zf_core_lib PUBLIC cxx_std_20)

target_include_directories(zf_core_lib
    PUBLIC include
    PRIVATE ../external/stb_image/include ../external/stb_truetype/include ../external/miniaudio/include
)

if(WIN32)
    target_link_libraries(zf_core_lib PRIVATE dbghelp)
    target_compile_definitions(zf_core_lib PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if (ZF_DEBUG_INFO_IN_RELEASE)
    if (MSVC)
        target_compile_options(zf_core_lib PUBLIC
            $<$<CONFIG:Release>:/Zi /Oy->
        )

        target_link_options(zf_core_lib PUBLIC
            $<$<CONFIG:Release>:/DEBUG>
        )
    endif()
endif()
