project(zf CXX)

add_library(zf STATIC
    src/zf_game.cpp
    src/zf_input.cpp
    src/zf_rng.cpp

    include/zf.h
    include/zf_game.h
    include/zf_input.h
    include/zf_rng.h
)

target_include_directories(zf PUBLIC
    include
)

target_link_libraries(zf PUBLIC
    zf_core
    glfw
    bgfx
    bimg
    bx
)

target_compile_definitions(zf PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
