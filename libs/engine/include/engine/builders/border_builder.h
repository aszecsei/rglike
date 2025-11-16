#pragma once

#include "../map_builder.h"
#include <ftxui/screen/color.hpp>
#include <utility>

namespace engine {

// Meta builder that adds a border of walls around the map
class BorderBuilder : public MetaMapBuilder {
public:
    explicit BorderBuilder(Terrain wall)
        : wall_(std::move(wall)) {}

    [[nodiscard]] MapBuilderState build(const MapBuilderState& input_state, WELL512& rng) const override {
        MapBuilderState state = input_state;

        // Top and bottom borders
        for (int x = 0; x < state.width; ++x) {
            state.set_terrain(x, 0, wall_);
            state.set_terrain(x, state.height - 1, wall_);
        }

        // Left and right borders
        for (int y = 0; y < state.height; ++y) {
            state.set_terrain(0, y, wall_);
            state.set_terrain(state.width - 1, y, wall_);
        }

        return state;
    }

private:
    Terrain wall_;
};

} // namespace engine
