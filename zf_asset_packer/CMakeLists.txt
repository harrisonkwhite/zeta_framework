project(zf_asset_packer CXX)

add_executable(zf_asset_packer
    src/zap_main.cpp
    src/zap_textures.cpp
    src/zap_fonts.cpp
    src/zap_shaders.cpp
    ../external/c_json/src/cJSON.c
    ../external/stb_truetype/src/stb_truetype.c

    src/zap.h
)

target_include_directories(zf_asset_packer PRIVATE
    ../external/c_json/include
    ../external/stb_truetype/include
)

target_link_libraries(zf_asset_packer PRIVATE
    zf_core
)

target_compile_definitions(zf_asset_packer PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
