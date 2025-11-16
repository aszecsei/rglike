#pragma once

#include "../map_builder.h"
#include <ftxui/screen/color.hpp>
#include <algorithm>
#include <utility>

namespace engine {

// Meta builder that adds corridors between floor tiles
class CorridorBuilder : public MetaMapBuilder {
public:
    explicit CorridorBuilder(Terrain floor)
        : floor_(std::move(floor)) {}

    [[nodiscard]] MapBuilderState build(const MapBuilderState& input_state, WELL512& rng) const override {
        MapBuilderState state = input_state;

        // Need at least 2 rooms to connect
        if (state.rooms.size() < 2) {
            return state;
        }

        // Get room center positions
        std::vector<Vector2> room_centers;
        for (const auto& room : state.rooms) {
            room_centers.push_back(room.center());
        }

        // Shuffle room centers using Fisher-Yates shuffle
        for (size_t i = room_centers.size() - 1; i > 0; --i) {
            size_t j = rng() % (i + 1);
            std::swap(room_centers[i], room_centers[j]);
        }

        // Connect consecutive pairs with L-shaped corridors
        for (size_t i = 0; i < room_centers.size() - 1; ++i) {
            const Vector2& pos1 = room_centers[i];
            const Vector2& pos2 = room_centers[i + 1];

            // Horizontal then vertical corridor
            int min_x = std::min(pos1.x, pos2.x);
            int max_x = std::max(pos1.x, pos2.x);
            for (int x = min_x; x <= max_x; ++x) {
                state.set_terrain(x, pos1.y, floor_);
            }

            int min_y = std::min(pos1.y, pos2.y);
            int max_y = std::max(pos1.y, pos2.y);
            for (int y = min_y; y <= max_y; ++y) {
                state.set_terrain(pos2.x, y, floor_);
            }
        }

        return state;
    }

private:
    Terrain floor_;
};

} // namespace engine
