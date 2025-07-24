cmake_minimum_required(VERSION 3.28)
project(zeta_framework C)

set(CMAKE_C_STANDARD 11)

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
    src/zfw_math.c
    src/graphics/zfw_rendering.c
    src/graphics/zfw_textures.c
    src/graphics/zfw_fonts.c
    src/graphics/zfw_shaders.c
    src/zfw_audio.c
    src/zfw_random.c
    src/zfw_mem.c
    src/zfw_io.c
    external/glad/src/glad.c
    external/stb/src/stb.c
    external/miniaudio/src/miniaudio.c

    include/zfw.h
    include/zfw_game.h
    include/zfw_math.h
    include/zfw_graphics.h
    include/zfw_audio.h
    include/zfw_random.h
    include/zfw_mem.h
    include/zfw_io.h
    external/glad/include/glad/glad.h
    external/stb/include/stb_image.h
    external/stb/include/stb_truetype.h
    external/miniaudio/include/miniaudio.h
)

target_include_directories(zeta_framework PUBLIC
    include
    external/stb/include
    external/glad/include
    external/miniaudio/include
)

target_link_libraries(zeta_framework PUBLIC
    glfw
    OpenGL::GL
)

target_compile_definitions(zeta_framework PUBLIC
    GLFW_INCLUDE_NONE
)

target_compile_definitions(zeta_framework PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
