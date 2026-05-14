#pragma once

#include <engine/world.h>
#include <entt/entt.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>

namespace engine::ui {

// The kind of inventory modal currently open. Drives both the rendered title
// and the prompt shown at the bottom of the modal.
enum class InventoryModalKind {
    None,           // No modal — caller should not render
    Inspect,        // 'i': just show the bag, Esc closes
    DropChoice,     // 'd': show the bag with a "Drop which?" prompt
    PickupChoice    // 'g' with multiple items on the tile: pick which to grab
};

// Render the inventory modal as a bordered FTXUI element.
// - For Inspect / DropChoice, slots come from the player's InventoryComponent.
// - For PickupChoice, `pickup_choices` is the list of item entities on the
//   player's tile; each is given a letter starting at 'a' for selection.
// Caller composes this on top of the world view via dbox().
[[nodiscard]] auto render_inventory_modal(
    const World& world,
    InventoryModalKind kind,
    const std::vector<entt::entity>& pickup_choices) -> ftxui::Element;

} // namespace engine::ui
