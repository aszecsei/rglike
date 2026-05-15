#pragma once

#include <engine/scene.h>
#include <engine/world.h>
#include <engine/character_data.h>
#include <engine/ui/inventory_panel.h>
#include <entt/entt.hpp>
#include <ftxui/component/component.hpp>
#include <memory>
#include <vector>

class GameplayScene : public engine::Scene {
public:
    explicit GameplayScene(engine::Engine* engine, engine::CharacterCreationData character_data);
    ~GameplayScene() override = default;

    void update() override;
    ftxui::Component get_component() override;

    void on_enter() override;
    void on_exit() override;

private:
    // Helper methods for initialization
    void initialize_map();
    void initialize_player_stats();
    void initialize_starting_loadout();
    void spawn_entities();
    void setup_ui();
    void add_initial_log_messages();

    // Inventory interaction. The modal short-circuits world-panel key handling
    // while open; the scene's CatchEvent layer dispatches the keystroke and
    // closes the modal on completion.
    void open_pickup_choice();
    bool handle_modal_event(const ftxui::Event& event);

    engine::CharacterCreationData character_data_;
    engine::World world_;
    ftxui::Component component_;
    int log_width_ = 30;
    int stats_width_ = 28;

    engine::ui::InventoryModalKind modal_kind_ = engine::ui::InventoryModalKind::None;
    std::vector<entt::entity> pickup_choices_;

    // When EquipChoice selects a ring and both ring slots are full, the
    // modal pivots to RingSlotChoice and we remember which bag letter the
    // player chose so the follow-up keystroke can re-queue EquipAction
    // with the resolved ring slot.
    char pending_equip_letter_ = 0;
};
