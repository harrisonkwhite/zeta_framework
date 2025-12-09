add_library(zf_core_lib STATIC
    src/zcl_basic.cpp
    src/zcl_mem.cpp
    src/zcl_math.cpp
    src/zcl_strs.cpp

    ../external/stb_image/src/stb_image.c
    ../external/stb_truetype/src/stb_truetype.c
	../external/miniaudio/src/miniaudio.c

    include/zcl.h
    include/zcl/zcl_mem.h
    include/zcl/zcl_strs.h
    include/zcl/zcl_ds.h
    include/zcl/zcl_math.h
    include/zcl/zcl_basic.h
    
)

target_include_directories(zf_core_lib PUBLIC
    include
   ../external/miniaudio/include # Might want to make this private in the future?
)

target_include_directories(zf_core_lib PRIVATE
    ../external/stb_image/include
    ../external/stb_truetype/include
)

if(WIN32)
    target_link_libraries(zf_core_lib PRIVATE dbghelp)
endif()

if (MSVC)
    target_compile_options(zf_core_lib PRIVATE
        $<$<CONFIG:Release>:/Zi /Oy->
    )
    target_link_options(zf_core_lib PRIVATE
        $<$<CONFIG:Release>:/DEBUG>
    )
endif()

target_compile_definitions(zf_core_lib PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
