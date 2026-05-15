#pragma once

#include "map.h"
#include "stats.h"
#include <entt/entt.hpp>
#include <ftxui/screen/color.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

// Faction response type - how one faction responds to another
enum class FactionResponse {
    IGNORE,  // Ignores entities of that faction
    ATTACK,  // Hostile - will attack on sight
    FLEE     // Will flee from entities of that faction
};

// Tag component to mark the player entity
struct Player {};

// Display name for log messages. The player gets "You"; mobs get their
// template name (e.g., "Goblin"). Optional — actions fall back to a generic
// "something" if missing.
struct NameComponent {
    std::string name;
    NameComponent() = default;
    explicit NameComponent(std::string n) : name(std::move(n)) {}
};

// Position component for entities
struct Position {
    int x;
    int y;
};

// Renderable component - any entity with this component will be rendered in the world panel
// Entities at the same position are drawn by render_order (higher = drawn on top)
struct Renderable {
    std::string glyph;
    ftxui::Color fg_color;
    ftxui::Color bg_color;
    bool bold;
    int render_order;  // Higher values drawn on top (e.g., player=100, items=50, corpses=10)

    Renderable(std::string glyph,
               ftxui::Color fg = ftxui::Color::Default,
               ftxui::Color bg = ftxui::Color::Default,
               bool bold = false,
               int render_order = 0)
        : glyph(std::move(glyph)), fg_color(fg), bg_color(bg), bold(bold), render_order(render_order) {}
};

// Map component - stores the entire game map
// This should be attached to a single map entity
struct MapComponent {
    Map map;
    std::string name;

    MapComponent(int width, int height, int depth, std::string map_name)
        : map(width, height, depth), name(std::move(map_name)) {}
};

// Camera component for viewport tracking
struct Camera {
    int x = 0;
    int y = 0;
};

// Vision component - entities with this can see (and reveal fog-of-war)
struct VisionComponent {
    int range;  // How far the entity can see

    explicit VisionComponent(int range = 8) : range(range) {}
};

// BlocksMovement component - entities with this block movement
struct BlocksMovement {};

// BlocksVision component - entities with this block line of sight
struct BlocksVision {};

// Door component - runtime state for an openable prop. The glyphs come from
// the Prop template at spawn time; OpenDoorAction/CloseDoorAction flip the
// Renderable's glyph between closed_glyph and open_glyph.
struct Door {
    bool is_open = false;
    std::string open_glyph;
    std::string closed_glyph;

    Door() = default;
    Door(std::string closed, std::string open)
        : open_glyph(std::move(open)), closed_glyph(std::move(closed)) {}
};

// ActionCooldown component - time-based action system
// Entities with cooldown 0 can take actions. After acting, cooldown is increased.
// When all active entities have cooldown > 0, time advances by the minimum cooldown.
struct ActionCooldown {
    int cooldown = 0;  // Current cooldown (0 = ready to act)

    explicit ActionCooldown(int initial_cooldown = 0) : cooldown(initial_cooldown) {}
};

// Faction component - represents which faction an entity belongs to
// Allows querying how this entity should respond to other factions
struct FactionComponent {
    std::string faction_id;  // ID of the faction this entity belongs to

    explicit FactionComponent(std::string faction_id = "neutral")
        : faction_id(std::move(faction_id)) {}
};

// Stats component - attached to entities with stats (player, NPCs, mobs)
// Note: Stats struct is defined in stats.h
struct StatsComponent : public Stats {
    std::string growth_pattern_id = "";  // Optional: ID of growth pattern for level ups

    StatsComponent() = default;
    StatsComponent(Stats stats, std::string growth_id = "")
        : Stats(std::move(stats)), growth_pattern_id(std::move(growth_id)) {}
};

// Tag component: entity has died this frame. Scenes may use this to drive
// death animations or delayed cleanup; the engine itself destroys mobs
// immediately on death and only sets a flag on World for the player.
struct Dead {};

// Tag component: entity crossed an XP threshold and is awaiting growth-pattern
// application. The scene drains these each frame because applying the growth
// pattern requires access to the engine's GrowthPatternRegistry, which World
// is intentionally not coupled to.
struct PendingLevelUp {
    int levels = 1;
};

// Combat stats component - simple combat attributes for entities
// Used alongside or instead of full StatsComponent for simpler entities
struct CombatStats {
    int defense = 0;   // Armor class / defense value
    int power = 1;     // Attack power / damage

    CombatStats() = default;
    CombatStats(int defense, int power) : defense(defense), power(power) {}
};

// Marks an entity as an item (on the ground or carried). The `item_id` links
// back to the Item template so stack merging, drop respawning, and modal
// rendering can look up the canonical name/glyph without duplicating them.
// `count` is meaningful only when `is_stackable` is true; for non-stackable
// items it is always 1.
struct ItemComponent {
    std::string item_id;
    bool is_stackable = false;
    int count = 1;
};

// Tag attached to a non-stackable item entity while it is held in an
// InventoryComponent. The item entity keeps Renderable/NameComponent/Item
// for display purposes but loses Position so it does not render in the world.
struct Carried {
    entt::entity owner = entt::null;
};

// One slot of an InventoryComponent. Exactly one of the two payloads is set:
//   - unique_item present  → non-stackable: holds an entt::entity handle.
//                            count is always 1.
//   - stack_item_id present → stackable: holds the template id and `count`.
// `letter` is assigned at pickup ('a'..'z') and stays stable until the slot
// empties, so the player's mental model of "slot a is the potion" survives
// drops of other slots.
struct InventorySlot {
    char letter = 'a';
    std::optional<entt::entity> unique_item;
    std::optional<std::string> stack_item_id;
    int count = 0;
};

// A bag. Slots are unordered; the renderer sorts by `letter`. The 26-slot cap
// is enforced by PickupAction; the vector itself is unbounded.
struct InventoryComponent {
    std::vector<InventorySlot> slots;
    static constexpr std::size_t MAX_SLOTS = 26;
};

// Find the next unused letter in [a..z] for a new inventory slot. Returns 0
// when the bag is full. Shared by pickup, unequip eviction, and starting
// loadout spawning so all paths agree on letter assignment.
inline char allocate_inventory_letter(const InventoryComponent& inv) {
    if (inv.slots.size() >= InventoryComponent::MAX_SLOTS) return 0;
    bool used[26] = {false};
    for (const auto& slot : inv.slots) {
        int idx = slot.letter - 'a';
        if (idx >= 0 && idx < 26) used[idx] = true;
    }
    for (int i = 0; i < 26; ++i) {
        if (!used[i]) return static_cast<char>('a' + i);
    }
    return 0;
}

// One roll for a mob's drop table. Each entry is independent: the entry
// drops with probability `chance`; when it drops, the count is uniformly
// chosen from [min_count, max_count]. Non-stackable items ignore the count
// (they always spawn one entity per success).
struct DropEntry {
    std::string item_id;
    float chance = 1.0f;
    int min_count = 1;
    int max_count = 1;
};

// Attached at mob-spawn time, copied from the Mob template. Living on the
// entity avoids needing a template-id lookup from arbitrary entity handles
// later — the death handler in AttackAction just queries this component.
struct DropsComponent {
    std::vector<DropEntry> entries;
};

} // namespace engine