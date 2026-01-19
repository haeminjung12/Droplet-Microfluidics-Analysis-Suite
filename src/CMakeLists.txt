# src/CMakeLists.txt

# Create a static library from the source files in this directory
add_library(libdroplet STATIC)

# Publicly expose the include directory for other modules
target_include_directories(libdroplet PUBLIC ../include)
