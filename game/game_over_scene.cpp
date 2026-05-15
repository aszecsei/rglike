#include "game_over_scene.h"
#include "main_menu_scene.h"
#include <engine/engine.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>

using namespace ftxui;

GameOverScene::GameOverScene(engine::Engine* engine,
                             engine::CharacterCreationData character_data,
                             engine::Stats final_stats,
                             std::string cause_of_death)
    : engine::Scene(engine),
      character_data_(std::move(character_data)),
      final_stats_(std::move(final_stats)),
      cause_of_death_(std::move(cause_of_death)) {

    // Resolve race + class display names for the summary.
    std::string race_name = character_data_.race_id;
    std::string class_name = character_data_.class_id;
    if (auto race = engine->get_race_registry().get(character_data_.race_id)) {
        race_name = race->name;
    }
    if (auto cls = engine->get_class_registry().get(character_data_.class_id)) {
        class_name = cls->name;
    }

    auto renderer = Renderer([this, race_name, class_name] {
        auto banner = text("YOU DIED") | bold | color(Color::Red) | center;
        auto cause = text("Killed by " + cause_of_death_) | color(Color::RedLight) | center;

        auto name_line = hbox({text("Name: ") | dim, text(character_data_.name) | bold}) | center;
        auto race_line = hbox({text("Race: ") | dim, text(race_name)}) | center;
        auto class_line = hbox({text("Class: ") | dim, text(class_name)}) | center;
        auto level_line = hbox({
            text("Level: ") | dim,
            text(std::to_string(final_stats_.level)),
            text("   XP: ") | dim,
            text(std::to_string(final_stats_.experience)),
        }) | center;

        auto footer = text("Press any key to return to the main menu") | dim | center;

        return vbox({
            text("") | flex,
            banner,
            text(""),
            cause,
            text(""),
            separator() | size(WIDTH, EQUAL, 40) | center,
            name_line,
            race_line,
            class_line,
            level_line,
            separator() | size(WIDTH, EQUAL, 40) | center,
            text("") | flex,
            footer,
            text(""),
        }) | borderDouble | center;
    });

    component_ = CatchEvent(renderer, [this](const Event& event) {
        // Any keypress returns to the main menu. Filter mouse events so
        // mouse movement doesn't immediately dismiss the screen.
        if (event.is_mouse()) return false;
        get_engine()->get_scene_manager().set_scene(
            std::make_unique<MainMenuScene>(get_engine()));
        return true;
    });
}

void GameOverScene::update() {
    // Static screen, no per-frame logic.
}

Component GameOverScene::get_component() {
    return component_;
}
