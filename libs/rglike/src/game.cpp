/**
 * @file game.cpp
 * @author Alic Szecsei
 * @date 12/15/2022
 */

#include "rglike/game.hpp"
#include "main_menu_scene.hpp"
#include <spdlog/spdlog.h>

namespace rglike {
    void Game::Initialize() {
        spdlog::info("Initializing");
        m_current_scene = std::make_unique<MainMenuScene>(this);
    }

    void Game::Run() {
        while (true) {
            spdlog::info("Running scene '{}'", m_current_scene->SceneName());
            m_current_scene->Initialize();
            m_current_scene->Render();

            if (m_next_scene != nullptr) {
                m_current_scene = std::move(m_next_scene);
                m_next_scene = nullptr;
            } else {
                break;
            }
        }
    }
} // namespace rglike
