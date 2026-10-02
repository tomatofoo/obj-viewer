cmake_minimum_required(VERSION 4.0)
project(
    "OBJ Viewer"
    DESCRIPTION "OBJ Viewer"
    VERSION 1.0.0
    LANGUAGES C
)

include(FetchContent)
include(GNUInstallDirs)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE, "Debug")
endif()

set(CMAKE_C_FLAGS "")
set(CMAKE_C_FLAGS_DEBUG "-g -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O3")

function(vendored name url tag)
    FetchContent_Declare(
        ${name}
        GIT_REPOSITORY ${url}
        GIT_TAG ${tag}
    )
    FetchContent_MakeAvailable(${name})
endfunction()

option(DEV "Create a development executable" ON)
if(DEV)
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
    find_package(SDL3_image REQUIRED CONFIG REQUIRED COMPONENTS)
    find_package(SDL3_ttf REQUIRED CONFIG REQUIRED COMPONENTS)

    add_executable(main.out src/main.c)
    add_subdirectory(src/include)
    target_include_directories(main.out PRIVATE src/include)
    target_link_libraries(
        main.out PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
    )
else()
    # https://martin-fieber.de/blog/cmake-cpack-cross-platform-distributables
    vendored(SDL3 https://github.com/libsdl-org/SDL.git release-3.4.16)
    vendored(SDL3_image https://github.com/libsdl-org/SDL_image.git release-3.4.6)
    vendored(SDL3_ttf https://github.com/libsdl-org/SDL_ttf.git release-3.2.2)

    add_executable(main WIN32 MACOSX_BUNDLE src/main.c)
    add_subdirectory(src/include)

    set(SHARED_STATIC_ASSETS "data")

    install(
        TARGETS main
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        include(plat/linux/CMakeLists.txt)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        include(plat/darwin/CMakeLists.txt)
    endif()

    set_target_properties(
        main PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_NAME}_${PROJECT_VERSION}_${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}
        BUILD_RPATH_USE_ORIGIN TRUE # just in case
        BUILD_WITH_INSTALL_RPATH TRUE
    )

    target_include_directories(main PUBLIC src/include)
    target_link_libraries(
        main PUBLIC
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
    )

    set(CPACK_VERBATIM_VARIABLES YES)
    set(CPACK_PACKAGE_VENDOR "Tomatofu")
    set(CPACK_PACKAGE_DIRECTORY dist)
    set(CPACK_SOURCE_PACKAGE_FILE_NAME "obj-viewer-${CMAKE_PROJECT_VERSION}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
    set(CPACK_GENERATOR ZIP)
    include(CPack)
endif()

