add_library(zf STATIC
    src/game.cpp
    src/window.cpp
    src/rng.cpp
    src/rendering.cpp
	../external/glad/src/glad.c

    include/zf/game.h
    include/zf/window.h
    include/zf/rng.h
    include/zf/rendering.h
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
