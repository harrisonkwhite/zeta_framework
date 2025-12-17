set(BTA_EXE $<TARGET_FILE:zf_bin_to_array>)

function(embed_shader SHADER_NAME_LOWER SHADER_NAME_UPPER SHADER_TYPE)
    set(SHADERC_EXE ${CMAKE_CURRENT_SOURCE_DIR}/../tools/external/bgfx/shaderc.exe)
    set(SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER_NAME_LOWER}.sc)
    set(VD_FILE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying.def.sc)
    set(BIN_FILE ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME_LOWER}.bin)
    set(CPP_FILE ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME_LOWER}.cpp)

    add_custom_command(
        OUTPUT ${CPP_FILE}
        COMMAND ${SHADERC_EXE}
            -f ${SRC_FILE}
            -o ${BIN_FILE}
            --type ${SHADER_TYPE}
            --platform windows
            --profile s_5_0
            --varyingdef ${VD_FILE}
            -i ${CMAKE_CURRENT_SOURCE_DIR}/../external/bgfx_cmake/bgfx/src
            COMMAND ${BTA_EXE} ${BIN_FILE} ${CPP_FILE} ${SHADER_NAME_LOWER}_src
        DEPENDS ${SRC_FILE} ${VD_FILE} zf_bin_to_array
        VERBATIM
    )

    set(${SHADER_NAME_UPPER}_CPP ${CPP_FILE} PARENT_SCOPE)
endfunction()

embed_shader(batch_vert_shader BATCH_VERT_SHADER vertex)
embed_shader(batch_frag_shader BATCH_FRAG_SHADER fragment)

add_library(zf_game_lib STATIC
    src/zgl_game.cpp
    src/zgl_platform.cpp
    src/zgl_renderer.cpp
    src/zgl_audio.cpp

    include/zgl.h
    include/zgl/zgl_game.h
    include/zgl/zgl_platform.h
    include/zgl/zgl_renderer.h
    include/zgl/zgl_audio.h

    ${BATCH_VERT_SHADER_CPP}
    ${BATCH_FRAG_SHADER_CPP}
)

add_dependencies(zf_game_lib zf_bin_to_array)

target_include_directories(zf_game_lib PUBLIC
    include
)

target_include_directories(zf_game_lib PRIVATE
    ../external/miniaudio/include
)

target_link_libraries(zf_game_lib PUBLIC
    zf_core_lib
)

target_link_libraries(zf_game_lib PRIVATE
    glfw
    bgfx
    bimg
    bx
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
