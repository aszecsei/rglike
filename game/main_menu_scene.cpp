#include "main_menu_scene.h"
#include "character_builder_scene.h"
#include <engine/engine.h>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

MainMenuScene::MainMenuScene(engine::Engine* engine) : Scene(engine) {
    auto play_button = Button("Play", [this] { on_play(); });
    auto settings_button = Button("Settings", [this] { on_settings(); });
    auto quit_button = Button("Quit", [this] { on_quit(); });

    auto menu_container = Container::Vertical({
        play_button,
        settings_button,
        quit_button
    }, &selected_);

    // Wrap in a renderer to add title and styling
    component_ = Renderer(menu_container, [this, menu_container] {
        auto title = text("ROGUELIKE") | bold | center;
        auto subtitle = text("A Terminal Roguelike Adventure") | dim | center;

        auto menu = menu_container->Render() | center;

        return vbox({
            text("") | flex,
            title,
            subtitle,
            text(""),
            menu,
            text("") | flex,
        }) | borderDouble | center;
    });
}

void MainMenuScene::update() {
    // Turn-based, no continuous updates needed
}

Component MainMenuScene::get_component() {
    return component_;
}

void MainMenuScene::on_play() {
    get_engine()->get_logger()->info("Play button clicked - starting character creation");
    get_engine()->get_scene_manager().set_scene(std::make_unique<CharacterBuilderScene>(get_engine()));
}

void MainMenuScene::on_settings() {
    get_engine()->get_logger()->info("Settings button clicked");
    // TODO: Push settings scene when implemented
}

void MainMenuScene::on_quit() {
    get_engine()->get_logger()->info("Quit button clicked - shutting down");
    get_engine()->shutdown();
}
