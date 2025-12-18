cmake_minimum_required(VERSION 3.28)

project(zeta_framework LANGUAGES C CXX)

add_subdirectory(external/glfw)

set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/bgfx_cmake)

add_subdirectory(zf_core_lib)
add_subdirectory(zf_bin_to_array)
add_subdirectory(zf_asset_packer)
add_subdirectory(zf_game_lib)
add_subdirectory(zf_tests)
