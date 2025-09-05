project(zfw_asset_packer CXX)

add_executable(zfw_asset_packer
    src/zfwap_main.cpp
    src/zfwap_textures.cpp
    src/zfwap_fonts.cpp
    src/zfwap_shaders.cpp
    ../external/c_json/src/cJSON.c
    ../external/stb_truetype/src/stb_truetype.c

    src/zfwap.h
)

target_include_directories(zfw_asset_packer PRIVATE
    ../external/c_json/include
    ../external/stb_truetype/include
)

target_link_libraries(zfw_asset_packer PRIVATE
    zfw_shared
)
