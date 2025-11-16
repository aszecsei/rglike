#pragma once

#include "../map_builder.h"
#include <ftxui/screen/color.hpp>
#include <queue>
#include <unordered_set>
#include <utility>

namespace engine {

// Meta builder that culls (fills with walls) unreachable areas of the map
// Uses flood-fill from the player start position to find all reachable tiles
class CullUnreachableBuilder : public MetaMapBuilder {
public:
    explicit CullUnreachableBuilder(Terrain wall)
        : wall_(std::move(wall)) {}

    [[nodiscard]] MapBuilderState build(const MapBuilderState& input_state, WELL512& rng) const override {
        MapBuilderState state = input_state;

        // Find all reachable tiles using flood-fill from player start position
        std::unordered_set<int> reachable = flood_fill(state, state.player_start_position);

        // Fill unreachable tiles with walls
        for (int y = 0; y < state.height; ++y) {
            for (int x = 0; x < state.width; ++x) {
                int index = y * state.width + x;

                // If this tile is not reachable, make it a wall
                if (reachable.find(index) == reachable.end()) {
                    state.set_terrain(x, y, wall_);
                }
            }
        }

        return state;
    }

private:
    Terrain wall_;

    // Flood-fill algorithm to find all reachable tiles from start position
    [[nodiscard]] std::unordered_set<int> flood_fill(const MapBuilderState& state, const Vector2& start) const {
        std::unordered_set<int> visited;
        std::queue<Vector2> to_visit;

        // Start from player position
        to_visit.push(start);
        int start_index = start.y * state.width + start.x;
        visited.insert(start_index);

        // Directions: North, South, East, West
        const Vector2 directions[] = {
            {0, -1},  // North
            {0, 1},   // South
            {1, 0},   // East
            {-1, 0}   // West
        };

        // Breadth-first search
        while (!to_visit.empty()) {
            Vector2 current = to_visit.front();
            to_visit.pop();

            // Check all four adjacent tiles
            for (const auto& dir : directions) {
                int nx = current.x + dir.x;
                int ny = current.y + dir.y;

                // Check bounds
                if (nx < 0 || nx >= state.width || ny < 0 || ny >= state.height) {
                    continue;
                }

                int index = ny * state.width + nx;

                // Skip if already visited
                if (visited.find(index) != visited.end()) {
                    continue;
                }

                // Get terrain at this position
                auto terrain = state.get(nx, ny);
                if (!terrain || !terrain->passable) {
                    continue; // Skip walls
                }

                // Mark as visited and add to queue
                visited.insert(index);
                to_visit.push({nx, ny});
            }
        }

        return visited;
    }
};

} // namespace engine