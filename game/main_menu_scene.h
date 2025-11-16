#pragma once

#include <engine/scene.h>
#include <ftxui/component/component.hpp>

class MainMenuScene : public engine::Scene {
public:
    explicit MainMenuScene(engine::Engine* engine);
    ~MainMenuScene() override = default;

    void update() override;
    ftxui::Component get_component() override;

private:
    void on_play();
    void on_settings();
    void on_quit();

    ftxui::Component component_;
    int selected_ = 0;
};
