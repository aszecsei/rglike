#include "rglike/lib.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <spdlog/spdlog.h>

namespace rglike {
    void Game::initialize() {
        spdlog::info("Initializing...");
        m_world.initialize();
    }

    void Game::run() {
        spdlog::info("Running...");

        auto screen = ftxui::ScreenInteractive::FitComponent();
        auto renderer = ftxui::Renderer([this] {
            return border(m_world.render());
        });
        auto component = CatchEvent(renderer, [&](const ftxui::Event& event) {
            if (event == ftxui::Event::Character('q')) {
                screen.ExitLoopClosure()();
                return true;
            }

            if (event == ftxui::Event::ArrowUp) {
                m_world.move_player_y(-1);
                return true;
            }

            if (event == ftxui::Event::ArrowRight) {
                m_world.move_player_x(1);
                return true;
            }

            if (event == ftxui::Event::ArrowDown) {
                m_world.move_player_y(1);
                return true;
            }

            if (event == ftxui::Event::ArrowLeft) {
                m_world.move_player_x(-1);
                return true;
            }

            return false;
        });
        screen.Loop(component);
    }
} // namespace rglike
