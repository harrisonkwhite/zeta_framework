project(zfw_shared CXX)

add_library(zfw_shared STATIC
    src/zfws.cpp
    ../external/stb_image/src/stb_image.c

    include/zfws.h
)

target_include_directories(zfw_shared PUBLIC
    include
    ../external/stb_image/include
)

target_link_libraries(zfw_shared PUBLIC
    cpp_utils
)

target_compile_definitions(zfw_shared PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
