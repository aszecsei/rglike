#pragma once

#include "constants.h"
#include "registry.h"
#include <ftxui/screen/color.hpp>
#include <string>

namespace engine {

// An Item is a template for things the player can pick up, carry, and drop.
// Equipment effects, charges, and use semantics are intentionally not modeled
// here yet — for now, items exist only as bag contents.
//
// Stackable items merge by `id` in inventory: a single InventorySlot holds the
// template id and a count. Non-stackable items are kept as distinct entities
// in the inventory so that per-instance state (durability, modifiers) can be
// layered on later without changing the inventory shape.
struct Item {
    std::string id;
    std::string name;
    std::string glyph;
    ftxui::Color fg_color = ftxui::Color::Default;
    ftxui::Color bg_color = ftxui::Color::Default;
    bool bold = false;
    int render_order = constants::RENDER_ORDER_ITEMS;
    bool is_stackable = false;
};

using ItemRegistry = Registry<Item>;

} // namespace engine
