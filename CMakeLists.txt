cmake_minimum_required(VERSION 3.28)

project(zeta_framework)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/utf-8)
endif()

add_subdirectory(zf)
add_subdirectory(zf_asset_packer)
add_subdirectory(zf_bin_to_array)
add_subdirectory(zf_core)
add_subdirectory(external/glfw)
