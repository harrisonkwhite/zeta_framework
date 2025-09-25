project(zfw_core CXX)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

add_library(zfw_core STATIC
    src/zfwc_game.cpp
    src/zfwc_input.cpp
    src/zfwc_math.cpp
    src/zfwc_rng.cpp
    src/graphics/zfwc_graphics.cpp
    src/graphics/zfwc_textures.cpp
    src/graphics/zfwc_fonts.cpp
    src/graphics/zfwc_shaders.cpp
    src/graphics/zfwc_surfaces.cpp
    ../external/glad/src/glad.c

    include/zfwc.h
    include/zfwc_game.h
    include/zfwc_input.h
    include/zfwc_graphics.h
    include/zfwc_math.h
    include/zfwc_rng.h
)

target_include_directories(zfw_core PUBLIC
    include
    ../external/glad/include
)

target_link_libraries(zfw_core PUBLIC
    zfw_shared
    glfw
    OpenGL::GL
)

target_compile_definitions(zfw_core PUBLIC
    GLFW_INCLUDE_NONE
)

target_compile_definitions(zfw_core PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
