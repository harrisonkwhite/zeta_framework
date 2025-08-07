cmake_minimum_required(VERSION 3.28)
project(zeta_framework C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(external/c_utils)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

add_library(zeta_framework STATIC
    src/zfw_game.c
    src/zfw_input.c
    src/zfw_rendering.c
    src/zfw_math.c
    src/zfw_audio.c
    src/zfw_rng.c

    external/miniaudio/src/miniaudio.c
)

target_include_directories(zeta_framework PUBLIC
    include
    external/miniaudio/include
)

target_link_libraries(zeta_framework PUBLIC
    c_utils
    glfw
)

target_compile_definitions(zeta_framework PUBLIC
    GLFW_INCLUDE_NONE
)

target_compile_definitions(zeta_framework PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
