# cli/CMakeLists.txt

add_executable(DropletAnalyzer
    main.cpp
)

target_link_libraries(DropletAnalyzer PRIVATE libdroplet)
