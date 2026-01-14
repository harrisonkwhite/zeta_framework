add_library(zf_core_lib STATIC
    src/basic/zcl_basic_arenas.cpp
    src/basic/zcl_basic_errors.cpp
    src/zcl_bits.cpp
    src/zcl_audio.cpp
    src/zcl_math.cpp
    src/zcl_strs.cpp
    src/gfx/zcl_gfx_textures.cpp
    src/gfx/zcl_gfx_fonts.cpp
    src/gfx/zcl_gfx_shaders.cpp
    src/zcl_file_sys.cpp
    src/zcl_printing.cpp
    src/zcl_streams.cpp
    src/zcl_rand.cpp
    ../external/stb_image/src/stb_image.c
    ../external/stb_truetype/src/stb_truetype.c
	../external/miniaudio/src/miniaudio.c

    include/zcl.h
    include/zcl/zcl_basic.h
    include/zcl/zcl_bits.h
    include/zcl/zcl_gfx.h
    include/zcl/zcl_audio.h
    include/zcl/zcl_strs.h
    include/zcl/zcl_lists.h
    include/zcl/zcl_hash_maps.h
    include/zcl/zcl_math.h
    include/zcl/zcl_streams.h
    include/zcl/zcl_printing.h
    include/zcl/zcl_file_sys.h
    include/zcl/zcl_algos.h
    include/zcl/zcl_rand.h
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
