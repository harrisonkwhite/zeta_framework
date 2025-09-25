project(zf CXX)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

add_library(zf STATIC
    src/zf_game.cpp
    src/zf_input.cpp
    src/zf_rng.cpp
    src/graphics/zf_rendering.cpp
    src/graphics/zf_textures.cpp
    src/graphics/zf_fonts.cpp
    src/graphics/zf_shaders.cpp
    src/graphics/zf_surfaces.cpp
    ../external/glad/src/glad.c

    include/zf.h
    include/zf_game.h
    include/zf_input.h
    include/zf_rendering.h
    include/zf_rng.h
)

target_include_directories(zf PUBLIC
    include
    ../external/glad/include
)

target_link_libraries(zf PUBLIC
    zf_core
    glfw
    OpenGL::GL
)

target_compile_definitions(zf PUBLIC
    GLFW_INCLUDE_NONE
)

target_compile_definitions(zf PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
