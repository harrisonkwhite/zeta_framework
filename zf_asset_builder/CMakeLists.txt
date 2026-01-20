add_executable(zf_asset_builder
    src/zab_entry.cpp
    src/zab_building.cpp
    src/zab_shaders.cpp
    ../external/c_json/src/cJSON.c

    src/zab.h
)

target_compile_features(zf_asset_builder PRIVATE cxx_std_20)
target_include_directories(zf_asset_builder PRIVATE ../external/c_json/include)
target_link_libraries(zf_asset_builder PRIVATE zf_core_lib reproc)

add_custom_command(TARGET zf_asset_builder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:zf_asset_builder>/tools"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../tools" "$<TARGET_FILE_DIR:zf_asset_builder>/tools"
    COMMENT "Copying tools..."
    VERBATIM
)
