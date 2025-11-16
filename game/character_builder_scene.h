#pragma once

#include <engine/scene.h>
#include <engine/character_data.h>
#include <ftxui/component/component.hpp>
#include <string>
#include <vector>

class CharacterBuilderScene : public engine::Scene {
public:
    explicit CharacterBuilderScene(engine::Engine* engine);
    ~CharacterBuilderScene() override = default;

    void on_enter() override;
    void on_exit() override;
    void update() override;
    ftxui::Component get_component() override;

private:
    enum class FocusedPanel {
        NAME,
        GENDER,
        RACE,
        CLASS
    };

    FocusedPanel focused_panel_ = FocusedPanel::NAME;
    engine::CharacterCreationData character_data_;

    // UI Components
    ftxui::Component main_container_;
    ftxui::Component name_panel_;
    ftxui::Component gender_panel_;
    ftxui::Component race_panel_;
    ftxui::Component class_panel_;
    ftxui::Component info_panel_;
    ftxui::Component name_input_;
    ftxui::Component gender_selector_;
    ftxui::Component race_selector_;
    ftxui::Component class_selector_;
    ftxui::Component finish_button_;

    // Selection state
    std::string name_text_ = "";
    int gender_index_ = 0;
    int race_index_ = 0;
    int class_index_ = 0;

    std::vector<std::string> gender_options_ = {"Male", "Female", "Neutral"};
    std::vector<std::string> race_ids_;
    std::vector<std::string> race_names_;
    std::vector<std::string> class_ids_;
    std::vector<std::string> class_names_;

    // Helper methods
    void setup_ui();
    void update_focused_panel();
    void finish_character_creation();
    std::string get_info_text_for_panel(FocusedPanel panel);
};