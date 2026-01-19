# tests/CMakeLists.txt

add_executable(droplet_analyzer_tests
    smoke_tests.cpp
)

target_link_libraries(droplet_analyzer_tests
    PRIVATE
        libdroplet
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(droplet_analyzer_tests)
