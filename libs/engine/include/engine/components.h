#pragma once

#include "map.h"
#include "stats.h"
#include <ftxui/screen/color.hpp>
#include <string>
#include <unordered_map>

namespace engine {

// Faction response type - how one faction responds to another
enum class FactionResponse {
    IGNORE,  // Ignores entities of that faction
    ATTACK,  // Hostile - will attack on sight
    FLEE     // Will flee from entities of that faction
};

// Tag component to mark the player entity
struct Player {};

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

// Door component - represents a door that can be opened/closed
struct Door {
    bool is_open = false;
    std::string open_glyph = "'";
    std::string closed_glyph = "+";
    ftxui::Color color;

    explicit Door(ftxui::Color color = ftxui::Color::RGB(139, 69, 19))
        : color(color) {}
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
};

// Combat stats component - simple combat attributes for entities
// Used alongside or instead of full StatsComponent for simpler entities
struct CombatStats {
    int defense = 0;   // Armor class / defense value
    int power = 1;     // Attack power / damage

    CombatStats() = default;
    CombatStats(int defense, int power) : defense(defense), power(power) {}
};

} // namespace engine