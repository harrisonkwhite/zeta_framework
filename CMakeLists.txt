cmake_minimum_required(VERSION 3.28)

project(zeta_framework)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(zfw_core)
add_subdirectory(zfw_asset_packer)
add_subdirectory(zfw_shared)
add_subdirectory(external/cpp_utils)
