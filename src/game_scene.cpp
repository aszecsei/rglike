/**
 * @file game_scene.cpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#include "game_scene.hpp"

#include "components/log_component.hpp"
#include "constants.hpp"
#include "game_log.hpp"
#include "main_menu_scene.hpp"

#include <fmt/core.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

namespace rglike {
    void GameScene::Render() {
        int left_size = LEFT_SIDEBAR_WIDTH;
        int right_size = RIGHT_SIDEBAR_WIDTH;
        int bottom_size = LOG_HEIGHT;

        auto screen = ftxui::ScreenInteractive::Fullscreen();

        auto log = components::GameLogViewer(GameLog::GetInstance());
        GameLog::GetInstance()
            .Entry()
            .Text("Welcome to")
            .Color(ftxui::Color::Red)
            .Blink(true)
            .Text("rglike")
            .Log();

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

            return m_world.Render(world_width, world_height);
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
                ChangeScene<MainMenuScene>();
                screen.ExitLoopClosure()();
                return true;
            }

            if (event == ftxui::Event::Character(' ')) {
                auto dim = ftxui::Terminal::Size();
                auto world_width = dim.dimx - (5 + left_size + right_size);
                auto world_height = dim.dimy - (3 + bottom_size);
                GameLog::GetInstance().Log(
                    fmt::format("World view size: ({}, {})", world_width, world_height)
                );
            }

            if (event == ftxui::Event::ArrowUp) {
                m_world.MovePlayerY(-1);
                return true;
            }

            if (event == ftxui::Event::ArrowRight) {
                m_world.MovePlayerX(1);
                return true;
            }

            if (event == ftxui::Event::ArrowDown) {
                m_world.MovePlayerY(1);
                return true;
            }

            if (event == ftxui::Event::ArrowLeft) {
                m_world.MovePlayerX(-1);
                return true;
            }

            return false;
        });
        screen.Loop(component);
    }
} // namespace rglike