#include "gameplay_scene.h"
#include <engine/engine.h>
#include <engine/ui/log_panel.h>
#include <engine/ui/world_panel.h>
#include <engine/ui/stats_panel.h>
#include <engine/builders/town_builder.h>
#include <engine/constants.h>
#include <engine/map_builder.h>
#include <engine/well512.h>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <spdlog/spdlog.h>
#include "main_menu_scene.h"
#include "game_over_scene.h"

using namespace ftxui;

GameplayScene::GameplayScene(engine::Engine* engine, const engine::CharacterCreationData& character_data)
    : engine::Scene(engine), character_data_(character_data), world_(100, 50, "The Town of Millhaven") {

    // Wire faction lookups before anything queues actions or runs AI.
    world_.set_faction_registry(&engine->get_faction_registry());

    initialize_map();
    initialize_player_stats();
    spawn_entities();
    add_initial_log_messages();
    setup_ui();
}

void GameplayScene::initialize_player_stats() {
    auto& registry = world_.get_registry();
    auto player_entity = world_.get_player_entity();

    // Look up race and class definitions; either may be missing if the player
    // somehow got here with empty IDs. build_initial_stats tolerates nulls.
    auto race = get_engine()->get_race_registry().get(character_data_.race_id);
    auto cls = get_engine()->get_class_registry().get(character_data_.class_id);

    engine::Stats stats = engine::build_initial_stats(
        character_data_,
        race ? &*race : nullptr,
        cls ? &*cls : nullptr
    );

    std::string growth_id = cls ? cls->growth_pattern_id : std::string{};
    registry.emplace<engine::StatsComponent>(player_entity, std::move(stats), std::move(growth_id));
    registry.emplace<engine::FactionComponent>(player_entity, "player");
    // "You" reads naturally in combat log messages.
    registry.emplace<engine::NameComponent>(player_entity, "You");
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
    auto& prop_registry = get_engine()->get_prop_registry();
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
        // Try the prop registry first: doors, candles, tables, chairs, etc.
        if (auto prop_template = prop_registry.get(spawn.type)) {
            auto prop_entity = registry.create();
            registry.emplace<engine::Position>(prop_entity, spawn.position.x, spawn.position.y);
            registry.emplace<engine::Renderable>(prop_entity,
                                                 prop_template->glyph,
                                                 prop_template->fg_color,
                                                 prop_template->bg_color,
                                                 prop_template->bold,
                                                 prop_template->render_order);
            registry.emplace<engine::NameComponent>(prop_entity, prop_template->name);

            if (prop_template->blocks_movement) {
                registry.emplace<engine::BlocksMovement>(prop_entity);
            }
            if (prop_template->blocks_vision) {
                registry.emplace<engine::BlocksVision>(prop_entity);
            }

            // Openable prop -> attach Door for the toggle action to act on.
            if (prop_template->open_glyph.has_value()) {
                registry.emplace<engine::Door>(prop_entity,
                                                prop_template->glyph,
                                                *prop_template->open_glyph);
            }
            continue;
        }

        {
            // Fall back to the mob registry for AI-driven creatures.
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

                // Faction drives AI targeting and bump-attack relationships.
                registry.emplace<engine::FactionComponent>(mob_entity, mob_template->faction_id);
                registry.emplace<engine::NameComponent>(mob_entity, mob_template->name);

                // Stats: prefer the explicit stats block from Lua; otherwise
                // synthesize from legacy hp/defense/power so old data still works.
                engine::Stats mob_stats;
                if (mob_template->base_stats.has_value()) {
                    mob_stats = *mob_template->base_stats;
                    mob_stats.calculate_max_resources();
                    mob_stats.refill_resources();
                } else {
                    // Legacy conversion. The stat formula HEALTH = CON*10 + STR*2
                    // dwarfs legacy max_hp values, so pin max HEALTH to max_hp
                    // directly while picking STR/CON to drive the combat formula
                    // (dmg = STR - target.CON/2) at roughly the old feel.
                    mob_stats.set_stat(engine::CoreStat::STRENGTH, std::max(1, 6 + mob_template->power));
                    mob_stats.set_stat(engine::CoreStat::CONSTITUTION, std::max(1, 10 + mob_template->defense));
                    mob_stats.set_stat(engine::CoreStat::DEXTERITY, 10);
                    mob_stats.max_resources[engine::ResourcePool::HEALTH] = mob_template->max_hp;
                    mob_stats.current_resources[engine::ResourcePool::HEALTH] = mob_template->max_hp;
                }
                registry.emplace<engine::StatsComponent>(mob_entity, std::move(mob_stats));
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

    auto& log = world_.get_game_log();
    log.entry()
            .color(Color::Green)
            .text("Welcome to the game, ")
            .bold()
            .text(character_data_.name)
            .reset_style()
            .color(Color::Green)
            .text("!")
            .log();

    log.entry()
            .text("You are a ")
            .text(race_name)
            .text(" ")
            .text(class_name)
            .text(" exploring the world.")
            .log();
}

void GameplayScene::setup_ui() {
    auto log_panel = engine::ui::create_log_panel(world_.get_game_log());
    auto world_panel = engine::ui::create_world_panel(world_);
    auto stats_panel = engine::ui::create_stats_panel(world_);

    // 3-pane layout: [stats | world | log]. ResizableSplitLeft anchors stats
    // on the left; ResizableSplitRight anchors the log on the right of the
    // remaining space.
    auto right_split = ResizableSplitRight(log_panel, world_panel, &log_width_);
    auto split = ResizableSplitLeft(stats_panel, right_split, &stats_width_);

    // Create a component that handles keyboard input for player movement
    component_ = CatchEvent(Renderer(split, [this, split, log_panel] {
        auto status_text = log_panel->Focused()
            ? text("TAB: Game | Arrow/J/K: Scroll | PgUp/PgDn/Home/End: Jump | Q: Quit") | dim | center
            : text("TAB: Log | Arrow/Numpad: Move | 5/.: Wait | Q: Quit") | dim | center;

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

    // Drain pending level-ups. AttackAction tags entities that crossed the
    // XP threshold but can't apply the growth pattern itself because it would
    // need a reference to the engine's GrowthPatternRegistry. The scene has
    // that access, so it does the application here.
    {
        auto& registry = world_.get_registry();
        auto& growth_registry = get_engine()->get_growth_pattern_registry();
        auto view = registry.view<engine::PendingLevelUp, engine::StatsComponent>();
        for (auto entity : view) {
            auto& tag = view.get<engine::PendingLevelUp>(entity);
            auto& stats = view.get<engine::StatsComponent>(entity);
            const bool is_player = registry.all_of<engine::Player>(entity);

            auto pattern = growth_registry.get(stats.growth_pattern_id);
            for (int i = 0; i < tag.levels; ++i) {
                stats.level_up();
                std::unordered_map<engine::CoreStat, int> increases;
                if (pattern) {
                    increases = pattern->apply_level_up(stats, world_.get_rng());
                }

                if (is_player) {
                    auto& log = world_.get_game_log();
                    log.entry()
                       .color(Color::YellowLight).bold()
                       .text("You reach level " + std::to_string(stats.level) + "!")
                       .log();
                    for (const auto& [stat, delta] : increases) {
                        if (delta == 0) continue;
                        log.entry().color(Color::YellowLight)
                           .text("  " + std::string(engine::get_stat_name(stat)) +
                                 " +" + std::to_string(delta))
                           .log();
                    }
                }
            }
            registry.remove<engine::PendingLevelUp>(entity);
        }
    }

    // If the player has died, snapshot their final stats and transition to
    // the game-over scene. The world is still alive — we just stop processing
    // it from this scene.
    if (world_.is_player_dead()) {
        auto& registry = world_.get_registry();
        auto player_entity = world_.get_player_entity();
        engine::Stats final_stats;
        if (auto* sc = registry.try_get<engine::StatsComponent>(player_entity)) {
            final_stats = static_cast<engine::Stats>(*sc);
        }
        std::string cause = world_.get_player_death_cause().value_or("an unknown foe");

        get_engine()->get_scene_manager().set_scene(
            std::make_unique<GameOverScene>(
                get_engine(),
                character_data_,
                std::move(final_stats),
                std::move(cause)));
    }
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
