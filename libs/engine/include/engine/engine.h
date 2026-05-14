#pragma once

#include "scene_manager.h"
#include "terrain.h"
#include "mob.h"
#include "prop.h"
#include "item.h"
#include "faction.h"
#include "growth_pattern.h"
#include "character_data.h"
#include <ftxui/component/screen_interactive.hpp>
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>
#include <memory>

namespace engine {

class Scene;

/**
 * @brief Core engine managing game lifecycle, rendering, and subsystems.
 *
 * The Engine owns:
 * - FTXUI screen for terminal rendering
 * - Lua state for data/scripting
 * - Scene manager for game state transitions
 * - Terrain, mob, and faction registries for game data
 * - Logging system (console + file output)
 *
 * Lifecycle:
 * 1. Construct engine (initializes subsystems)
 * 2. Load data files via load_data_files()
 * 3. Set initial scene via get_scene_manager().set_scene()
 * 4. Call run() to start main loop (blocks until exit)
 * 5. Shutdown via shutdown() or destructor
 *
 * @example
 * Engine engine;
 * engine.load_data_files("data");
 * engine.get_scene_manager().set_scene(std::make_unique<MainMenuScene>(&engine));
 * engine.run();
 */
class Engine {
public:
    Engine();
    ~Engine();

    void run();
    void shutdown();

    // Load Lua data files from a directory
    void load_data_files(const std::string& data_dir);

    sol::state& get_lua_state() { return lua_; }
    std::shared_ptr<spdlog::logger> get_logger() { return logger_; }
    SceneManager& get_scene_manager() { return scene_manager_; }
    ftxui::ScreenInteractive* get_screen() { return screen_; }
    TerrainRegistry& get_terrain_registry() { return terrain_registry_; }
    MobRegistry& get_mob_registry() { return mob_registry_; }
    PropRegistry& get_prop_registry() { return prop_registry_; }
    ItemRegistry& get_item_registry() { return item_registry_; }
    FactionRegistry& get_faction_registry() { return faction_registry_; }
    GrowthPatternRegistry& get_growth_pattern_registry() { return growth_pattern_registry_; }
    RaceRegistry& get_race_registry() { return race_registry_; }
    CharacterClassRegistry& get_class_registry() { return class_registry_; }

    [[nodiscard]] bool loaded() const { return loaded_; }

private:
    sol::state lua_;
    std::shared_ptr<spdlog::logger> logger_;
    SceneManager scene_manager_;
    ftxui::ScreenInteractive* screen_ = nullptr;
    TerrainRegistry terrain_registry_;
    MobRegistry mob_registry_;
    PropRegistry prop_registry_;
    ItemRegistry item_registry_;
    FactionRegistry faction_registry_;
    GrowthPatternRegistry growth_pattern_registry_;
    RaceRegistry race_registry_;
    CharacterClassRegistry class_registry_;
    bool running_ = false;
    bool loaded_ = false;
};

} // namespace engine
