#pragma once

#include "item.h"
#include "stats.h"
#include <array>
#include <entt/entt.hpp>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace engine {

// Physical equipment slots an actor wears. RING_1 / RING_2 are distinct
// because the equipment map needs to hold two rings simultaneously; the
// item template only declares the slot KIND (RING), not which physical
// slot it occupies. ItemSlotKind lives in item.h alongside the Item
// struct that uses it.
enum class EquipmentSlot {
    MAIN_HAND,
    OFF_HAND,
    HEAD,
    CHEST,
    LEGS,
    BOOTS,
    GLOVES,
    AMULET,
    RING_1,
    RING_2,
};

// All physical slots, in display order. Used by the unequip modal to
// render a stable list.
inline constexpr std::array<EquipmentSlot, 10> kAllEquipmentSlots = {
    EquipmentSlot::MAIN_HAND,
    EquipmentSlot::OFF_HAND,
    EquipmentSlot::HEAD,
    EquipmentSlot::CHEST,
    EquipmentSlot::LEGS,
    EquipmentSlot::BOOTS,
    EquipmentSlot::GLOVES,
    EquipmentSlot::AMULET,
    EquipmentSlot::RING_1,
    EquipmentSlot::RING_2,
};

[[nodiscard]] std::string_view to_string(EquipmentSlot slot);
[[nodiscard]] std::string_view display_name(EquipmentSlot slot);
[[nodiscard]] std::string_view to_string(ItemSlotKind kind);

// Parse a Lua-side string (e.g. "main_hand") into an ItemSlotKind. Returns
// nullopt for unknown values — callers should log and reject the data file.
[[nodiscard]] std::optional<ItemSlotKind> parse_item_slot_kind(std::string_view s);

// Parse a Lua-side string (e.g. "ring_1") into a physical EquipmentSlot.
// Used by starting_loadout where slots are explicit (ring_1 vs ring_2).
[[nodiscard]] std::optional<EquipmentSlot> parse_equipment_slot(std::string_view s);

// Returns the candidate physical slots an item kind can occupy. RING
// returns {RING_1, RING_2}; everything else returns a single slot.
[[nodiscard]] std::vector<EquipmentSlot> physical_slots_for(ItemSlotKind kind);

// Held equipment. Map keys are physical slots; missing key = empty slot.
// An entity in `slots` is expected to also have Carried and Equipped
// components attached.
struct EquipmentComponent {
    std::unordered_map<EquipmentSlot, entt::entity> slots;
};

// Refinement of Carried: this item entity is owned AND mounted in a
// specific slot of `owner`'s EquipmentComponent.
struct Equipped {
    entt::entity owner = entt::null;
    EquipmentSlot slot = EquipmentSlot::MAIN_HAND;
};

// Aggregations used by combat and stat readers. Walk every slot in
// `equip`, resolve the item entity's template via `ItemRegistry`, and
// accumulate the requested field. Missing/invalid entries are skipped.
[[nodiscard]] int sum_damage_bonus(const entt::registry& reg,
                                    const EquipmentComponent& equip,
                                    const ItemRegistry& items);

[[nodiscard]] int sum_defense_bonus(const entt::registry& reg,
                                     const EquipmentComponent& equip,
                                     const ItemRegistry& items);

[[nodiscard]] int sum_stat_bonus(const entt::registry& reg,
                                  const EquipmentComponent& equip,
                                  const ItemRegistry& items,
                                  CoreStat stat);

// Convenience: return effective core-stat value for an entity, summing
// its StatsComponent base value and any equipped stat_bonuses for the
// stat. Falls back to 0 if the entity lacks a StatsComponent.
[[nodiscard]] int effective_stat(const entt::registry& reg,
                                  const ItemRegistry& items,
                                  entt::entity actor,
                                  CoreStat stat);

// Convenience: weapon/armor totals attached to an entity. Return 0 if
// the actor has no EquipmentComponent.
[[nodiscard]] int equipment_damage_bonus(const entt::registry& reg,
                                          const ItemRegistry& items,
                                          entt::entity actor);

[[nodiscard]] int equipment_defense_bonus(const entt::registry& reg,
                                           const ItemRegistry& items,
                                           entt::entity actor);

} // namespace engine
