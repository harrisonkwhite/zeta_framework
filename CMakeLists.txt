cmake_minimum_required(VERSION 3.28)
project(zeta_framework C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(c_utils)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

add_library(zeta_framework STATIC
    src/zfw_game.c
    src/zfw_input.c
    src/zfw_gl.c
    src/zfw_rendering.c
    src/zfw_math.c
    src/zfw_audio.c
    src/zfw_rng.c
    external/glad/src/glad.c
    external/stb/src/stb.c
    external/miniaudio/src/miniaudio.c

    include/zfw.h
    include/zfw_game.h
    include/zfw_input.h
    include/zfw_gl.h
    include/zfw_rendering.h
    include/zfw_math.h
    include/zfw_audio.h
    include/zfw_rng.h
    external/glad/include/glad/glad.h
    external/stb/include/stb_image.h
    external/stb/include/stb_truetype.h
    external/miniaudio/include/miniaudio.h
)

target_include_directories(zeta_framework PUBLIC
    include
    c_utils/include
    external/stb/include
    external/glad/include
    external/miniaudio/include
)

target_link_libraries(zeta_framework PUBLIC
    c_utils
    glfw
    OpenGL::GL
)

target_compile_definitions(zeta_framework PUBLIC
    GLFW_INCLUDE_NONE
)

target_compile_definitions(zeta_framework PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
