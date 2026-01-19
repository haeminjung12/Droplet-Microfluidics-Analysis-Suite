cmake_minimum_required(VERSION 3.20)
project(DropletAnalyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the module path for custom find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find dependencies
include(FindQt6)
include(FindOpenCV)
include(FindLibTIFF)
include(FindSpdlog)
include(FindNlohmannJson)
include(FindGTest)

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
