# src/CMakeLists.txt

# Create a static library from the source files in this directory
add_library(libdroplet STATIC
    dummy.cpp
    logging.cpp
)

# Publicly expose the include directory for other modules
target_include_directories(libdroplet PUBLIC ../include)

if(DROPLET_WITH_OPENCV)
    target_include_directories(libdroplet PUBLIC ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(libdroplet PUBLIC ${OpenCV_LIBS})
endif()

if(DROPLET_WITH_TIFF)
    if(TARGET TIFF::TIFF)
        target_link_libraries(libdroplet PUBLIC TIFF::TIFF)
    else()
        target_include_directories(libdroplet PUBLIC ${TIFF_INCLUDE_DIR})
        target_link_libraries(libdroplet PUBLIC ${TIFF_LIBRARIES})
    endif()
endif()

if(DROPLET_WITH_NLOHMANN_JSON)
    target_link_libraries(libdroplet PUBLIC nlohmann_json::nlohmann_json)
endif()

if(DROPLET_WITH_SPDLOG)
    if(TARGET spdlog::spdlog)
        target_link_libraries(libdroplet PUBLIC spdlog::spdlog)
    elseif(TARGET spdlog::spdlog_header_only)
        target_link_libraries(libdroplet PUBLIC spdlog::spdlog_header_only)
    endif()

    if(TARGET fmt::fmt)
        target_link_libraries(libdroplet PUBLIC fmt::fmt)
    endif()
endif()
