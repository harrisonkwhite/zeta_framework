add_library(zf_core STATIC
    src/math.cpp
    src/io.cpp
    src/gfx.cpp
    ../external/stb_image/src/stb_image.c
    ../external/stb_truetype/src/stb_truetype.c

    include/zc/gfx.h
    include/zc/io.h
    include/zc/math.h
    include/zc/mem/avl_tree.h
    include/zc/mem/bit_vector.h
    include/zc/mem/arrays.h
    include/zc/mem/dynamic_array.h
    include/zc/mem/heaps.h
    include/zc/mem/mem.h
    include/zc/mem/sorting.h
    include/zc/mem/strs.h
    include/zc/mem/unordered_map.h
)

target_include_directories(zf_core PUBLIC
    include
)

target_include_directories(zf_core PRIVATE
    ../external/stb_image/include
    ../external/stb_truetype/include
)

target_link_libraries(zf_core PRIVATE
    reproc
)

target_compile_definitions(zf_core PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
