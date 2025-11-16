#pragma once

#include "map.h"
#include "math.h"
#include "terrain.h"
#include "well512.h"
#include <memory>
#include <vector>
#include <functional>

namespace engine {

// Entity spawn information for the builder
struct EntitySpawn {
    Vector2 position;
    std::string type;  // "door", etc.
    bool data_bool = false;  // Generic data field (e.g., door open state)
};

// Map builder state - used during map construction
struct MapBuilderState {
    int width;
    int height;
    int depth;
    std::vector<Terrain> tiles;
    Vector2 player_start_position;
    std::vector<Rect> rooms;
    std::vector<EntitySpawn> entity_spawns;  // Entities to spawn after map creation

    MapBuilderState(int w, int h, int d = 1)
        : width(w), height(h), depth(d), tiles(w * h), player_start_position(0, 0) {}

    // Get terrain at position (returns nullopt if out of bounds)
    [[nodiscard]] std::optional<Terrain> get(int x, int y) const {
        if (!in_bounds(x, y)) return std::nullopt;
        return tiles[to_index(x, y)];
    }

    // Modify terrain at position
    void set_terrain(int x, int y, const Terrain& terrain) {
        if (in_bounds(x, y)) {
            tiles[to_index(x, y)] = terrain;
        }
    }

    // Fill a rectangle with terrain
    void fill_rect(int x, int y, int w, int h, const Terrain& terrain) {
        for (int dy = 0; dy < h; ++dy) {
            for (int dx = 0; dx < w; ++dx) {
                int px = x + dx;
                int py = y + dy;
                if (in_bounds(px, py)) {
                    tiles[to_index(px, py)] = terrain;
                }
            }
        }
    }

    // Finalize the builder state into a Map
    [[nodiscard]] Map finalize() const {
        Map map(width, height, depth);
        map.tiles = tiles;
        return map;
    }

private:
    [[nodiscard]] int to_index(int x, int y) const { return y * width + x; }
    [[nodiscard]] bool in_bounds(int x, int y) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }
};

// Forward declarations
class InitialMapBuilder;
class MetaMapBuilder;

// Base class for initial map builders - creates a map from scratch
class InitialMapBuilder {
public:
    virtual ~InitialMapBuilder() = default;

    // Pure virtual function to build initial map state
    [[nodiscard]] virtual MapBuilderState build(int width, int height, WELL512& rng) const = 0;
};

// Base class for meta map builders - transforms an existing map state
class MetaMapBuilder {
public:
    virtual ~MetaMapBuilder() = default;

    // Pure virtual function to modify a map state
    [[nodiscard]] virtual MapBuilderState build(const MapBuilderState& input_state, WELL512& rng) const = 0;
};

// Functional map builder chain with fluent interface
class MapBuilderChain {
public:
    // Default constructor - chain starts empty
    MapBuilderChain() = default;

    // Set the initial builder (fluent interface) - takes unique_ptr
    MapBuilderChain& start_with(std::unique_ptr<InitialMapBuilder> initial_builder) {
        initial_builder_ = std::move(initial_builder);
        return *this;
    }

    // Set the initial builder (fluent interface) - template version with automatic make_unique
    template<typename T, typename... Args>
    MapBuilderChain& start_with(Args&&... args) {
        static_assert(std::is_base_of_v<InitialMapBuilder, T>,
                      "T must derive from InitialMapBuilder");
        initial_builder_ = std::make_unique<T>(std::forward<Args>(args)...);
        return *this;
    }

    // Add a meta builder to the chain (fluent interface) - takes unique_ptr
    MapBuilderChain& with(std::unique_ptr<MetaMapBuilder> meta_builder) {
        meta_builders_.push_back(std::move(meta_builder));
        return *this;
    }

    // Add a meta builder to the chain (fluent interface) - template version with automatic make_unique
    template<typename T, typename... Args>
    MapBuilderChain& with(Args&&... args) {
        static_assert(std::is_base_of_v<MetaMapBuilder, T>,
                      "T must derive from MetaMapBuilder");
        meta_builders_.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        return *this;
    }

    // Build the final map by executing the chain
    [[nodiscard]] Map build(int width, int height, WELL512& rng) const {
        if (!initial_builder_) {
            throw std::runtime_error("MapBuilderChain: No initial builder set. Call start_with() first.");
        }

        // Start with initial map state
        MapBuilderState current_state = initial_builder_->build(width, height, rng);

        // Apply each meta builder in sequence
        for (const auto& meta_builder : meta_builders_) {
            current_state = meta_builder->build(current_state, rng);
        }

        // Finalize the state into a Map
        return current_state.finalize();
    }

    // Convenience method to build with a seed
    [[nodiscard]] Map build(int width, int height, uint32_t seed) const {
        WELL512 rng(seed);
        return build(width, height, rng);
    }

    // Build and return the final state (useful for accessing player_start_position)
    [[nodiscard]] MapBuilderState build_state(int width, int height, WELL512& rng) const {
        if (!initial_builder_) {
            throw std::runtime_error("MapBuilderChain: No initial builder set. Call start_with() first.");
        }

        // Start with initial map state
        MapBuilderState current_state = initial_builder_->build(width, height, rng);

        // Apply each meta builder in sequence
        for (const auto& meta_builder : meta_builders_) {
            current_state = meta_builder->build(current_state, rng);
        }

        return current_state;
    }

    // Convenience method to build state with a seed
    [[nodiscard]] MapBuilderState build_state(int width, int height, uint32_t seed) const {
        WELL512 rng(seed);
        return build_state(width, height, rng);
    }

private:
    std::unique_ptr<InitialMapBuilder> initial_builder_;
    std::vector<std::unique_ptr<MetaMapBuilder>> meta_builders_;
};

} // namespace engine
