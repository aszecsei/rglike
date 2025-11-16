#pragma once

#include "terrain.h"
#include <vector>
#include <optional>
#include <unordered_set>

namespace engine {

// Map state for gameplay
struct Map {
    int width;
    int height;
    int depth;                                // Dungeon depth/level
    std::vector<Terrain> tiles;
    std::vector<bool> revealed;               // Which tiles have been revealed to the player
    std::vector<bool> visible;                // Which tiles are currently visible to the player
    std::unordered_set<int> bloodstains;      // Indices where creatures have been slain

    Map(int w, int h, int d = 1)
        : width(w), height(h), depth(d), tiles(w * h),
          revealed(w * h, false), visible(w * h, false) {}

    // Get terrain at position (returns nullopt if out of bounds)
    [[nodiscard]] std::optional<Terrain> get(int x, int y) const {
        if (!in_bounds(x, y)) return std::nullopt;
        return tiles[to_index(x, y)];
    }

    [[nodiscard]] bool is_visible(int x, int y) const {
        return visible[to_index(x, y)];
    }

    [[nodiscard]] bool is_revealed(int x, int y) const {
        return revealed[to_index(x, y)];
    }

    [[nodiscard]] ftxui::Color get_background_color(int x, int y) const {
        return bloodstains.contains(to_index(x, y)) ? ftxui::Color::Red : get(x, y).value().bg_color;
    }

private:
    [[nodiscard]] int to_index(int x, int y) const { return y * width + x; }
    [[nodiscard]] bool in_bounds(int x, int y) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }
};

} // namespace engine
