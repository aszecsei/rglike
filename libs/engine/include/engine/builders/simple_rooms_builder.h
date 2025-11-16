#pragma once

#include "../map_builder.h"
#include <ftxui/screen/color.hpp>
#include <utility>

namespace engine {

// Initial builder that creates a map filled with walls and random rooms
class SimpleRoomsBuilder : public InitialMapBuilder {
public:
    SimpleRoomsBuilder(Terrain wall, Terrain floor, int num_rooms = 5)
        : wall_(std::move(wall)), floor_(std::move(floor)), num_rooms_(num_rooms) {}

    [[nodiscard]] MapBuilderState build(int width, int height, WELL512& rng) const override {
        // Use MapBuilderState for construction
        MapBuilderState state(width, height);

        // Fill with walls
        for (auto& tile : state.tiles) {
            tile = wall_;
        }

        // Generate random rooms
        for (int i = 0; i < num_rooms_; ++i) {
            int room_x = rng.range(1, width - 10);
            int room_y = rng.range(1, height - 10);
            int room_w = rng.range(4, 8);
            int room_h = rng.range(4, 8);

            // Fill room with floor
            state.fill_rect(room_x, room_y, room_w, room_h, floor_);

            // Store room position
            state.rooms.push_back({room_x, room_y, room_w, room_h});
        }

        // Set player start position to center of first room
        if (!state.rooms.empty()) {
            state.player_start_position = state.rooms[0].center();
        }

        return state;
    }

private:
    Terrain wall_;
    Terrain floor_;
    int num_rooms_;
};

} // namespace engine
