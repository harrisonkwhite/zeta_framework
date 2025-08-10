project(zfw_shared C)

add_library(zfw_shared STATIC
    src/zfws.c
)

target_include_directories(zfw_shared PUBLIC
    include
)

target_link_libraries(zfw_shared PUBLIC
    c_utils
)
