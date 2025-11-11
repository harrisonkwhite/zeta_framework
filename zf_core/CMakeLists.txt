add_library(zf_core STATIC
    src/math.cpp
    src/io.cpp
    src/gfx.cpp
    src/audio.cpp
    src/debug.cpp
    src/bits.cpp
    src/strs.cpp
    src/allocators.cpp
    ../external/stb_image/src/stb_image.c
    ../external/stb_truetype/src/stb_truetype.c
	../external/miniaudio/src/miniaudio.c

    include/zc/essential.h
    include/zc/gfx.h
    include/zc/audio.h
    include/zc/io.h
    include/zc/math.h
    include/zc/allocators.h
    include/zc/bits.h
    include/zc/debug.h
    include/zc/heaps.h
    include/zc/sorting.h
    include/zc/strs.h
    include/zc/maps.h
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

target_compile_definitions(zf_core PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
