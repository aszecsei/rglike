/**
 * @file main_menu_scene.hpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#pragma once

#include "rglike/rglike.hpp"

namespace rglike {
    class MainMenuScene : public Scene {

    public:
        explicit MainMenuScene(Game* game)
            : Scene(game) { }

        inline auto SceneName() -> std::string_view override {
            return "MainMenu";
        }
        void Render() override;
    };
} // namespace rglike