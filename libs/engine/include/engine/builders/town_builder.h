#pragma once

#include "../map_builder.h"
#include <ftxui/screen/color.hpp>
#include <algorithm>
#include <utility>
#include <vector>

namespace engine {

// Initial builder that creates a medieval town with walls, buildings, and a river
class TownBuilder : public InitialMapBuilder {
public:
    TownBuilder(Terrain wall, Terrain floor, Terrain water, Terrain grass)
        : wall_(std::move(wall)), floor_(std::move(floor)),
          water_(std::move(water)), grass_(std::move(grass)) {}

    [[nodiscard]] MapBuilderState build(int width, int height, WELL512& rng) const override {
        MapBuilderState state(width, height);

        // Fill entire map with grass
        for (auto& tile : state.tiles) {
            tile = grass_;
        }

        // Calculate town dimensions (leave room for river to the east)
        int town_width = static_cast<int>(width * 0.65f);  // Town takes ~65% of width
        int town_height = height - 4;  // Leave margins
        int town_x = 2;
        int town_y = 2;

        // Build town perimeter wall
        build_town_wall(state, town_x, town_y, town_width, town_height);

        // Fill interior with floor
        state.fill_rect(town_x + 1, town_y + 1, town_width - 2, town_height - 2, floor_);

        // Add main gate on south wall (as a door entity)
        int gate_x = town_x + town_width / 2;
        state.set_terrain(gate_x, town_y + town_height - 1, floor_);
        state.entity_spawns.push_back({Vector2(gate_x, town_y + town_height - 1), "door", false});

        // Add buildings inside town
        build_buildings(state, rng, town_x + 2, town_y + 2, town_width - 4, town_height - 4);

        // Build river to the east
        int river_x = town_x + town_width + 3;
        build_river(state, river_x, 0, height);

        // Build bridge across the river
        int bridge_y = height / 2;
        build_bridge(state, river_x - 1, bridge_y, 4);

        // Add a road from town gate to bridge
        build_road(state, gate_x, town_y + town_height, river_x + 2, bridge_y);

        // Spawn merchants in the town (1-3 merchants)
        int num_merchants = rng.range(1, 3);
        for (int i = 0; i < num_merchants; ++i) {
            // Find a random floor position inside the town
            int merchant_x = town_x + rng.range(2, town_width - 3);
            int merchant_y = town_y + rng.range(2, town_height - 3);

            // Make sure it's a floor tile and not already occupied
            auto terrain = state.get(merchant_x, merchant_y);
            if (terrain && terrain->glyph == floor_.glyph) {
                // Check if position is already occupied by another spawn
                bool occupied = false;
                for (const auto& spawn : state.entity_spawns) {
                    if (spawn.position.x == merchant_x && spawn.position.y == merchant_y) {
                        occupied = true;
                        break;
                    }
                }

                if (!occupied) {
                    state.entity_spawns.push_back({Vector2(merchant_x, merchant_y), "merchant", false});
                }
            }
        }

        // Set player start position in the town center
        state.player_start_position = Vector2(town_x + town_width / 2, town_y + town_height / 2);

        return state;
    }

private:
    Terrain wall_;
    Terrain floor_;
    Terrain water_;
    Terrain grass_;

    // Build the town perimeter wall
    void build_town_wall(MapBuilderState& state, int x, int y, int w, int h) const {
        // Horizontal walls
        for (int i = 0; i < w; ++i) {
            state.set_terrain(x + i, y, wall_);           // Top wall
            state.set_terrain(x + i, y + h - 1, wall_);   // Bottom wall
        }

        // Vertical walls
        for (int i = 0; i < h; ++i) {
            state.set_terrain(x, y + i, wall_);           // Left wall
            state.set_terrain(x + w - 1, y + i, wall_);   // Right wall
        }
    }

    // Build buildings within the town
    void build_buildings(MapBuilderState& state, WELL512& rng, int x, int y, int w, int h) const {
        // Number of buildings to generate
        int num_buildings = rng.range(4, 8);

        for (int i = 0; i < num_buildings; ++i) {
            // Random building size
            int bw = rng.range(5, 10);
            int bh = rng.range(5, 8);

            // Random position (with padding)
            int bx = x + rng.range(0, std::max(1, w - bw - 2));
            int by = y + rng.range(0, std::max(1, h - bh - 2));

            // Check if building would overlap with existing buildings
            if (!is_area_clear(state, bx - 1, by - 1, bw + 2, bh + 2)) {
                continue;  // Skip this building
            }

            // Build the building
            build_single_building(state, rng, bx, by, bw, bh);
            state.rooms.push_back({bx, by, bw, bh});
        }
    }

    // Check if an area is clear (all floor tiles)
    [[nodiscard]] bool is_area_clear(const MapBuilderState& state, int x, int y, int w, int h) const {
        for (int dy = 0; dy < h; ++dy) {
            for (int dx = 0; dx < w; ++dx) {
                auto terrain = state.get(x + dx, y + dy);
                if (!terrain || !terrain->passable || terrain->glyph != floor_.glyph) {
                    return false;
                }
            }
        }
        return true;
    }

    // Build a single building with walls and a door
    void build_single_building(MapBuilderState& state, WELL512& rng, int x, int y, int w, int h) const {
        // Outer walls
        for (int i = 0; i < w; ++i) {
            state.set_terrain(x + i, y, wall_);
            state.set_terrain(x + i, y + h - 1, wall_);
        }
        for (int i = 0; i < h; ++i) {
            state.set_terrain(x, y + i, wall_);
            state.set_terrain(x + w - 1, y + i, wall_);
        }

        // Add a door entity on a random wall
        int door_side = rng() % 4;  // 0=north, 1=south, 2=east, 3=west
        int door_x, door_y;
        switch (door_side) {
            case 0: // North
                door_x = x + w / 2;
                door_y = y;
                break;
            case 1: // South
                door_x = x + w / 2;
                door_y = y + h - 1;
                break;
            case 2: // East
                door_x = x + w - 1;
                door_y = y + h / 2;
                break;
            case 3: // West
            default:
                door_x = x;
                door_y = y + h / 2;
                break;
        }
        // Replace wall with floor and spawn door entity
        state.set_terrain(door_x, door_y, floor_);
        state.entity_spawns.push_back({Vector2(door_x, door_y), "door", false});
    }

    // Build a vertical river
    void build_river(MapBuilderState& state, int x, int y_start, int y_end) const {
        int river_width = 3;  // River is 3 tiles wide

        for (int y = y_start; y < y_end; ++y) {
            for (int dx = 0; dx < river_width; ++dx) {
                state.set_terrain(x + dx, y, water_);
            }
        }
    }

    // Build a bridge across the river
    void build_bridge(MapBuilderState& state, int x, int y, int length) const {
        for (int i = 0; i < length; ++i) {
            state.set_terrain(x + i, y, floor_);
            state.set_terrain(x + i, y + 1, floor_);  // Bridge is 2 tiles wide
        }
    }

    // Build a simple road (straight lines)
    void build_road(MapBuilderState& state, int x1, int y1, int x2, int y2) const {
        // Horizontal segment
        int min_x = std::min(x1, x2);
        int max_x = std::max(x1, x2);
        for (int x = min_x; x <= max_x; ++x) {
            auto terrain = state.get(x, y1);
            if (terrain && terrain->glyph == grass_.glyph) {
                state.set_terrain(x, y1, floor_);
            }
        }

        // Vertical segment
        int min_y = std::min(y1, y2);
        int max_y = std::max(y1, y2);
        for (int y = min_y; y <= max_y; ++y) {
            auto terrain = state.get(x2, y);
            if (terrain && terrain->glyph == grass_.glyph) {
                state.set_terrain(x2, y, floor_);
            }
        }
    }
};

} // namespace engine