project(zfw_core C)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

add_library(zfw_core STATIC
    src/zfwc_game.c
    src/zfwc_input.c
    src/zfwc_math.c
    src/zfwc_rng.c
    src/graphics/zfwc_graphics.c
    src/graphics/zfwc_textures.c
    src/graphics/zfwc_fonts.c
    src/graphics/zfwc_shaders.c
    src/graphics/zfwc_surfaces.c

    ../external/glad/src/glad.c
)

target_include_directories(zfw_core PUBLIC
    include
    ../external/glad/include
)

target_link_libraries(zfw_core PUBLIC
    zfw_shared
    glfw
    OpenGL::GL
)

target_compile_definitions(zfw_core PUBLIC
    GLFW_INCLUDE_NONE
)
