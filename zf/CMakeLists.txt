set(SHADERC_EXE $<TARGET_FILE:shaderc>)
set(BTA_EXE $<TARGET_FILE:zf_bin_to_array>)

function(embed_shader SHADER_NAME SHADER_TYPE)
    set(SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER_NAME}.sc)
    set(VD_FILE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying.def.sc)
    set(BIN_FILE ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.bin)
    set(CPP_FILE ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.cpp)

    add_custom_command(
        OUTPUT ${CPP_FILE}
        COMMAND ${SHADERC_EXE}
            -f ${SRC_FILE}
            -o ${BIN_FILE}
            --type ${SHADER_TYPE}
            --platform windows
            --profile s_5_0
            --varyingdef ${VD_FILE}
            -i ${CMAKE_CURRENT_SOURCE_DIR}/../external/bgfx.cmake/bgfx/src
        COMMAND ${BTA_EXE} ${BIN_FILE} ${CPP_FILE} ${SHADER_NAME}
        DEPENDS ${SRC_FILE} ${VD_FILE} shaderc zf_bin_to_array
        VERBATIM
    )

    set(${SHADER_NAME}_CPP ${CPP_FILE} PARENT_SCOPE)
endfunction()

embed_shader(quad_vs vertex)
embed_shader(quad_fs fragment)

add_library(zf STATIC
    src/zf_game.cpp
    src/zf_window.cpp
    src/zf_rng.cpp
    src/zf_rendering.cpp

    include/zf.h
    include/zf_game.h
    include/zf_window.h
    include/zf_rng.h
    include/zf_rendering.h

    ${quad_vs_CPP}
    ${quad_fs_CPP}
)

add_dependencies(zf shaderc zf_bin_to_array)

target_include_directories(zf PUBLIC
    include
    ${CMAKE_BINARY_DIR}
)

target_link_libraries(zf PUBLIC
    zf_core
    glfw
    bgfx
    bimg
    bx
)

target_compile_definitions(zf PRIVATE _CRT_SECURE_NO_WARNINGS)
