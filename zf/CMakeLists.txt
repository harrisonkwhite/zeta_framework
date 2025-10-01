set(SHADERC_EXE $<TARGET_FILE:shaderc>)
set(BIN2C_EXE $<TARGET_FILE:bin2c>)

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
        COMMAND ${CMAKE_COMMAND} -E echo "#include <cstdint>" > ${CPP_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "namespace zf {" >> ${CPP_FILE}
        COMMAND ${BIN2C_EXE} -f ${BIN_FILE} -o ${CPP_FILE}.tmp -n g_${SHADER_NAME}
        COMMAND ${CMAKE_COMMAND} -E cat ${CPP_FILE}.tmp >> ${CPP_FILE}
        COMMAND ${CMAKE_COMMAND} -E rm ${CPP_FILE}.tmp
        COMMAND ${CMAKE_COMMAND} -E echo "const size_t g_${SHADER_NAME}_size = sizeof(g_${SHADER_NAME});" >> ${CPP_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "}" >> ${CPP_FILE}
        DEPENDS ${SRC_FILE} ${VD_FILE} shaderc bin2c
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

add_dependencies(zf shaderc bin2c)

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
