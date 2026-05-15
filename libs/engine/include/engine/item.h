#pragma once

#include "constants.h"
#include "registry.h"
#include "stats.h"
#include <ftxui/screen/color.hpp>
#include <optional>
#include <string>
#include <unordered_map>

namespace engine {

// Slot KIND an item template can occupy. RING means "either ring slot",
// resolved to RING_1 or RING_2 at equip time. The matching physical
// slot enum (EquipmentSlot) and the helpers that translate between them
// live in equipment.h, which depends on this header.
enum class ItemSlotKind {
    MAIN_HAND,
    OFF_HAND,
    HEAD,
    CHEST,
    LEGS,
    BOOTS,
    GLOVES,
    AMULET,
    RING,
};

// An Item is a template for things the player can pick up, carry, drop,
// and (if equip_slot is set) wear. Equipment effects apply only while
// the item is mounted in an EquipmentComponent slot — bag-only carry has
// no gameplay effect.
//
// Stackable items merge by `id` in inventory: a single InventorySlot holds the
// template id and a count. Non-stackable items are kept as distinct entities
// in the inventory so that per-instance state (durability, modifiers) can be
// layered on later without changing the inventory shape. Equippable items
// must be non-stackable; the Lua binding rejects stackable+equippable.
struct Item {
    std::string id;
    std::string name;
    std::string glyph;
    ftxui::Color fg_color = ftxui::Color::Default;
    ftxui::Color bg_color = ftxui::Color::Default;
    bool bold = false;
    int render_order = constants::RENDER_ORDER_ITEMS;
    bool is_stackable = false;

    // Equipment metadata. equip_slot.has_value() marks the item as
    // wearable. two_handed is only meaningful when equip_slot == MAIN_HAND
    // — wearing such a weapon evicts the off-hand slot.
    std::optional<ItemSlotKind> equip_slot;
    bool two_handed = false;
    int damage_bonus = 0;
    int defense_bonus = 0;

    // Per-stat bonuses applied while the item is equipped, summed across
    // all worn equipment by equipment.h helpers and consumed by combat.
    // Resource pool maxes (HP, mana, etc.) still derive from base stats
    // for now — see the note in AttackAction.
    std::unordered_map<CoreStat, int> stat_bonuses;
};

using ItemRegistry = Registry<Item>;

} // namespace engine
