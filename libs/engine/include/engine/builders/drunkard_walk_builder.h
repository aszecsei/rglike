#pragma once

#include "../map_builder.h"
#include <ftxui/screen/color.hpp>
#include <algorithm>
#include <utility>

namespace engine {

// Drunkard walk algorithm - randomly walks the map carving out floor tiles
// Can be used as both an InitialMapBuilder and a MetaMapBuilder
class DrunkardWalkBuilder : public InitialMapBuilder, public MetaMapBuilder {
public:
    // Constructor
    DrunkardWalkBuilder(Terrain wall, Terrain floor, float open_percentage = 0.5f, int max_walks = 10)
        : wall_(std::move(wall)), floor_(std::move(floor)),
          open_percentage_(open_percentage), max_walks_(max_walks) {}

    // InitialMapBuilder implementation - creates map from scratch
    [[nodiscard]] MapBuilderState build(int width, int height, WELL512& rng) const override {
        MapBuilderState state(width, height);

        // Fill with walls
        for (auto& tile : state.tiles) {
            tile = wall_;
        }

        // Calculate target number of floor tiles
        int total_tiles = width * height;
        int target_floor_tiles = static_cast<int>(total_tiles * open_percentage_);

        // Track how many floor tiles we've carved
        int floor_count = 0;

        // Perform multiple drunkard walks
        for (int walk = 0; walk < max_walks_ && floor_count < target_floor_tiles; ++walk) {
            // Start at random position
            int x = rng.range(1, width - 2);
            int y = rng.range(1, height - 2);

            // Save first position as player start
            if (walk == 0) {
                state.player_start_position = Vector2(x, y);
            }

            // Walk until we hit target percentage
            while (floor_count < target_floor_tiles) {
                // Carve current position if it's a wall
                auto current_terrain = state.get(x, y);
                if (current_terrain && !current_terrain->passable) {
                    state.set_terrain(x, y, floor_);
                    floor_count++;
                }

                // Random direction: 0=North, 1=South, 2=East, 3=West
                int direction = rng() % 4;

                // Move in random direction (stay in bounds)
                switch (direction) {
                    case 0: // North
                        if (y > 1) y--;
                        break;
                    case 1: // South
                        if (y < height - 2) y++;
                        break;
                    case 2: // East
                        if (x < width - 2) x++;
                        break;
                    case 3: // West
                        if (x > 1) x--;
                        break;
                }

                // Small chance to jump to a new random location
                if (rng() % 20 == 0) {
                    x = rng.range(1, width - 2);
                    y = rng.range(1, height - 2);
                }
            }
        }

        return state;
    }

    // MetaMapBuilder implementation - modifies existing map
    [[nodiscard]] MapBuilderState build(const MapBuilderState& input_state, WELL512& rng) const override {
        MapBuilderState state = input_state;

        // Calculate current and target floor tiles
        int floor_count = 0;
        for (const auto& tile : state.tiles) {
            if (tile.passable) floor_count++;
        }

        int total_tiles = state.width * state.height;
        int target_floor_tiles = static_cast<int>(total_tiles * open_percentage_);

        // If we already have enough floor, return unchanged
        if (floor_count >= target_floor_tiles) {
            return state;
        }

        // Perform drunkard walks to open up more area
        for (int walk = 0; walk < max_walks_ && floor_count < target_floor_tiles; ++walk) {
            // Start at random position
            int x = rng.range(1, state.width - 2);
            int y = rng.range(1, state.height - 2);

            // Walk until we hit target percentage
            while (floor_count < target_floor_tiles) {
                // Carve current position if it's a wall
                auto current_terrain = state.get(x, y);
                if (current_terrain && !current_terrain->passable) {
                    state.set_terrain(x, y, floor_);
                    floor_count++;
                }

                // Random direction: 0=North, 1=South, 2=East, 3=West
                int direction = rng() % 4;

                // Move in random direction (stay in bounds)
                switch (direction) {
                    case 0: // North
                        if (y > 1) y--;
                        break;
                    case 1: // South
                        if (y < state.height - 2) y++;
                        break;
                    case 2: // East
                        if (x < state.width - 2) x++;
                        break;
                    case 3: // West
                        if (x > 1) x--;
                        break;
                }

                // Small chance to jump to a new random location
                if (rng() % 20 == 0) {
                    x = rng.range(1, state.width - 2);
                    y = rng.range(1, state.height - 2);
                }
            }
        }

        return state;
    }

private:
    Terrain wall_;
    Terrain floor_;
    float open_percentage_;  // Percentage of map to be floor (0.0 to 1.0)
    int max_walks_;          // Maximum number of drunkard walks to perform
};

} // namespace engine