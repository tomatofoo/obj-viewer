set_source_files_properties(
    ${SHARED_STATIC_ASSETS} PROPERTIES
    MACOSX_PACKAGE_LOCATION ${CMAKE_INSTALL_DATADIR}
)
target_sources(main PUBLIC ${SHARED_STATIC_ASSETS})
add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE_DIR:main>/../Frameworks/$<TARGET_FILE_NAME:SDL3::SDL3>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3_image::SDL3_image>
    $<TARGET_FILE_DIR:main>/../Frameworks/$<TARGET_FILE_NAME:SDL3_image::SDL3_image>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3_ttf::SDL3_ttf>
    $<TARGET_FILE_DIR:main>/../Frameworks/$<TARGET_FILE_NAME:SDL3_ttf::SDL3_ttf>
)
set_target_properties(
    main PROPERTIES
    INSTALL_RPATH @executable_path/../Frameworks
    RESOURCE "${SHARED_STATIC_ASSETS}"
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

