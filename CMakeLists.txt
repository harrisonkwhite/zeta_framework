cmake_minimum_required(VERSION 3.28)

project(zeta_framework)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(zeta_framework)
add_subdirectory(zf_asset_packer)
add_subdirectory(zf_core)
add_subdirectory(external/glfw)
add_subdirectory(external/bgfx.cmake)
add_subdirectory(external/reproc)
