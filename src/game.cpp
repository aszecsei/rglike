/**
 * @file game.cpp
 * @author Alic Szecsei
 * @date 12/15/2022
 */

#include "rglike/game.hpp"

#include "rglike/constants.hpp"
#include "rglike/game_log.hpp"

#include <fmt/core.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <spdlog/spdlog.h>

namespace rglike {
    void Game::initialize() { m_world.initialize(); }

    void Game::run() {
        spdlog::info("Running");

        int left_size = LEFT_SIDEBAR_WIDTH;
        int right_size = RIGHT_SIDEBAR_WIDTH;
        int bottom_size = LOG_HEIGHT;

        auto screen = ftxui::ScreenInteractive::Fullscreen();

        auto log = Make<GameLog>();
        log->entry().text("Welcome to").color(Color::Red).blink(true).text("rglike").log();

        auto left = ftxui::Renderer([] {
            return ftxui::text("left") | ftxui::center;
        });
        auto right = ftxui::Renderer([] {
            return ftxui::text("right") | ftxui::center;
        });

        auto world = ftxui::Renderer([&] {
            auto dim = ftxui::Terminal::Size();

            auto world_width = dim.dimx - (5 + left_size + right_size);
            auto world_height = dim.dimy - (3 + bottom_size);

            return m_world.render(world_width, world_height);
        });

        auto container = world;
        container = ftxui::ResizableSplitLeft(left, container, &left_size);
        container = ftxui::ResizableSplitBottom(log, container, &bottom_size);
        container = ftxui::ResizableSplitRight(right, container, &right_size);

        auto renderer = ftxui::Renderer(container, [&] {
            return container->Render() | ftxui::borderHeavy;
        });

        auto component = CatchEvent(renderer, [&](const ftxui::Event& event) {
            if (event == ftxui::Event::Character('q')) {
                screen.ExitLoopClosure()();
                return true;
            }

            if (event == ftxui::Event::Character(' ')) {
                auto dim = ftxui::Terminal::Size();
                auto world_width = dim.dimx - (5 + left_size + right_size);
                auto world_height = dim.dimy - (3 + bottom_size);
                log->log(fmt::format("World view size: ({}, {})", world_width, world_height));
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
