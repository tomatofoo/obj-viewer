cmake_minimum_required(VERSION 4.3)
project("OBJ Viewer")

option(VENDORED "Use vendored libraries" OFF)
if(VENDORED)
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
else()
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

add_executable(main.out src/main.c)
add_subdirectory(src/include)
target_include_directories(main.out PRIVATE src/include)
target_link_libraries(main.out PRIVATE SDL3::SDL3)

