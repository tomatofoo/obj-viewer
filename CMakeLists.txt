cmake_minimum_required(VERSION 4.3)
project("OBJ Viewer")

include(GNUInstallDirs)

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

option(DEV "Create a development executable" ON)
if(DEV)
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
    add_executable(main WIN32 MACOSX_BUNDLE src/main.c)
    install(
        TARGETS main
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    add_subdirectory(src/include)
    target_include_directories(main PRIVATE src/include)
    target_link_libraries(
        main PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
    )
    set(SHARED_STATIC_ASSETS "data")

    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
      include(plat/darwin/CMakeLists.txt)
    endif ()

    set(CPACK_VERBATIM_VARIABLES YES)
    set(CPACK_PACKAGE_VENDOR "Tomatofu")
    set(CPACK_PACKAGE_DIRECTORY dist)
    set(CPACK_SOURCE_PACKAGE_FILE_NAME "obj-viewer-${CMAKE_PROJECT_VERSION}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
    set(CPACK_GENERATOR ZIP)
    include(CPack)
endif()

