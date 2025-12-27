add_executable(zf_asset_packer
    src/zap_entry.cpp
    src/zap_packing.cpp
    src/zap_shaders.cpp
    ../external/c_json/src/cJSON.c

    src/zap.h
)

target_compile_features(zf_asset_packer PRIVATE cxx_std_20)
target_include_directories(zf_asset_packer PRIVATE ../external/c_json/include)
target_link_libraries(zf_asset_packer PRIVATE zf_core_lib reproc)

add_custom_command(TARGET zf_asset_packer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:zf_asset_packer>/tools"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../tools" "$<TARGET_FILE_DIR:zf_asset_packer>/tools"
    COMMENT "Copying tools..."
    VERBATIM
)
