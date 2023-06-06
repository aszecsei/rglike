# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
# file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")
set(HEADER_LIST
        "include/rglike/rglike.hpp"
        "include/rglike/game.hpp"
        "include/rglike/scene.hpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(rglike
        ${HEADER_LIST}
        src/constants.hpp
        src/game.cpp
        src/world.cpp
        src/world.hpp
        src/game_log.cpp
        src/game_log.hpp
        src/ui/log_component.cpp
        src/ui/log_component.hpp
        src/ui/world_component.cpp
        src/ui/world_component.hpp
        src/game_scene.cpp
        src/game_scene.hpp
        src/main_menu_scene.cpp
        src/main_menu_scene.hpp
        src/math.cpp
        src/math.hpp)

# We need this directory, and users of our library will need it too
target_include_directories(rglike PUBLIC include)

# target_link_libraries
target_link_libraries(rglike
        PRIVATE fmt::fmt
        PUBLIC spdlog::spdlog
        PUBLIC ftxui::screen
        PUBLIC ftxui::dom
        PUBLIC ftxui::component
        PUBLIC EnTT::EnTT
        PRIVATE fluency)

# All users of this library will need at least C++11
target_compile_features(rglike PUBLIC cxx_std_11)
