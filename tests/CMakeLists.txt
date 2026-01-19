add_executable(droplet_analyzer_tests
    smoke_test.cpp
)

target_link_libraries(droplet_analyzer_tests
    PRIVATE
        libdroplet
        GTest::gtest_main
)

add_test(NAME droplet_analyzer_tests COMMAND droplet_analyzer_tests)

if(WIN32 AND DEFINED VCPKG_ROOT)
    set(_vcpkg_bin "${VCPKG_ROOT}/installed/x64-windows/bin")
    if(EXISTS "${_vcpkg_bin}")
        set_tests_properties(
            droplet_analyzer_tests
            PROPERTIES ENVIRONMENT "PATH=${_vcpkg_bin};$ENV{PATH}"
        )
    endif()
endif()
