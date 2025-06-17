cmake_minimum_required(VERSION 3.30)
project(zeta_framework C)

set(CMAKE_C_STANDARD 11)

find_package(glfw3 CONFIG REQUIRED)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS src/*.c)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS include/*.h)
add_library(zeta_framework STATIC
  ${SRCS}
  ${HEADERS}
  ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c
  ${CMAKE_SOURCE_DIR}/external/stb/src/stb.c
)

target_include_directories(zeta_framework PUBLIC
  include
  external/stb/include
  external/glad/include
)

target_link_libraries(zeta_framework PUBLIC glfw opengl32)

target_compile_definitions(zeta_framework PUBLIC GLFW_INCLUDE_NONE)
target_compile_definitions(zeta_framework PRIVATE _CRT_SECURE_NO_WARNINGS)
