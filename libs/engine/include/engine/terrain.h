#pragma once

#include "registry.h"
#include <ftxui/screen/color.hpp>
#include <string>

namespace engine {

struct Terrain {
    std::string glyph;                          // Unicode character representation
    ftxui::Color fg_color = ftxui::Color::Default;  // Foreground color
    ftxui::Color bg_color = ftxui::Color::Default;  // Background color
    bool passable = true;                       // Can entities move through it?
    bool blocks_vision = false;                 // Does it block line of sight?

    Terrain() = default;
    Terrain(std::string glyph, ftxui::Color fg, ftxui::Color bg, bool passable, bool blocks_vision = false)
        : glyph(std::move(glyph)), fg_color(fg), bg_color(bg), passable(passable), blocks_vision(blocks_vision) {}
};

// Type alias for terrain registry
using TerrainRegistry = Registry<Terrain>;

} // namespace engine
