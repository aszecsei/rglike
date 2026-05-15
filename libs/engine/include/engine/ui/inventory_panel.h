#pragma once

#include <engine/equipment.h>
#include <engine/world.h>
#include <entt/entt.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>

namespace engine::ui {

// The kind of inventory modal currently open. Drives both the rendered title
// and the prompt shown at the bottom of the modal.
enum class InventoryModalKind {
    None,            // No modal — caller should not render
    Inspect,         // 'i': show bag + equipped section, Esc closes
    DropChoice,      // 'd': show bag, pick a letter to drop
    PickupChoice,    // 'g' with multiple items on the tile: pick which to grab
    EquipChoice,     // 'w': show only equippable bag items, pick a letter
    RingSlotChoice,  // follow-up to EquipChoice when both rings are full
    UnequipChoice    // 'T': show occupied slots, pick one to remove
};

// Render the inventory modal as a bordered FTXUI element.
// - For Inspect / DropChoice / EquipChoice, slots come from the player's
//   InventoryComponent. Inspect additionally renders an "Equipped" header
//   from the player's EquipmentComponent. EquipChoice filters bag rows to
//   templates whose equip_slot is set.
// - For PickupChoice, `pickup_choices` is the list of item entities on the
//   player's tile; each is given a letter starting at 'a' for selection.
// - For UnequipChoice, each currently-occupied equipment slot is rendered
//   with a selection letter 'a'..'j' in the canonical slot order.
// - For RingSlotChoice the modal prompts the player to pick ring_1 or
//   ring_2 (responded to with the digit keys '1' / '2').
// Caller composes this on top of the world view via dbox().
[[nodiscard]] auto render_inventory_modal(
    const World& world,
    InventoryModalKind kind,
    const std::vector<entt::entity>& pickup_choices) -> ftxui::Element;

// Map an unequip-modal letter back to its EquipmentSlot. The mapping is
// the order of kAllEquipmentSlots in equipment.h, filtered to currently
// occupied slots. Returns nullopt for letters outside the live range.
[[nodiscard]] std::optional<EquipmentSlot> unequip_letter_to_slot(
    const World& world, char letter);

} // namespace engine::ui
