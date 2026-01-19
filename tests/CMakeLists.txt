add_executable(droplet_analyzer_tests
    analysis_results_test.cpp
    data_models_test.cpp
    hash_utils_tests.cpp
    input_source_signatures_test.cpp
    logging_tests.cpp
    math_utils_tests.cpp
    progress_callback_tests.cpp
    smoke_tests.cpp
)

target_link_libraries(droplet_analyzer_tests
    PRIVATE
        libdroplet
        GTest::gtest_main
)

add_test(NAME droplet_analyzer_tests COMMAND droplet_analyzer_tests)

set(vcpkg_bin_dir "${VCPKG_ROOT}/installed/x64-windows/bin")
set(vcpkg_debug_bin_dir "${VCPKG_ROOT}/installed/x64-windows/debug/bin")
if(EXISTS "${vcpkg_bin_dir}")
    set(test_path_env "PATH=${vcpkg_bin_dir}\\;$ENV{PATH}")
    if(EXISTS "${vcpkg_debug_bin_dir}")
        set(test_path_env "PATH=${vcpkg_bin_dir}\\;${vcpkg_debug_bin_dir}\\;$ENV{PATH}")
    endif()

    set_tests_properties(
        droplet_analyzer_tests
        PROPERTIES
            ENVIRONMENT "${test_path_env}"
    )
endif()
