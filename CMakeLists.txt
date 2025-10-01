cmake_minimum_required(VERSION 3.28)

project(zeta_framework)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BGFX_BUILD_TOOLS ON CACHE BOOL "" FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/bgfx.cmake)

add_subdirectory(zf)
add_subdirectory(zf_asset_packer)
add_subdirectory(zf_bin_to_array)
add_subdirectory(zf_core)
add_subdirectory(external/glfw)
add_subdirectory(external/reproc)
