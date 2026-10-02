set_source_files_properties(
    ${SHARED_STATIC_ASSETS} PROPERTIES
    MACOSX_PACKAGE_LOCATION ${CMAKE_INSTALL_DATADIR}
)
target_sources(main PUBLIC ${SHARED_STATIC_ASSETS})
add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE_DIR:main>/../Frameworks/libSDL3.0.dylib
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3_image::SDL3_image>
    $<TARGET_FILE_DIR:main>/../Frameworks/libSDL3_image.0.dylib
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3_ttf::SDL3_ttf>
    $<TARGET_FILE_DIR:main>/../Frameworks/libSDL3_ttf.0.dylib
)
if (NOT "${CMAKE_GENERATOR}" STREQUAL "Xcode")
    install(
        FILES
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE:SDL3_image::SDL3_image>
        $<TARGET_FILE:SDL3_ttf::SDL3_ttf>
        DESTINATION $<TARGET_FILE_DIR:main>/../Frameworks/
    )
endif()
# NOTE: IF THE LC_LOAD_DYLIB HAS AN ABSOLUTE PATH
# USE install_name_tool -change /usr/local/opt/... @rpath/....dylib main
# while inside the main.app/Contents/MacOS
set_target_properties(
    main PROPERTIES
    MACOSX_RPATH TRUE
    BUILD_RPATH "@executable_path/../Frameworks/"
    INSTALL_RPATH "@executable_path/../Frameworks/"
    RESOURCE "${SHARED_STATIC_ASSETS}"
)

