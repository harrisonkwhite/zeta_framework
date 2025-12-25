add_executable(zf_asset_packer
    src/zap_entry.cpp
    src/zap_packing.cpp
    src/zap_shaders.cpp
    ../external/c_json/src/cJSON.c

    src/zap_packing.h
)

target_compile_features(zf_asset_packer PRIVATE cxx_std_20)
target_include_directories(zf_asset_packer PRIVATE ../external/c_json/include)
target_link_libraries(zf_asset_packer PRIVATE zf_core_lib reproc)
