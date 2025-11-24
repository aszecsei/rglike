add_library(engine STATIC
    src/engine.cpp
    src/scene_manager.cpp
    src/world.cpp
    src/game_log.cpp
    src/lua_bindings.cpp
    src/ui/log_panel.cpp
    src/ui/world_panel.cpp
    src/action.cpp
        src/utils.hpp
)

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(engine PUBLIC
    ftxui::screen
    ftxui::dom
    ftxui::component
    EnTT::EnTT
    lua::lua
    sol2::sol2
    fmt::fmt
    spdlog::spdlog
)

# Require C++20
target_compile_features(engine PUBLIC cxx_std_20)
