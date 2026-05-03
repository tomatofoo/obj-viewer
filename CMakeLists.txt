cmake_minimum_required(VERSION 4.3)
project("OBJ Viewer")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE, "Debug")
endif()

set(CMAKE_C_FLAGS "")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

option(VENDORED "Use vendored libraries" OFF)
if(VENDORED)
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
    add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)
    add_subdirectory(vendored/SDL_ttf EXCLUDE_FROM_ALL)
else()
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
    find_package(SDL3_image REQUIRED CONFIG REQUIRED COMPONENTS)
    find_package(SDL3_ttf REQUIRED CONFIG REQUIRED COMPONENTS)
endif()

add_executable(main.out src/main.c)
add_subdirectory(src/include)
target_include_directories(main.out PRIVATE src/include)
target_link_libraries(main.out PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf)

