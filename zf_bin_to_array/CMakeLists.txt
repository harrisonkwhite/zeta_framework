add_executable(zf_bin_to_array
    src/main.cpp
)

target_link_libraries(zf_bin_to_array PRIVATE
    zf_core
)

target_compile_definitions(zf_bin_to_array PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
