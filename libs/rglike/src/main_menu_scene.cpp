/**
 * @file main_menu_scene.cpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#include "main_menu_scene.hpp"

#include "game_scene.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

namespace rglike {
    void MainMenuScene::Render() {
        auto screen = ftxui::ScreenInteractive::Fullscreen();
        auto exit = screen.ExitLoopClosure();

        auto play_game = [=] {
            ChangeScene<GameScene>();
            exit();
        };
        auto quit_game = [=] {
            exit();
        };

        auto renderer =
            ftxui::Container::Vertical({
                ftxui::Renderer([] {
                    return ftxui::text("rglike") | ftxui::color(ftxui::Color::Red) | ftxui::hcenter;
                }),
                ftxui::Button("Play Game", play_game),
                ftxui::Button("Quit", quit_game),
            }) |
            ftxui::center;

        screen.Loop(renderer);
    }
} // namespace rglike
