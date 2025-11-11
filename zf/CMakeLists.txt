add_library(zf STATIC
    src/zf_game.cpp
    src/zf_window.cpp
    src/zf_rng.cpp
    src/zf_gfx.cpp
    src/zf_rendering.cpp
    src/zf_audio.cpp
	../external/glad/src/glad.c

    include/zf/zf_game.h
    include/zf/zf_window.h
    include/zf/zf_rng.h
    include/zf/zf_gfx.h
    include/zf/zf_rendering.h
    include/zf/zf_audio.h
)

target_include_directories(zf PUBLIC
    include
    ../external/glad/include
)

target_link_libraries(zf PUBLIC
    zf_core
    glfw
)

target_compile_definitions(zf PRIVATE
	_CRT_SECURE_NO_WARNINGS
    GLFW_INCLUDE_NONE
)
