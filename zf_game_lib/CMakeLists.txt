function(embed_shader SHADER_NAME_LOWER SHADER_NAME_UPPER SHADER_TYPE)
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

    set(SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER_NAME_LOWER}.sc)
    set(VD_FILE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying.def.sc)
    set(BIN_FILE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${SHADER_NAME_LOWER}.bin)
    set(CPP_FILE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${SHADER_NAME_LOWER}.cpp)

    add_custom_command(
        OUTPUT ${CPP_FILE}
        BYPRODUCTS ${BIN_FILE}
        COMMAND ${SHADERC_EXE} -f ${SRC_FILE} -o ${BIN_FILE} --type ${SHADER_TYPE} --platform ${SHADERC_PLATFORM} --profile ${SHADERC_PROFILE} --varyingdef ${VD_FILE} -i ${CMAKE_CURRENT_SOURCE_DIR}/../external/bgfx_cmake/bgfx/src
        COMMAND ${BTA_EXE} ${BIN_FILE} ${CPP_FILE} ${SHADER_NAME_LOWER}_src
        COMMAND_EXPAND_LISTS
        DEPENDS ${SRC_FILE} ${VD_FILE} zf_bin_to_array
        VERBATIM
    )

    set(${SHADER_NAME_UPPER}_CPP ${CPP_FILE} PARENT_SCOPE)
endfunction()

embed_shader(batch_vert_shader BATCH_VERT_SHADER vertex)
embed_shader(batch_frag_shader BATCH_FRAG_SHADER fragment)

set_source_files_properties(
    ${BATCH_VERT_SHADER_CPP}
    ${BATCH_FRAG_SHADER_CPP}
    PROPERTIES GENERATED TRUE
)

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

target_sources(zf_game_lib PRIVATE
    ${BATCH_VERT_SHADER_CPP}
    ${BATCH_FRAG_SHADER_CPP}
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
