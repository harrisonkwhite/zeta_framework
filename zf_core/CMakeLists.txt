project(zf_core CXX)

add_library(zf_core STATIC
    src/zc_io.cpp
    src/zc_mem.cpp
    src/zc_graphics.cpp
    ../external/stb_image/src/stb_image.c

    include/zc.h
)

target_include_directories(zf_core PUBLIC
    include
    ../external/stb_image/include
)

target_compile_definitions(zf_core PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
