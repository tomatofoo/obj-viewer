target_sources(main PRIVATE ${SHARED_STATIC_ASSETS})
install(
    DIRECTORY ${SHARED_STATIC_ASSETS}
    DESTINATION ${CMAKE_INSTALL_DATADIR}
)
add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE_DIR:main>/libSDL3.so.0
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3_image::SDL3_image>
    $<TARGET_FILE_DIR:main>/libSDL3_image.so.0
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3_ttf::SDL3_ttf>
    $<TARGET_FILE_DIR:main>/libSDL3_ttf.so.0
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/${SHARED_STATIC_ASSETS}
    $<TARGET_FILE_DIR:main>/${SHARED_STATIC_ASSETS}
)
install(
    FILES
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE:SDL3_image::SDL3_image>
    $<TARGET_FILE:SDL3_ttf::SDL3_ttf>
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
set_target_properties(
    main PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN"
    RESOURCE "${SHARED_STATIC_ASSETS}"
)

