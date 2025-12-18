add_executable(zf_tests src/zt_main.cpp)
target_compile_features(zf_tests PRIVATE cxx_std_20)
target_link_libraries(zf_tests PRIVATE zf_core_lib)
