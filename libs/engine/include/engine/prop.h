#pragma once

#include "constants.h"
#include "registry.h"
#include <ftxui/screen/color.hpp>
#include <optional>
#include <string>

namespace engine {

// A Prop is a placeable scenery/interactive entity defined in data: doors,
// candles, tables, chairs, barrels, etc. Mobs are intentionally a separate
// type (they have stats/AI/factions); props are the static-or-toggleable
// furniture of the world.
//
// Doors are modeled as a prop with `open_glyph` set. At spawn time, if a prop
// has an open_glyph, the entity gets a Door component carrying the
// open/closed state and the two glyphs; opening/closing flips the Renderable
// glyph and toggles the blocking components.
struct Prop {
    std::string id;
    std::string name;
    std::string glyph;                              // Default / closed glyph.
    ftxui::Color fg_color = ftxui::Color::Default;
    ftxui::Color bg_color = ftxui::Color::Default;
    bool bold = false;
    int render_order = constants::RENDER_ORDER_DOORS;
    bool blocks_movement = false;
    bool blocks_vision = false;

    // If set, this prop is openable. Spawning attaches a Door component;
    // opening swaps the displayed glyph to open_glyph and clears the
    // blocking components.
    std::optional<std::string> open_glyph;
};

using PropRegistry = Registry<Prop>;

} // namespace engine
