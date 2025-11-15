add_library(zf_core STATIC
    src/zc_allocators.cpp
    src/zc_audio.cpp
    src/zc_gfx.cpp
    src/zc_io.cpp
    src/zc_math.cpp
    src/zc_strs.cpp
    src/zc_debug.cpp
    src/ds/zc_bit_vector.cpp
    ../external/stb_image/src/stb_image.c
    ../external/stb_truetype/src/stb_truetype.c
	../external/miniaudio/src/miniaudio.c

    include/zc.h
    include/zc/zc_allocators.h
    include/zc/zc_audio.h
    include/zc/zc_basic.h
    include/zc/zc_gfx.h
    include/zc/zc_io.h
    include/zc/zc_math.h
    include/zc/zc_sorting.h
    include/zc/zc_strs.h
    include/zc/zc_debug.h
    include/zc/ds/zc_array.h
    include/zc/ds/zc_bit_vector.h
    include/zc/ds/zc_list.h
    include/zc/ds/zc_min_heap.h
    include/zc/ds/zc_unordered_map.h
)

target_include_directories(zf_core PUBLIC
    include
   ../external/miniaudio/include # Might want to make this private in the future?
)

target_include_directories(zf_core PRIVATE
    ../external/stb_image/include
    ../external/stb_truetype/include
)

if(WIN32)
    target_link_libraries(zf_core PRIVATE dbghelp)
endif()

if (MSVC)
    target_compile_options(zf PRIVATE
        $<$<CONFIG:Release>:/Zi /Oy->
    )
    target_link_options(zf PRIVATE
        $<$<CONFIG:Release>:/DEBUG>
    )
endif()

target_compile_definitions(zf_core PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
