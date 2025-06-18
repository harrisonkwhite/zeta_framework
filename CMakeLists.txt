cmake_minimum_required(VERSION 3.28)
project(zeta_framework C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
find_package(OpenGL REQUIRED)

add_library(zeta_framework STATIC
    src/zfw_game.c
    src/zfw_math.c
    src/zfw_rendering.c
    src/zfw_random.c
    src/zfw_utils.c

    include/zfw_game.h
    include/zfw_math.h
    include/zfw_rendering.h
    include/zfw_random.h
    include/zfw_utils.h

    external/glad/src/glad.c
    external/stb/src/stb.c
)

target_include_directories(zeta_framework PUBLIC
  include
  external/stb/include
  external/glad/include
  ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(zeta_framework PUBLIC
  ${GLFW_LIBRARIES}
  OpenGL::GL
)

target_compile_definitions(zeta_framework PUBLIC GLFW_INCLUDE_NONE)
target_compile_definitions(zeta_framework PRIVATE _CRT_SECURE_NO_WARNINGS)
