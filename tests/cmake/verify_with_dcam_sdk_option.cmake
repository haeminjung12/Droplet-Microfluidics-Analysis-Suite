if(NOT DEFINED CACHE_FILE)
    message(FATAL_ERROR "CACHE_FILE is required")
endif()

if(NOT EXISTS "${CACHE_FILE}")
    message(FATAL_ERROR "CMake cache file not found: ${CACHE_FILE}")
endif()

file(READ "${CACHE_FILE}" cache_contents)

string(REGEX MATCH "WITH_DCAM_SDK:BOOL=(ON|OFF)" match "${cache_contents}")
if(NOT match)
    message(FATAL_ERROR "Expected cache entry 'WITH_DCAM_SDK:BOOL=ON|OFF' in ${CACHE_FILE}")
endif()

