/**
 * @file game_scene.cpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#include "game_scene.hpp"

#include "constants.hpp"
#include "game_log.hpp"
#include "main_menu_scene.hpp"
#include "ui/log_component.hpp"
#include "ui/world_component.hpp"

#include <fmt/core.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

namespace rglike {
    void GameScene::Initialize() { m_world.Initialize(); }

    void GameScene::Render() {
        int left_size = LEFT_SIDEBAR_WIDTH;
        int right_size = RIGHT_SIDEBAR_WIDTH;
        int bottom_size = LOG_HEIGHT;

        auto screen = ftxui::ScreenInteractive::Fullscreen();

        auto log = ui::GameLogViewer(GameLog::GetInstance());
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

        auto world = ui::WorldViewer(m_world);

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

            if (event == ftxui::Event::Escape) {
                world->TakeFocus();
                return true;
            }

            if (event == ftxui::Event::Character('l')) {
                log->TakeFocus();
                return true;
            }

            return false;
        });
        screen.Loop(component);
    }
} // namespace rglike