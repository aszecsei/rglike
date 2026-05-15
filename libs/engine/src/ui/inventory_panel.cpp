#include <engine/ui/inventory_panel.h>
#include <engine/components.h>
#include <engine/equipment.h>
#include <algorithm>

namespace engine::ui {
using namespace ftxui;

namespace {

// One row: "<letter> - <glyph> <name> [xN]" with the glyph rendered in its
// template color before the name so the modal is scannable.
Element slot_row(char letter,
                 const std::string& glyph,
                 Color fg,
                 bool bold_glyph,
                 const std::string& name,
                 int count,
                 bool stackable,
                 const std::string& trailing = "") {
    Element glyph_el = text(glyph);
    if (fg != Color::Default) glyph_el |= color(fg);
    if (bold_glyph) glyph_el |= bold;

    std::string suffix;
    if (stackable && count > 1) {
        suffix = " (x" + std::to_string(count) + ")";
    }
    if (!trailing.empty()) suffix += " " + trailing;

    return hbox({
        text(std::string(1, letter)) | bold,
        text(" - "),
        glyph_el,
        text(" "),
        text(name + suffix),
    });
}

// Pull the glyph / color / name fields out of a non-stackable item entity.
struct ItemDisplay {
    std::string name = "item";
    std::string glyph = "?";
    Color fg = Color::Default;
    bool bold_glyph = false;
};
ItemDisplay describe_entity(const entt::registry& registry, entt::entity e) {
    ItemDisplay d;
    if (registry.all_of<NameComponent>(e)) {
        d.name = registry.get<NameComponent>(e).name;
    }
    if (const auto* r = registry.try_get<Renderable>(e)) {
        d.glyph = r->glyph;
        d.fg = r->fg_color;
        d.bold_glyph = r->bold;
    }
    return d;
}

// Resolve the Item template of a non-stackable inventory entity. Returns
// nullopt when the entity is stale, lacks an ItemComponent, or the id
// is not in the registry. Used by the Equip modal to filter rows.
std::optional<Item> entity_template(const entt::registry& registry,
                                     const ItemRegistry* item_reg,
                                     entt::entity e) {
    if (!item_reg) return std::nullopt;
    if (!registry.valid(e)) return std::nullopt;
    const auto* ic = registry.try_get<ItemComponent>(e);
    if (!ic) return std::nullopt;
    return item_reg->get(ic->item_id);
}

// Sort a copy of an inventory's slots by letter for stable rendering.
std::vector<InventorySlot> sorted_slots(const InventoryComponent& inv) {
    std::vector<InventorySlot> sorted = inv.slots;
    std::ranges::sort(sorted,
              [](const InventorySlot& a, const InventorySlot& b) {
                  return a.letter < b.letter;
              });
    return sorted;
}

// Render one inventory slot. Stackable slots resolve display fields from
// the item registry; non-stackable slots use the entity's components.
Element render_one_slot(const World& world, const InventorySlot& slot) {
    const auto& registry = world.get_registry();
    if (slot.stack_item_id) {
        const auto* item_reg = world.get_item_registry();
        std::string name = *slot.stack_item_id;
        std::string glyph = "?";
        Color fg = Color::Default;
        bool bold_glyph = false;
        if (item_reg) {
            if (auto tmpl = item_reg->get(*slot.stack_item_id)) {
                name = tmpl->name;
                glyph = tmpl->glyph;
                fg = tmpl->fg_color;
                bold_glyph = tmpl->bold;
            }
        }
        return slot_row(slot.letter, glyph, fg, bold_glyph,
                        name, slot.count, /*stackable=*/true);
    }
    if (slot.unique_item && registry.valid(*slot.unique_item)) {
        auto d = describe_entity(registry, *slot.unique_item);
        return slot_row(slot.letter, d.glyph, d.fg, d.bold_glyph,
                        d.name, 1, /*stackable=*/false);
    }
    return text("");
}

// Bag listing: every non-empty slot.
std::vector<Element> render_inventory_rows(const World& world) {
    std::vector<Element> rows;
    const auto& registry = world.get_registry();
    const auto player = world.get_player_entity();
    const auto* inv = registry.try_get<InventoryComponent>(player);
    if (!inv || inv->slots.empty()) {
        rows.push_back(text("(empty)") | dim | center);
        return rows;
    }
    for (const auto& slot : sorted_slots(*inv)) {
        rows.push_back(render_one_slot(world, slot));
    }
    return rows;
}

// Bag listing filtered to equippable items, for the EquipChoice modal.
// Stackable slots are never equippable (the binding rejects equippable+
// stackable at load time), so we only consult unique_item handles.
std::vector<Element> render_equippable_rows(const World& world) {
    std::vector<Element> rows;
    const auto& registry = world.get_registry();
    const auto player = world.get_player_entity();
    const auto* inv = registry.try_get<InventoryComponent>(player);
    const auto* item_reg = world.get_item_registry();
    if (!inv || inv->slots.empty()) {
        rows.push_back(text("(no equippable items)") | dim | center);
        return rows;
    }
    bool any = false;
    for (const auto& slot : sorted_slots(*inv)) {
        if (!slot.unique_item) continue;
        auto tmpl = entity_template(registry, item_reg, *slot.unique_item);
        if (!tmpl || !tmpl->equip_slot) continue;
        rows.push_back(render_one_slot(world, slot));
        any = true;
    }
    if (!any) rows.push_back(text("(no equippable items)") | dim | center);
    return rows;
}

std::vector<Element> render_pickup_choice_rows(const World& world,
                                                const std::vector<entt::entity>& choices) {
    std::vector<Element> rows;
    const auto& registry = world.get_registry();
    if (choices.empty()) {
        rows.push_back(text("(nothing)") | dim | center);
        return rows;
    }
    char letter = 'a';
    for (entt::entity e : choices) {
        if (!registry.valid(e)) { ++letter; continue; }
        auto d = describe_entity(registry, e);
        int count = 1;
        bool stackable = false;
        if (const auto* ic = registry.try_get<ItemComponent>(e)) {
            count = ic->count;
            stackable = ic->is_stackable;
        }
        rows.push_back(slot_row(letter, d.glyph, d.fg, d.bold_glyph,
                                d.name, count, stackable));
        ++letter;
        if (letter > 'z') break;
    }
    return rows;
}

// Header + occupant for the "Equipped" panel shown inside Inspect.
std::vector<Element> render_equipment_rows(const World& world,
                                            bool include_empty) {
    std::vector<Element> rows;
    const auto& registry = world.get_registry();
    const auto player = world.get_player_entity();
    const auto* equip = registry.try_get<EquipmentComponent>(player);
    if (!equip) {
        if (include_empty) {
            rows.push_back(text("(no equipment slots)") | dim | center);
        }
        return rows;
    }
    for (EquipmentSlot slot : kAllEquipmentSlots) {
        auto it = equip->slots.find(slot);
        if (it == equip->slots.end()) {
            if (include_empty) {
                rows.push_back(hbox({
                    text(std::string(display_name(slot))) | dim,
                    text(": ") | dim,
                    text("(empty)") | dim,
                }));
            }
            continue;
        }
        auto d = describe_entity(registry, it->second);
        Element glyph_el = text(d.glyph);
        if (d.fg != Color::Default) glyph_el |= color(d.fg);
        if (d.bold_glyph) glyph_el |= bold;
        rows.push_back(hbox({
            text(std::string(display_name(slot))) | bold,
            text(": "),
            glyph_el,
            text(" "),
            text(d.name),
        }));
    }
    if (rows.empty() && !include_empty) {
        rows.push_back(text("(nothing equipped)") | dim | center);
    }
    return rows;
}

// Ordered list of (letter, slot, entity) for occupied slots — used by
// UnequipChoice to render rows and to translate the picked letter back
// to a slot.
struct UnequipEntry {
    char letter;
    EquipmentSlot slot;
    entt::entity entity;
};
std::vector<UnequipEntry> live_unequip_entries(const World& world) {
    std::vector<UnequipEntry> out;
    const auto& registry = world.get_registry();
    const auto player = world.get_player_entity();
    const auto* equip = registry.try_get<EquipmentComponent>(player);
    if (!equip) return out;
    char letter = 'a';
    for (EquipmentSlot slot : kAllEquipmentSlots) {
        auto it = equip->slots.find(slot);
        if (it == equip->slots.end()) continue;
        out.push_back({.letter = letter, .slot = slot, .entity = it->second});
        ++letter;
        if (letter > 'z') break;
    }
    return out;
}

std::vector<Element> render_unequip_rows(const World& world) {
    std::vector<Element> rows;
    const auto& registry = world.get_registry();
    auto entries = live_unequip_entries(world);
    if (entries.empty()) {
        rows.push_back(text("(nothing equipped)") | dim | center);
        return rows;
    }
    for (const auto& entry : entries) {
        auto d = describe_entity(registry, entry.entity);
        Element glyph_el = text(d.glyph);
        if (d.fg != Color::Default) glyph_el |= color(d.fg);
        if (d.bold_glyph) glyph_el |= bold;
        rows.push_back(hbox({
            text(std::string(1, entry.letter)) | bold,
            text(" - "),
            text(std::string(display_name(entry.slot))) | bold,
            text(": "),
            glyph_el,
            text(" "),
            text(d.name),
        }));
    }
    return rows;
}

std::vector<Element> render_ring_slot_choice_rows(const World& world) {
    std::vector<Element> rows;
    const auto& registry = world.get_registry();
    const auto player = world.get_player_entity();
    const auto* equip = registry.try_get<EquipmentComponent>(player);

    auto describe = [&](EquipmentSlot slot, char digit) {
        std::string label = std::string(display_name(slot));
        std::string occupant = "(empty)";
        if (equip) {
            auto it = equip->slots.find(slot);
            if (it != equip->slots.end()) {
                occupant = describe_entity(registry, it->second).name;
            }
        }
        return hbox({
            text(std::string(1, digit)) | bold,
            text(" - "),
            text(label) | bold,
            text(": "),
            text(occupant),
        });
    };

    rows.push_back(describe(EquipmentSlot::RING_1, '1'));
    rows.push_back(describe(EquipmentSlot::RING_2, '2'));
    return rows;
}

} // namespace

auto render_inventory_modal(
    const World& world,
    InventoryModalKind kind,
    const std::vector<entt::entity>& pickup_choices) -> ftxui::Element {
    std::string title;
    std::string prompt;
    Elements body;

    switch (kind) {
        case InventoryModalKind::Inspect: {
            title = "Inventory";
            prompt = "Esc to close";
            auto equipped_rows = render_equipment_rows(world, /*include_empty=*/true);
            body.push_back(text("Equipped") | bold);
            for (auto& r : equipped_rows) body.push_back(std::move(r));
            body.push_back(separator());
            body.push_back(text("Bag") | bold);
            for (auto& r : render_inventory_rows(world)) body.push_back(std::move(r));
            break;
        }
        case InventoryModalKind::DropChoice:
            title = "Drop which?";
            prompt = "Press a letter, or Esc to cancel";
            for (auto& r : render_inventory_rows(world)) body.push_back(std::move(r));
            break;
        case InventoryModalKind::PickupChoice:
            title = "Pick up which?";
            prompt = "Press a letter, or Esc to cancel";
            for (auto& r : render_pickup_choice_rows(world, pickup_choices)) body.push_back(std::move(r));
            break;
        case InventoryModalKind::EquipChoice:
            title = "Equip which?";
            prompt = "Press a letter, or Esc to cancel";
            for (auto& r : render_equippable_rows(world)) body.push_back(std::move(r));
            break;
        case InventoryModalKind::RingSlotChoice:
            title = "Which ring slot?";
            prompt = "Press 1 or 2, or Esc to cancel";
            for (auto& r : render_ring_slot_choice_rows(world)) body.push_back(std::move(r));
            break;
        case InventoryModalKind::UnequipChoice:
            title = "Take off which?";
            prompt = "Press a letter, or Esc to cancel";
            for (auto& r : render_unequip_rows(world)) body.push_back(std::move(r));
            break;
        case InventoryModalKind::None:
        default:
            return text("");
    }

    Elements framed;
    framed.push_back(text(title) | bold | center);
    framed.push_back(separator());
    for (auto& row : body) framed.push_back(std::move(row));
    framed.push_back(separator());
    framed.push_back(text(prompt) | dim | center);

    // clear_under wipes the area beneath the modal so the world view does not
    // bleed through. Without it dbox composites the modal as a translucent
    // overlay and the map shows behind the slot rows.
    auto modal = vbox(std::move(framed))
                 | borderDouble
                 | size(WIDTH, GREATER_THAN, 32)
                 | size(HEIGHT, GREATER_THAN, 6);
    return clear_under(modal) | center;
}

std::optional<EquipmentSlot> unequip_letter_to_slot(const World& world, char letter) {
    for (const auto& entry : live_unequip_entries(world)) {
        if (entry.letter == letter) return entry.slot;
    }
    return std::nullopt;
}

} // namespace engine::ui
