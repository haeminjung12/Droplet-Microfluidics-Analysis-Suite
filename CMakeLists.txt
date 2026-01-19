cmake_minimum_required(VERSION 3.25)

project(droplet_pipeline VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(WITH_DCAM_SDK "Build with Hamamatsu DCAM SDK support" OFF)

add_library(libdroplet STATIC
  src/libdroplet.cpp
)

target_include_directories(libdroplet PUBLIC
  "${PROJECT_SOURCE_DIR}/include"
)

add_executable(DropletAnalyzer
  cli/main.cpp
)
target_link_libraries(DropletAnalyzer PRIVATE libdroplet)

enable_testing()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(droplet_analyzer_tests
  tests/dummy_test.cpp
)
target_link_libraries(droplet_analyzer_tests PRIVATE libdroplet GTest::gtest_main)
add_test(NAME droplet_analyzer_tests COMMAND droplet_analyzer_tests)
