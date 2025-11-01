add_executable(zf_asset_packer
    src/entry.cpp
    src/packing.cpp
    ../external/c_json/src/cJSON.c

    src/packing.h
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
