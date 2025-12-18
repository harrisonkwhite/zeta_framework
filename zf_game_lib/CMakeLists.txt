add_library(zf_game_lib STATIC
    src/zgl_game.cpp
    src/zgl_platform.cpp
    src/zgl_gfx.cpp
    src/zgl_audio.cpp

    include/zgl.h
    include/zgl/zgl_game.h
    include/zgl/zgl_platform.h
    include/zgl/zgl_gfx.h
    include/zgl/zgl_audio.h
)

target_compile_features(zf_game_lib PUBLIC cxx_std_20)

target_include_directories(zf_game_lib
    PUBLIC include
    PRIVATE ../external/miniaudio/include
)

target_link_libraries(zf_game_lib
    PUBLIC zf_core_lib
    PRIVATE glfw bgfx bimg bx
)
