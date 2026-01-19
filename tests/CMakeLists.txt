add_executable(droplet_analyzer_tests
    data_models_test.cpp
    input_source_signatures_test.cpp
    logging_tests.cpp
    smoke_tests.cpp
)

target_link_libraries(droplet_analyzer_tests
    PRIVATE
        libdroplet
        GTest::gtest_main
)

add_test(NAME droplet_analyzer_tests COMMAND droplet_analyzer_tests)

set(vcpkg_bin_dir "${VCPKG_ROOT}/installed/x64-windows/bin")
if(EXISTS "${vcpkg_bin_dir}")
    set_tests_properties(
        droplet_analyzer_tests
        PROPERTIES
            ENVIRONMENT "PATH=${vcpkg_bin_dir};$ENV{PATH}"
    )
endif()
