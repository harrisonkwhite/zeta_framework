add_library(zf_game_lib STATIC
    src/zgl_game.cpp
    src/zgl_input.cpp
    src/zgl_audio.cpp
	../external/glad/src/glad.c

    include/zgl.h
    include/zgl_private.h
)

target_include_directories(zf_game_lib PUBLIC
    include
    ../external/glad/include
)

target_link_libraries(zf_game_lib PUBLIC
    zf_core_lib
    glfw
)

target_compile_definitions(zf_game_lib PRIVATE
	_CRT_SECURE_NO_WARNINGS
    GLFW_INCLUDE_NONE
)

if (MSVC)
    target_compile_options(zf_game_lib PRIVATE
        $<$<CONFIG:Release>:/Zi /Oy->
    )
    target_link_options(zf_game_lib PRIVATE
        $<$<CONFIG:Release>:/DEBUG>
    )
endif()
