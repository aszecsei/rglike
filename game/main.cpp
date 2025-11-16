#include <engine/engine.h>
#include <engine/lua_bindings.h>
#include "main_menu_scene.h"
#include <memory>

int main() {
    engine::Engine game_engine{};

    // Generate Lua type definitions for editor support
    // engine::LuaBindings::generate_type_definitions("data/engine_api.lua");

    // Load data files
    game_engine.load_data_files("data");

    // Set initial scene to main menu
    game_engine.get_scene_manager().set_scene(std::make_unique<MainMenuScene>(&game_engine));

    game_engine.run();
    return 0;
}
