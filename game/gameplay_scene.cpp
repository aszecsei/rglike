#include "gameplay_scene.h"
#include <engine/engine.h>
#include <engine/ui/log_panel.h>
#include <engine/ui/world_panel.h>
#include <engine/builders/town_builder.h>
#include <engine/map_builder.h>
#include <engine/well512.h>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <spdlog/spdlog.h>
#include "main_menu_scene.h"

using namespace ftxui;

GameplayScene::GameplayScene(engine::Engine* engine, const engine::CharacterCreationData& character_data)
    : engine::Scene(engine), character_data_(character_data), world_(100, 50, "The Town of Millhaven"), game_log_() {

    initialize_map();
    spawn_entities();
    add_initial_log_messages();
    setup_ui();
}

void GameplayScene::initialize_map() {
    // Get terrain from the TerrainRegistry
    auto& terrain_registry = get_engine()->get_terrain_registry();
    const auto& wall = terrain_registry.get("wall");
    const auto& floor = terrain_registry.get("floor");
    const auto& water = terrain_registry.get("water");
    const auto& grass = terrain_registry.get("grass");

    // Build town map using TownBuilder
    engine::MapBuilderChain chain;
    chain.start_with<engine::TownBuilder>(wall.value(), floor.value(), water.value(), grass.value());

    // Build the map state to get both the map and the player start position
    engine::WELL512 rng(12345); // Use a seed for reproducible generation
    engine::MapBuilderState final_state = chain.build_state(100, 50, rng);

    // Set the map and player position from the builder state
    world_.set_map(final_state.finalize());
    world_.set_player_position(final_state.player_start_position.x,
                               final_state.player_start_position.y);
}

void GameplayScene::spawn_entities() {
    auto& registry = world_.get_registry();
    auto& mob_registry = get_engine()->get_mob_registry();
    auto& terrain_registry = get_engine()->get_terrain_registry();

    // Get the map builder state to access entity spawns
    engine::MapBuilderChain chain;
    const auto& wall = terrain_registry.get("wall");
    const auto& floor = terrain_registry.get("floor");
    const auto& water = terrain_registry.get("water");
    const auto& grass = terrain_registry.get("grass");
    chain.start_with<engine::TownBuilder>(wall.value(), floor.value(), water.value(), grass.value());
    engine::WELL512 rng(12345);
    engine::MapBuilderState final_state = chain.build_state(100, 50, rng);

    for (const auto& spawn : final_state.entity_spawns) {
        if (spawn.type == "door") {
            auto door_entity = registry.create();
            registry.emplace<engine::Position>(door_entity, spawn.position.x, spawn.position.y);
            registry.emplace<engine::Door>(door_entity);
            registry.emplace<engine::BlocksMovement>(door_entity);
            registry.emplace<engine::BlocksVision>(door_entity);

            // Create renderable component for the door
            auto& door_comp = registry.get<engine::Door>(door_entity);
            std::string glyph = door_comp.is_open ? door_comp.open_glyph : door_comp.closed_glyph;
            registry.emplace<engine::Renderable>(door_entity, glyph, door_comp.color,
                                                 ftxui::Color::Default, false, 50);
        }
        else {
            // Try to spawn as a mob from the mob registry
            auto mob_template = mob_registry.get(spawn.type);
            if (mob_template) {
                auto mob_entity = registry.create();
                registry.emplace<engine::Position>(mob_entity, spawn.position.x, spawn.position.y);
                registry.emplace<engine::Renderable>(mob_entity,
                                                     mob_template->glyph,
                                                     mob_template->fg_color,
                                                     mob_template->bg_color,
                                                     mob_template->bold,
                                                     mob_template->render_order);

                // Add blocking components
                if (mob_template->blocks_movement) {
                    registry.emplace<engine::BlocksMovement>(mob_entity);
                }
                if (mob_template->blocks_vision) {
                    registry.emplace<engine::BlocksVision>(mob_entity);
                }

                // Add vision component
                if (mob_template->vision_range > 0) {
                    registry.emplace<engine::VisionComponent>(mob_entity, mob_template->vision_range);
                }

                // Add action cooldown component (start ready to act)
                registry.emplace<engine::ActionCooldown>(mob_entity, 0);

                // TODO: Add combat stats components when combat system is implemented
            }
        }
    }
}

void GameplayScene::add_initial_log_messages() {
    // Get race and class names for welcome message
    std::string race_name = "Unknown";
    std::string class_name = "Unknown";

    if (auto race = get_engine()->get_race_registry().get(character_data_.race_id)) {
        race_name = race->name;
    }
    if (auto char_class = get_engine()->get_class_registry().get(character_data_.class_id)) {
        class_name = char_class->name;
    }

    game_log_.entry()
            .color(Color::Green)
            .text("Welcome to the game, ")
            .bold()
            .text(character_data_.name)
            .reset_style()
            .color(Color::Green)
            .text("!")
            .log();

    game_log_.entry()
            .text("You are a ")
            .text(race_name)
            .text(" ")
            .text(class_name)
            .text(" exploring the world.")
            .log();
}

void GameplayScene::setup_ui() {
    auto log_panel = engine::ui::create_log_panel(game_log_);
    auto world_panel = engine::ui::create_world_panel(world_);

    auto split = ResizableSplitRight(log_panel, world_panel, &log_width_);

    // Create a component that handles keyboard input for player movement
    component_ = CatchEvent(Renderer(split, [this, split, log_panel] {
        auto status_text = log_panel->Focused()
            ? text("TAB: Game | Arrow/J/K: Scroll | PgUp/PgDn/Home/End: Jump | Q: Quit") | dim | center
            : text("TAB: Log | Arrow keys: Move | Q: Quit") | dim | center;

        return vbox({
            text(world_.get_map_name()) | bold | center,
            separator(),
            split->Render() | flex,
            separator(),
            status_text,
        }) | border;
    }), [this, log_panel, world_panel](Event event) {
        // Q to quit
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            get_engine()->get_scene_manager().set_scene(std::make_unique<MainMenuScene>(get_engine()));
            return true;
        }

        // Tab to toggle focus
        if (event == Event::Tab) {
            if (!log_panel->Focused()) {
                log_panel->TakeFocus();
            } else {
                world_panel->TakeFocus();
            }
            return true;
        }

        return false;
    });
}

void GameplayScene::update() {
    // Update all game systems (camera, AI, physics, etc.)
    world_.update_systems();
}

Component GameplayScene::get_component() {
    return component_;
}

void GameplayScene::on_enter() {
    spdlog::info("Entered gameplay scene");
}

void GameplayScene::on_exit() {
    spdlog::info("Exited gameplay scene");
}
