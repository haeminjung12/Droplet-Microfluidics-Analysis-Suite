# tests/CMakeLists.txt

add_test(
    NAME cmake_cache_has_with_dcam_sdk_bool
    COMMAND ${CMAKE_COMMAND}
        -DCACHE_FILE=${CMAKE_BINARY_DIR}/CMakeCache.txt
        -P ${CMAKE_CURRENT_LIST_DIR}/cmake/verify_with_dcam_sdk_option.cmake
)

