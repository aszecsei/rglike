#include <engine/ui/inventory_panel.h>
#include <engine/components.h>
#include <algorithm>

namespace engine::ui {
using namespace ftxui;

namespace {

// One row: "<letter> - <name> [xN]" with the glyph rendered in its template
// color before the name so the modal is scannable.
Element slot_row(char letter,
                 const std::string& glyph,
                 Color fg,
                 bool bold_glyph,
                 const std::string& name,
                 int count,
                 bool stackable) {
    Element glyph_el = text(glyph);
    if (fg != Color::Default) glyph_el |= color(fg);
    if (bold_glyph) glyph_el |= bold;

    std::string suffix;
    if (stackable && count > 1) {
        suffix = " (x" + std::to_string(count) + ")";
    }

    return hbox({
        text(std::string(1, letter)) | bold,
        text(" - "),
        glyph_el,
        text(" "),
        text(name + suffix),
    });
}

// Pull inventory slots from the player, sorted by letter for stable rendering.
std::vector<Element> render_inventory_rows(const World& world) {
    std::vector<Element> rows;
    const auto& registry = world.get_registry();
    const auto player = world.get_player_entity();
    const auto* inv = registry.try_get<InventoryComponent>(player);
    if (!inv || inv->slots.empty()) {
        rows.push_back(text("(empty)") | dim | center);
        return rows;
    }

    // Sort slots by letter so the modal listing is stable.
    std::vector<InventorySlot> sorted = inv->slots;
    std::sort(sorted.begin(), sorted.end(),
              [](const InventorySlot& a, const InventorySlot& b) {
                  return a.letter < b.letter;
              });

    for (const auto& slot : sorted) {
        // For stackable slots we render the template glyph & name through any
        // surviving ground entity is not available, so build the row from the
        // slot itself. Look up the item template via... we don't have it here.
        // The slot's stack_item_id is the template id; the rendering side can
        // resolve glyph by stashing it in the slot too. For now, fall back to
        // the item id as the name when no template lookup is available — this
        // path is reached only if the registry getter is missing. The normal
        // case uses the resolved name below.
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
            rows.push_back(slot_row(slot.letter, glyph, fg, bold_glyph,
                                    name, slot.count, /*stackable=*/true));
        } else if (slot.unique_item && registry.valid(*slot.unique_item)) {
            entt::entity e = *slot.unique_item;
            std::string name = registry.all_of<NameComponent>(e)
                               ? registry.get<NameComponent>(e).name
                               : std::string{"item"};
            std::string glyph = "?";
            Color fg = Color::Default;
            bool bold_glyph = false;
            if (const auto* r = registry.try_get<Renderable>(e)) {
                glyph = r->glyph;
                fg = r->fg_color;
                bold_glyph = r->bold;
            }
            rows.push_back(slot_row(slot.letter, glyph, fg, bold_glyph,
                                    name, 1, /*stackable=*/false));
        }
    }
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
        std::string name = registry.all_of<NameComponent>(e)
                           ? registry.get<NameComponent>(e).name
                           : std::string{"item"};
        std::string glyph = "?";
        Color fg = Color::Default;
        bool bold_glyph = false;
        if (const auto* r = registry.try_get<Renderable>(e)) {
            glyph = r->glyph;
            fg = r->fg_color;
            bold_glyph = r->bold;
        }
        int count = 1;
        bool stackable = false;
        if (const auto* ic = registry.try_get<ItemComponent>(e)) {
            count = ic->count;
            stackable = ic->is_stackable;
        }
        rows.push_back(slot_row(letter, glyph, fg, bold_glyph,
                                name, count, stackable));
        ++letter;
        if (letter > 'z') break;
    }
    return rows;
}

} // namespace

auto render_inventory_modal(
    const World& world,
    InventoryModalKind kind,
    const std::vector<entt::entity>& pickup_choices) -> ftxui::Element {
    std::string title;
    std::string prompt;
    std::vector<Element> rows;

    switch (kind) {
        case InventoryModalKind::Inspect:
            title = "Inventory";
            prompt = "Esc to close";
            rows = render_inventory_rows(world);
            break;
        case InventoryModalKind::DropChoice:
            title = "Drop which?";
            prompt = "Press a letter, or Esc to cancel";
            rows = render_inventory_rows(world);
            break;
        case InventoryModalKind::PickupChoice:
            title = "Pick up which?";
            prompt = "Press a letter, or Esc to cancel";
            rows = render_pickup_choice_rows(world, pickup_choices);
            break;
        case InventoryModalKind::None:
        default:
            return text("");
    }

    Elements body;
    body.push_back(text(title) | bold | center);
    body.push_back(separator());
    for (auto& row : rows) body.push_back(std::move(row));
    body.push_back(separator());
    body.push_back(text(prompt) | dim | center);

    // clear_under wipes the area beneath the modal so the world view does not
    // bleed through. Without it dbox composites the modal as a translucent
    // overlay and the map shows behind the slot rows.
    auto modal = vbox(std::move(body))
                 | borderDouble
                 | size(WIDTH, GREATER_THAN, 32)
                 | size(HEIGHT, GREATER_THAN, 6);
    return clear_under(modal) | center;
}

} // namespace engine::ui
