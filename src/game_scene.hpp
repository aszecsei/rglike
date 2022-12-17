/**
 * @file game_scene.hpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#pragma once

#include "rglike/rglike.hpp"
#include "world.hpp"

namespace rglike {
    class GameScene : public Scene {
    private:
        World m_world;

    public:
        explicit GameScene(Game* game)
            : Scene(game) { }

        inline auto SceneName() -> std::string_view override {
            return "GameScene";
        }
        void Render() override;
    };
} // namespace rglike