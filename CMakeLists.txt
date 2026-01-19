cmake_minimum_required(VERSION 3.20)
project(DropletAnalyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Standard CTest option: BUILD_TESTING
include(CTest)

# Set the module path for custom find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# vcpkg (default path aligns with Docs/TECHSPEC.md)
set(VCPKG_ROOT "C:/vcpkg" CACHE PATH "Path to vcpkg root")
if(EXISTS "${VCPKG_ROOT}/installed/x64-windows")
    list(PREPEND CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows")
endif()

option(DROPLET_WITH_QT6 "Enable Qt6 GUI dependencies" OFF)
option(DROPLET_WITH_OPENCV "Enable OpenCV dependency" ON)
option(DROPLET_WITH_TIFF "Enable TIFF dependency" ON)
option(DROPLET_WITH_SPDLOG "Enable spdlog dependency" OFF)
option(DROPLET_WITH_NLOHMANN_JSON "Enable nlohmann/json dependency" ON)
option(DROPLET_WITH_GTEST "Enable GoogleTest dependency" OFF)

if(DROPLET_WITH_QT6)
    include(FindQt6)
endif()
if(DROPLET_WITH_OPENCV)
    include(FindOpenCV)
endif()
if(DROPLET_WITH_TIFF)
    include(FindLibTIFF)
endif()
if(DROPLET_WITH_SPDLOG)
    include(FindSpdlog)
endif()
if(DROPLET_WITH_NLOHMANN_JSON)
    include(FindNlohmannJson)
endif()
if(DROPLET_WITH_GTEST)
    include(FindGTest)
endif()

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# MSVC specific flags
if(MSVC)
    # Add warning flags
    add_compile_options(/W4 /WX)
endif()

# Add subdirectories
add_subdirectory(src)
add_subdirectory(cli)
# add_subdirectory(tests) # Add when tests are available

if(BUILD_TESTING)
    add_test(
        NAME build_droplet_analyzer_tests_target
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target droplet_analyzer_tests --config $<CONFIG>
    )
endif()
