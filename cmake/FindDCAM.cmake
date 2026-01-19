# FindDCAM.cmake

set(DCAM_SDK_ROOT "" CACHE PATH "Path to Hamamatsu DCAM SDK root (contains inc/ and lib/)")

set(_dcam_roots "")
if(DCAM_SDK_ROOT)
    list(APPEND _dcam_roots "${DCAM_SDK_ROOT}")
endif()
if(DEFINED ENV{DCAM_SDK_ROOT})
    list(APPEND _dcam_roots "$ENV{DCAM_SDK_ROOT}")
endif()
if(DEFINED ENV{DCAMSDK4_ROOT})
    list(APPEND _dcam_roots "$ENV{DCAMSDK4_ROOT}")
endif()

find_path(
    DCAM_INCLUDE_DIR
    NAMES dcamapi4.h
    PATH_SUFFIXES inc
    PATHS ${_dcam_roots}
)

find_library(
    DCAM_LIBRARY
    NAMES dcamapi
    PATH_SUFFIXES lib lib/win64
    PATHS ${_dcam_roots}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DCAM REQUIRED_VARS DCAM_INCLUDE_DIR DCAM_LIBRARY)

if(DCAM_FOUND AND NOT TARGET DCAM::dcamapi)
    add_library(DCAM::dcamapi UNKNOWN IMPORTED)
    set_target_properties(
        DCAM::dcamapi
        PROPERTIES
            IMPORTED_LOCATION "${DCAM_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${DCAM_INCLUDE_DIR}"
    )
endif()

