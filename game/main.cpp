#include <engine/engine.h>
#include <engine/lua_bindings.h>
#include "main_menu_scene.h"
#include <cstdio>
#include <exception>
#include <memory>

int main() {
    try {
        engine::Engine game_engine{};

        // Generate Lua type definitions for editor support
        // engine::LuaBindings::generate_type_definitions("data/engine_api.lua");

        game_engine.load_data_files("data");

        game_engine.get_scene_manager().set_scene(std::make_unique<MainMenuScene>(&game_engine));

        game_engine.run();
        return 0;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Fatal: %s\n", e.what());
        return 1;
    } catch (...) {
        std::fprintf(stderr, "Fatal: unknown exception\n");
        return 1;
    }
}
