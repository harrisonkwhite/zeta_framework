project(zfw_shared C)

add_library(zfw_shared STATIC
    src/zfws.c
    ../external/stb_image/src/stb_image.c
)

target_include_directories(zfw_shared PUBLIC
    include
    ../external/stb_image/include
)

target_link_libraries(zfw_shared PUBLIC
    c_utils
)
