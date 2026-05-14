#pragma once

#include "constants.h"
#include "registry.h"
#include "stats.h"
#include <ftxui/screen/color.hpp>
#include <string>
#include <optional>

namespace engine {

// Mob definition - template for spawning mob entities
struct Mob {
    std::string name;                           // Display name (e.g., "Goblin")
    std::string glyph;                          // Unicode character representation
    ftxui::Color fg_color = ftxui::Color::Default;  // Foreground color
    ftxui::Color bg_color = ftxui::Color::Default;  // Background color
    bool bold = false;                          // Bold text rendering
    int render_order = constants::RENDER_ORDER_MOBS;  // Render priority (higher = on top)

    bool blocks_movement = true;                // Does this mob block the tile it stands on?
    bool blocks_vision = false;                 // Does this mob block line of sight?

    int vision_range = 8;                       // How far the mob can see

    // Stats system (optional - if not set, uses legacy hp/defense/power)
    std::optional<Stats> base_stats;            // Base stats template for spawned entities
    int min_level = 1;                          // Minimum level when spawned
    int max_level = 1;                          // Maximum level when spawned (randomly chosen in range)

    // Legacy combat stats (used if base_stats is not set)
    int max_hp = 10;                            // Maximum hit points
    int hp = 10;                                // Current hit points
    int defense = 0;                            // Armor class / defense value
    int power = 1;                              // Attack power / damage

    std::string faction_id = "neutral";         // Faction this mob belongs to

    Mob() = default;
    Mob(std::string name, std::string glyph, ftxui::Color fg, ftxui::Color bg,
        bool bold, int render_order, bool blocks_movement, bool blocks_vision,
        int vision_range, int max_hp, int defense, int power, std::string faction_id = "neutral")
        : name(std::move(name)), glyph(std::move(glyph)), fg_color(fg), bg_color(bg),
          bold(bold), render_order(render_order), blocks_movement(blocks_movement),
          blocks_vision(blocks_vision), vision_range(vision_range),
          max_hp(max_hp), hp(max_hp), defense(defense), power(power),
          faction_id(std::move(faction_id)) {}
};

// Type alias for mob registry
using MobRegistry = Registry<Mob>;

} // namespace engine