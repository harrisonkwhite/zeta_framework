project(zf_core CXX)

add_library(zf_core STATIC
    src/zc_io.cpp
    src/zc_mem.cpp
    src/zc_gfx.cpp
    ../external/stb_image/src/stb_image.c
    ../external/stb_truetype/src/stb_truetype.c

    include/zc.h
)

target_include_directories(zf_core PUBLIC
    include
)

target_include_directories(zf_core PRIVATE
    ../external/stb_image/include
    ../external/stb_truetype/include
)

target_link_libraries(zf_core PRIVATE
    reproc
)

target_compile_definitions(zf_core PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
