#pragma once

#include <engine/scene.h>
#include <engine/character_data.h>
#include <engine/stats.h>
#include <ftxui/component/component.hpp>
#include <string>

class GameOverScene : public engine::Scene {
public:
    GameOverScene(engine::Engine* engine,
                  engine::CharacterCreationData character_data,
                  engine::Stats final_stats,
                  std::string cause_of_death);
    ~GameOverScene() override = default;

    void update() override;
    ftxui::Component get_component() override;

private:
    engine::CharacterCreationData character_data_;
    engine::Stats final_stats_;
    std::string cause_of_death_;
    ftxui::Component component_;
};
