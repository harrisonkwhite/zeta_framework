function(embed_shader SHADER_FOLDER SHADER_NAME SHADER_TYPE)
    set(BTA_EXE $<TARGET_FILE:zf_bin_to_array>)

    set(SHADERC_PLATFORM
        $<$<PLATFORM_ID:Windows>:windows>
        $<$<PLATFORM_ID:Darwin>:osx>
        $<$<PLATFORM_ID:Linux>:linux>
    )

    set(SHADERC_PROFILE
        $<$<PLATFORM_ID:Windows>:s_5_0>
        $<$<PLATFORM_ID:Darwin>:metal>
        $<$<PLATFORM_ID:Linux>:glsl>
    )

    set(SHADERC_EXE
        $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/../tools/bgfx/shaderc_windows.exe>
        $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/../tools/bgfx/shaderc_macos>
        $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/../tools/bgfx/shaderc_linux>
    )

    set(SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER_FOLDER}/${SHADER_NAME}.sc)
    set(VD_FILE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER_FOLDER}/varying.def.sc)
    set(BIN_FILE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${SHADER_FOLDER}_${SHADER_NAME}.bin)
    set(CPP_FILE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${SHADER_FOLDER}_${SHADER_NAME}.cpp)

    add_custom_command(
        OUTPUT ${CPP_FILE}
        BYPRODUCTS ${BIN_FILE}
        COMMAND ${SHADERC_EXE} -f ${SRC_FILE} -o ${BIN_FILE} --type ${SHADER_TYPE} --platform ${SHADERC_PLATFORM} --profile ${SHADERC_PROFILE} --varyingdef ${VD_FILE} -i ${CMAKE_CURRENT_SOURCE_DIR}/../tools/bgfx/shaderc_include
        COMMAND ${BTA_EXE} ${BIN_FILE} ${CPP_FILE} ${SHADER_FOLDER}_${SHADER_NAME}_src rendering
        COMMAND_EXPAND_LISTS
        DEPENDS ${SRC_FILE} ${VD_FILE} zf_bin_to_array
        VERBATIM
    )

    set(${SHADER_FOLDER}_${SHADER_NAME}_CPP ${CPP_FILE} PARENT_SCOPE)
endfunction()

embed_shader(batch_triangle vert_shader_default vertex)
embed_shader(batch_triangle frag_shader_default fragment)
embed_shader(batch_triangle frag_shader_tint fragment)

set_source_files_properties(
    ${batch_triangle_vert_shader_default_CPP}
    ${batch_triangle_frag_shader_default_CPP}
    ${batch_triangle_frag_shader_tint_CPP}
    PROPERTIES GENERATED TRUE
)

add_library(zf_game_lib STATIC
    src/zgl_game.cpp
    src/zgl_platform.cpp
    src/zgl_input.cpp
    src/zgl_rendering_core.cpp
    src/zgl_rendering_helpers.cpp
    src/zgl_audio.cpp

    include/zgl.h
    include/zgl/zgl_game.h
    include/zgl/zgl_platform.h
    include/zgl/zgl_input.h
    include/zgl/zgl_rendering.h
    include/zgl/zgl_audio.h
)

target_sources(zf_game_lib PRIVATE
    ${batch_triangle_vert_shader_default_CPP}
    ${batch_triangle_frag_shader_default_CPP}
    ${batch_triangle_frag_shader_tint_CPP}
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
