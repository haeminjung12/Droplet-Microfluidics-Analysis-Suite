# cli/CMakeLists.txt

add_executable(DropletAnalyzer
    main.cpp
)

target_link_libraries(DropletAnalyzer PRIVATE libdroplet)

if(WITH_DCAM_SDK)
    target_sources(DropletAnalyzer PRIVATE dcam_link_check.cpp)
    target_link_libraries(DropletAnalyzer PRIVATE DCAM::dcamapi)
endif()
