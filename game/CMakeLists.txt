add_executable(rglike
    main.cpp
    main_menu_scene.cpp
    gameplay_scene.cpp
    character_builder_scene.cpp
    game_over_scene.cpp
)

# Link engine library
target_link_libraries(rglike PRIVATE engine)

# Copy data files to build directory
add_custom_command(TARGET rglike POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E $<$<CONFIG:Debug>:create_symlink>
                                $<$<NOT:$<CONFIG:Debug>>:copy_directory>
    ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:rglike>/data
    COMMENT "Copying data files to build directory"
)
