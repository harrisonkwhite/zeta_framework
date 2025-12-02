add_library(zf STATIC
    src/zf.cpp
	../external/glad/src/glad.c

    include/zf.h
    include/zf_private/zf_gl.h
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
