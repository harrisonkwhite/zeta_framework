project(zfw_asset_packer C)

add_executable(zfw_asset_packer
    src/zfwap_main.c
    src/zfwap_textures.c
    src/zfwap_fonts.c
    src/zfwap_shaders.c

    ../external/c_json/src/cJSON.c
    ../external/stb/src/stb.c
)

target_include_directories(zfw_asset_packer PRIVATE
    ../external/c_json/include
    ../external/stb/include
)

target_link_libraries(zfw_asset_packer PRIVATE
    zfw_shared
)
