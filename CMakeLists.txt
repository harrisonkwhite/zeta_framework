cmake_minimum_required(VERSION 3.28)

project(zeta_framework LANGUAGES C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ZF_DEBUG_INFO_IN_RELEASE "Generate debug information in Release builds." ON)

add_subdirectory(external/glfw)

set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/bgfx_cmake)

add_subdirectory(external/reproc)

set(CMAKE_FOLDER "${PROJECT_NAME}")
add_subdirectory(zf_core_lib)
add_subdirectory(zf_bin_to_array)
add_subdirectory(zf_asset_builder)
add_subdirectory(zf_game_lib)
add_subdirectory(zf_tests)
unset(CMAKE_FOLDER)

foreach(t bgfx bx bimg bimg_decode bimg_encode)
    if (TARGET ${t})
        set_target_properties(${t} PROPERTIES FOLDER "${PROJECT_NAME}/external/bgfx")
    endif()
endforeach()

if (TARGET examples)
    set_target_properties(examples PROPERTIES
        FOLDER "${PROJECT_NAME}/external/bgfx"
        EXCLUDE_FROM_ALL TRUE
    )
endif()

foreach(t glfw uninstall update_mappings)
    if (TARGET ${t})
        set_target_properties(${t} PROPERTIES FOLDER "${PROJECT_NAME}/external/GLFW3")
    endif()
endforeach()

set_target_properties(reproc PROPERTIES FOLDER "${PROJECT_NAME}/external/reproc")

set(_NO_WARNINGS
  $<$<CXX_COMPILER_ID:MSVC>:/W0>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)

target_compile_options(glfw PRIVATE ${_NO_WARNINGS})
target_compile_options(bgfx PRIVATE ${_NO_WARNINGS})
target_compile_options(bx PRIVATE ${_NO_WARNINGS})
target_compile_options(bimg PRIVATE ${_NO_WARNINGS})
target_compile_options(bimg_decode PRIVATE ${_NO_WARNINGS})
target_compile_options(bimg_encode PRIVATE ${_NO_WARNINGS})
target_compile_options(reproc PRIVATE ${_NO_WARNINGS})
