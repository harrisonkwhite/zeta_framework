project(zf_asset_packer CXX)

add_executable(zf_asset_packer
    src/zap_entry.cpp
    src/zap_packing.cpp
    ../external/c_json/src/cJSON.c

    src/zap_packing.h
)

target_include_directories(zf_asset_packer PRIVATE
    ../external/c_json/include
)

target_link_libraries(zf_asset_packer PRIVATE
    zf_core
)

target_compile_definitions(zf_asset_packer PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
