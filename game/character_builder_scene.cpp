#include "character_builder_scene.h"
#include "gameplay_scene.h"
#include <engine/engine.h>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <sstream>

using namespace ftxui;

CharacterBuilderScene::CharacterBuilderScene(engine::Engine* engine)
    : Scene(engine) {
    setup_ui();
}

void CharacterBuilderScene::on_enter() {
    // Load races and classes from registries
    auto& race_registry = get_engine()->get_race_registry();
    auto& class_registry = get_engine()->get_class_registry();

    race_ids_ = race_registry.get_all_ids();
    for (const auto& id : race_ids_) {
        if (auto race = race_registry.get(id)) { race_names_.push_back(race->name); }
    }

    class_ids_ = class_registry.get_all_ids();
    for (const auto& id : class_ids_) {
        if (auto char_class = class_registry.get(id)) { class_names_.push_back(char_class->name); }
    }
}

void CharacterBuilderScene::on_exit() {
    // Cleanup if needed
}

void CharacterBuilderScene::update() {
    // No per-frame logic needed
}

void CharacterBuilderScene::setup_ui() {
    // Name input
    name_input_ = Input(&name_text_, "Enter your name...");

    // Wrap name input in a panel with focus tracking
    name_panel_ = Container::Vertical({name_input_});
    name_panel_ = CatchEvent(name_panel_, [this](Event event) {
        if (name_panel_->Focused()) { focused_panel_ = FocusedPanel::NAME; }
        return false;
    });

    // Gender selector
    gender_selector_ = Radiobox(&gender_options_, &gender_index_);
    gender_panel_ = Container::Vertical({gender_selector_});
    gender_panel_ = CatchEvent(gender_panel_, [this](Event event) {
        if (gender_panel_->Focused()) { focused_panel_ = FocusedPanel::GENDER; }
        return false;
    });

    // Race selector
    race_selector_ = Radiobox(&race_names_, &race_index_);
    race_panel_ = Container::Vertical({race_selector_});
    race_panel_ = CatchEvent(race_panel_, [this](Event event) {
        if (race_panel_->Focused()) { focused_panel_ = FocusedPanel::RACE; }
        return false;
    });

    // Class selector
    class_selector_ = Radiobox(&class_names_, &class_index_);
    class_panel_ = Container::Vertical({class_selector_});
    class_panel_ = CatchEvent(class_panel_, [this](Event event) {
        if (class_panel_->Focused()) { focused_panel_ = FocusedPanel::CLASS; }
        return false;
    });

    // Finish button
    finish_button_ = Button("Start Game", [this] {
        finish_character_creation();
    });

    // Create scrollable info panel
    info_panel_ = Renderer([] {
        return text("");
    });

    // Main container with all panels
    main_container_ = Container::Horizontal(
        {Container::Vertical({name_panel_, gender_panel_, race_panel_, class_panel_, finish_button_}
         ),
         info_panel_}
    );
}

Component CharacterBuilderScene::get_component() {
    return Renderer(main_container_, [this] {
        // Title
        auto title = text("Character Creation") | bold | center;

        // Render each panel with title and border
        auto name_section = vbox({text("Name") | bold, separator(), name_input_->Render()}) |
                            border |
                            (name_panel_->Focused() ? color(Color::Cyan) : color(Color::White));

        auto gender_section =
            vbox({text("Gender") | bold, separator(), gender_selector_->Render()}) | border |
            (gender_panel_->Focused() ? color(Color::Cyan) : color(Color::White));

        Element race_section;
        if (race_names_.empty()) {
            race_section = vbox(
                               {text("Race") | bold, separator(),
                                text("No races available") | color(Color::Red)}
                           ) |
                           border;
        } else {
            race_section = vbox({text("Race") | bold, separator(), race_selector_->Render()}) |
                           border |
                           (race_panel_->Focused() ? color(Color::Cyan) : color(Color::White));
        }

        Element class_section;
        if (class_names_.empty()) {
            class_section = vbox(
                                {text("Class") | bold, separator(),
                                 text("No classes available") | color(Color::Red)}
                            ) |
                            border;
        } else {
            class_section = vbox({text("Class") | bold, separator(), class_selector_->Render()}) |
                            border |
                            (class_panel_->Focused() ? color(Color::Cyan) : color(Color::White));
        }

        // Grid layout: 2x2 grid with equal sizing
        auto top_row = hbox({
                           name_section | flex,
                           separator(),
                           gender_section | flex,
                       }) |
                       flex;

        auto bottom_row = hbox({
                              race_section | flex,
                              separator(),
                              class_section | flex,
                          }) |
                          flex;

        auto input_panels =
            vbox({top_row, separator(), bottom_row, separator(), finish_button_->Render() | center}
            );

        // Info panel on the right
        std::string info_text = get_info_text_for_panel(focused_panel_);

        // Split info text by newlines and create elements
        std::vector<Element> info_lines;
        std::stringstream ss(info_text);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.empty()) {
                info_lines.push_back(paragraph(""));
            } else {
                info_lines.push_back(paragraph(line));
            }
        }

        auto info_section = vbox(
                                {text("Information") | bold | underlined | center, separator(),
                                 vbox(info_lines) | vscroll_indicator | yframe}
                            ) |
                            border;

        // Split layout: inputs on left, info on right
        auto content =
            hbox({input_panels | flex, separator(), info_section | size(WIDTH, EQUAL, 50)});

        return vbox({title, separator(), content | flex}) | border;
    });
}

std::string CharacterBuilderScene::get_info_text_for_panel(FocusedPanel panel) {
    switch (panel) {
    case FocusedPanel::NAME:
        return "Choose a name for your character. This will be displayed in the game world.";

    case FocusedPanel::GENDER:
        return "Select your character's gender. This is primarily for roleplay purposes and does "
               "not affect gameplay mechanics.";

    case FocusedPanel::RACE: {
        if (race_names_.empty() || race_index_ >= race_ids_.size()) {
            return "Select a race to see its description and stat modifiers.";
        }

        auto& registry = get_engine()->get_race_registry();
        if (auto race = registry.get(race_ids_[race_index_])) {
            std::string info = race->description + "\n\n";
            info += "Stat Modifiers:\n";

            for (const auto& [stat, modifier] : race->stat_modifiers) {
                if (modifier != 0) {
                    info += "  " + std::string(engine::get_stat_name(stat)) + ": ";
                    if (modifier > 0) info += "+";
                    info += std::to_string(modifier) + "\n";
                }
            }

            return info;
        }
        return "No race information available.";
    }

    case FocusedPanel::CLASS: {
        if (class_names_.empty() || class_index_ >= class_ids_.size()) {
            return "Select a class to see its description and growth rates.";
        }

        auto& class_registry = get_engine()->get_class_registry();
        auto& growth_registry = get_engine()->get_growth_pattern_registry();

        if (auto char_class = class_registry.get(class_ids_[class_index_])) {
            std::string info = char_class->description + "\n\n";

            // Show starting stat bonuses
            if (!char_class->starting_stats.empty()) {
                info += "Starting Bonuses:\n";
                for (const auto& [stat, bonus] : char_class->starting_stats) {
                    if (bonus != 0) {
                        info += "  " + std::string(engine::get_stat_name(stat)) + ": +";
                        info += std::to_string(bonus) + "\n";
                    }
                }
                info += "\n";
            }

            // Show growth rates
            if (!char_class->growth_pattern_id.empty()) {
                if (auto pattern = growth_registry.get(char_class->growth_pattern_id)) {
                    info += "Growth Rates (chance to increase on level up):\n";
                    for (const auto& [stat, rate] : pattern->growth_rates) {
                        int percentage = static_cast<int>(rate * 100);
                        info += "  " + std::string(engine::get_stat_name(stat)) + ": ";
                        info += std::to_string(percentage) + "%\n";
                    }
                }
            }

            return info;
        }
        return "No class information available.";
    }
    }

    return "";
}

void CharacterBuilderScene::update_focused_panel() {
    // Update focused panel based on which component is focused
    if (name_panel_->Focused()) {
        focused_panel_ = FocusedPanel::NAME;
    } else if (gender_panel_->Focused()) {
        focused_panel_ = FocusedPanel::GENDER;
    } else if (race_panel_->Focused()) {
        focused_panel_ = FocusedPanel::RACE;
    } else if (class_panel_->Focused()) {
        focused_panel_ = FocusedPanel::CLASS;
    }
}

void CharacterBuilderScene::finish_character_creation() {
    // Validate all fields
    if (name_text_.empty()) {
        get_engine()->get_logger()->warn("Name cannot be empty");
        return;
    }

    if (race_index_ >= race_ids_.size()) {
        get_engine()->get_logger()->warn("Must select a valid race");
        return;
    }

    if (class_index_ >= class_ids_.size()) {
        get_engine()->get_logger()->warn("Must select a valid class");
        return;
    }

    // Populate character data
    character_data_.name = name_text_;
    character_data_.gender = gender_options_[gender_index_];
    std::transform(
        character_data_.gender.begin(), character_data_.gender.end(),
        character_data_.gender.begin(), ::tolower
    );
    character_data_.race_id = race_ids_[race_index_];
    character_data_.class_id = class_ids_[class_index_];

    // Log character creation
    get_engine()->get_logger()->info(
        "Character created: {} ({}) - Race: {}, Class: {}", character_data_.name,
        character_data_.gender, race_names_[race_index_], class_names_[class_index_]
    );

    // Transition to gameplay scene with character data
    get_engine()->get_scene_manager().set_scene(
        std::make_unique<GameplayScene>(get_engine(), character_data_)
    );
}