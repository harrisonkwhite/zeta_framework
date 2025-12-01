add_library(zf STATIC
    src/zf_game.cpp
    src/zf_window.cpp
    src/zf_renderer.cpp
    src/zf_audio_sys.cpp
	../external/glad/src/glad.c

    include/zf.h
    include/zf/zf_game.h
    include/zf/zf_window.h
    include/zf/zf_renderer.h
    include/zf/zf_audio_sys.h
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

if (MSVC)
    target_compile_options(zf PRIVATE
        $<$<CONFIG:Release>:/Zi /Oy->
    )
    target_link_options(zf PRIVATE
        $<$<CONFIG:Release>:/DEBUG>
    )
endif()
