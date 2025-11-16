add_executable(rg_oct_25
    main.cpp
    main_menu_scene.cpp
    gameplay_scene.cpp
    character_builder_scene.cpp
)

# Link engine library
target_link_libraries(rg_oct_25 PRIVATE engine)

# Copy data files to build directory
add_custom_command(TARGET rg_oct_25 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:rg_oct_25>/data
    COMMENT "Copying data files to build directory"
)
