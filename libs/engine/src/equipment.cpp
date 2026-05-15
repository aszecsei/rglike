#include <engine/equipment.h>
#include <engine/components.h>

namespace engine {

std::string_view to_string(EquipmentSlot slot) {
    switch (slot) {
        case EquipmentSlot::MAIN_HAND: return "main_hand";
        case EquipmentSlot::OFF_HAND:  return "off_hand";
        case EquipmentSlot::HEAD:      return "head";
        case EquipmentSlot::CHEST:     return "chest";
        case EquipmentSlot::LEGS:      return "legs";
        case EquipmentSlot::BOOTS:     return "boots";
        case EquipmentSlot::GLOVES:    return "gloves";
        case EquipmentSlot::AMULET:    return "amulet";
        case EquipmentSlot::RING_1:    return "ring_1";
        case EquipmentSlot::RING_2:    return "ring_2";
    }
    return "?";
}

std::string_view display_name(EquipmentSlot slot) {
    switch (slot) {
        case EquipmentSlot::MAIN_HAND: return "Main hand";
        case EquipmentSlot::OFF_HAND:  return "Off hand";
        case EquipmentSlot::HEAD:      return "Head";
        case EquipmentSlot::CHEST:     return "Chest";
        case EquipmentSlot::LEGS:      return "Legs";
        case EquipmentSlot::BOOTS:     return "Boots";
        case EquipmentSlot::GLOVES:    return "Gloves";
        case EquipmentSlot::AMULET:    return "Amulet";
        case EquipmentSlot::RING_1:    return "Ring 1";
        case EquipmentSlot::RING_2:    return "Ring 2";
    }
    return "?";
}

std::string_view to_string(ItemSlotKind kind) {
    switch (kind) {
        case ItemSlotKind::MAIN_HAND: return "main_hand";
        case ItemSlotKind::OFF_HAND:  return "off_hand";
        case ItemSlotKind::HEAD:      return "head";
        case ItemSlotKind::CHEST:     return "chest";
        case ItemSlotKind::LEGS:      return "legs";
        case ItemSlotKind::BOOTS:     return "boots";
        case ItemSlotKind::GLOVES:    return "gloves";
        case ItemSlotKind::AMULET:    return "amulet";
        case ItemSlotKind::RING:      return "ring";
    }
    return "?";
}

std::optional<ItemSlotKind> parse_item_slot_kind(std::string_view s) {
    if (s == "main_hand") return ItemSlotKind::MAIN_HAND;
    if (s == "off_hand")  return ItemSlotKind::OFF_HAND;
    if (s == "head")      return ItemSlotKind::HEAD;
    if (s == "chest")     return ItemSlotKind::CHEST;
    if (s == "legs")      return ItemSlotKind::LEGS;
    if (s == "boots")     return ItemSlotKind::BOOTS;
    if (s == "gloves")    return ItemSlotKind::GLOVES;
    if (s == "amulet")    return ItemSlotKind::AMULET;
    if (s == "ring")      return ItemSlotKind::RING;
    return std::nullopt;
}

std::optional<EquipmentSlot> parse_equipment_slot(std::string_view s) {
    if (s == "main_hand") return EquipmentSlot::MAIN_HAND;
    if (s == "off_hand")  return EquipmentSlot::OFF_HAND;
    if (s == "head")      return EquipmentSlot::HEAD;
    if (s == "chest")     return EquipmentSlot::CHEST;
    if (s == "legs")      return EquipmentSlot::LEGS;
    if (s == "boots")     return EquipmentSlot::BOOTS;
    if (s == "gloves")    return EquipmentSlot::GLOVES;
    if (s == "amulet")    return EquipmentSlot::AMULET;
    if (s == "ring_1")    return EquipmentSlot::RING_1;
    if (s == "ring_2")    return EquipmentSlot::RING_2;
    return std::nullopt;
}

std::vector<EquipmentSlot> physical_slots_for(ItemSlotKind kind) {
    switch (kind) {
        case ItemSlotKind::MAIN_HAND: return {EquipmentSlot::MAIN_HAND};
        case ItemSlotKind::OFF_HAND:  return {EquipmentSlot::OFF_HAND};
        case ItemSlotKind::HEAD:      return {EquipmentSlot::HEAD};
        case ItemSlotKind::CHEST:     return {EquipmentSlot::CHEST};
        case ItemSlotKind::LEGS:      return {EquipmentSlot::LEGS};
        case ItemSlotKind::BOOTS:     return {EquipmentSlot::BOOTS};
        case ItemSlotKind::GLOVES:    return {EquipmentSlot::GLOVES};
        case ItemSlotKind::AMULET:    return {EquipmentSlot::AMULET};
        case ItemSlotKind::RING:      return {EquipmentSlot::RING_1, EquipmentSlot::RING_2};
    }
    return {};
}

namespace {

// Resolve `entity` to its Item template. Returns nullopt if the entity is
// dead, lacks an ItemComponent, or the template id is not registered.
std::optional<Item> resolve_template(const entt::registry& reg,
                                      const ItemRegistry& items,
                                      entt::entity entity) {
    if (entity == entt::null || !reg.valid(entity)) return std::nullopt;
    const auto* ic = reg.try_get<ItemComponent>(entity);
    if (!ic) return std::nullopt;
    return items.get(ic->item_id);
}

} // namespace

int sum_damage_bonus(const entt::registry& reg,
                     const EquipmentComponent& equip,
                     const ItemRegistry& items) {
    int total = 0;
    for (const auto& [_slot, entity] : equip.slots) {
        if (auto tmpl = resolve_template(reg, items, entity)) {
            total += tmpl->damage_bonus;
        }
    }
    return total;
}

int sum_defense_bonus(const entt::registry& reg,
                      const EquipmentComponent& equip,
                      const ItemRegistry& items) {
    int total = 0;
    for (const auto& [_slot, entity] : equip.slots) {
        if (auto tmpl = resolve_template(reg, items, entity)) {
            total += tmpl->defense_bonus;
        }
    }
    return total;
}

int sum_stat_bonus(const entt::registry& reg,
                   const EquipmentComponent& equip,
                   const ItemRegistry& items,
                   CoreStat stat) {
    int total = 0;
    for (const auto& [_slot, entity] : equip.slots) {
        auto tmpl = resolve_template(reg, items, entity);
        if (!tmpl) continue;
        auto it = tmpl->stat_bonuses.find(stat);
        if (it != tmpl->stat_bonuses.end()) {
            total += it->second;
        }
    }
    return total;
}

int effective_stat(const entt::registry& reg,
                   const ItemRegistry& items,
                   entt::entity actor,
                   CoreStat stat) {
    const auto* sc = reg.try_get<StatsComponent>(actor);
    int base = sc ? sc->get_stat(stat) : 0;
    if (const auto* eq = reg.try_get<EquipmentComponent>(actor)) {
        base += sum_stat_bonus(reg, *eq, items, stat);
    }
    return base;
}

int equipment_damage_bonus(const entt::registry& reg,
                           const ItemRegistry& items,
                           entt::entity actor) {
    if (const auto* eq = reg.try_get<EquipmentComponent>(actor)) {
        return sum_damage_bonus(reg, *eq, items);
    }
    return 0;
}

int equipment_defense_bonus(const entt::registry& reg,
                            const ItemRegistry& items,
                            entt::entity actor) {
    if (const auto* eq = reg.try_get<EquipmentComponent>(actor)) {
        return sum_defense_bonus(reg, *eq, items);
    }
    return 0;
}

} // namespace engine
