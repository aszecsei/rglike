#pragma once

#include <engine/scene.h>
#include <engine/world.h>
#include <engine/game_log.h>
#include <engine/character_data.h>
#include <ftxui/component/component.hpp>
#include <memory>

class GameplayScene : public engine::Scene {
public:
    explicit GameplayScene(engine::Engine* engine, const engine::CharacterCreationData& character_data);
    ~GameplayScene() override = default;

    void update() override;
    ftxui::Component get_component() override;

    void on_enter() override;
    void on_exit() override;

private:
    // Helper methods for initialization
    void initialize_map();
    void spawn_entities();
    void setup_ui();
    void add_initial_log_messages();

    engine::CharacterCreationData character_data_;
    engine::World world_;
    engine::GameLog game_log_;
    ftxui::Component component_;
    int log_width_ = 30;
};
