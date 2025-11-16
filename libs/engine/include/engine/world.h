#pragma once

#include "map.h"
#include "terrain.h"
#include "components.h"
#include "action.h"
#include <optional>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <entt/entt.hpp>

namespace engine {

/**
 * @brief World manages the game state using an Entity Component System (ECS).
 *
 * The world encapsulates the game state including:
 * - Terrain map (stored as MapComponent on a map entity)
 * - Player entity (with Position, Player, Renderable, Vision, ActionCooldown components)
 * - Camera entity (tracks viewport)
 * - Action queue system (for turn-based gameplay)
 * - Game systems (vision, camera follow, etc.)
 *
 * Entity Storage:
 * - Map Entity: Has MapComponent storing the terrain grid and map metadata
 * - Player Entity: Has Position, Player, Renderable, Vision, and ActionCooldown components
 * - Camera Entity: Has Camera component for viewport tracking
 *
 * Direct ECS Access:
 * The underlying EnTT registry is accessible for advanced queries:
 * @code
 * auto& registry = world.get_registry();
 *
 * // Query player position directly
 * auto player_entity = world.get_player_entity();
 * auto* pos = registry.try_get<Position>(player_entity);
 *
 * // Iterate all entities with Position component
 * auto view = registry.view<Position>();
 * for (auto entity : view) {
 *     auto& pos = view.get<Position>(entity);
 *     // ... process position
 * }
 * @endcode
 */
class World {
public:
    World(int width, int height, std::string map_name);
    ~World() = default;

    // Get world dimensions (from map entity)
    [[nodiscard]] int get_width() const;
    [[nodiscard]] int get_height() const;
    [[nodiscard]] std::string get_map_name() const;

    // Get player entity
    [[nodiscard]] entt::entity get_player_entity() const { return player_entity_; }

    // Get camera entity
    [[nodiscard]] entt::entity get_camera_entity() const { return camera_entity_; }

    // Update all game systems
    void update_systems();

    // Get player position (convenience method)
    [[nodiscard]] Position get_player_position() const;

    // Set player position (convenience method)
    void set_player_position(int x, int y);

    // Move player by offset (checks passability)
    bool move_player(int dx, int dy);

    // Get terrain at position (returns nullopt if out of bounds)
    [[nodiscard]] std::optional<Terrain> get_terrain(int x, int y) const;

    // Set terrain at position (returns false if out of bounds)
    bool set_terrain(int x, int y, const Terrain& terrain);

    // Check if position is passable
    [[nodiscard]] bool is_passable(int x, int y) const;

    // Check if position is currently visible (in line-of-sight)
    [[nodiscard]] bool is_visible(int x, int y) const;

    // Check if position has ever been revealed (explored)
    [[nodiscard]] bool is_revealed(int x, int y) const;

    // Get all renderable entities at a position, sorted by render_order
    [[nodiscard]] std::vector<entt::entity> get_entities_at(int x, int y) const;

    // Temporary: Fill rectangle with terrain (for testing)
    void fill_rect(int x, int y, int width, int height, const Terrain& terrain);

    // Get the underlying map entity
    [[nodiscard]] entt::entity get_map_entity() const { return map_entity_; }

    // Get the underlying map component
    [[nodiscard]] const MapComponent* get_map_component() const;
    [[nodiscard]] MapComponent* get_map_component();

    // Replace the entire map
    void set_map(const Map& map);

    // Get the ECS registry
    [[nodiscard]] entt::registry& get_registry() { return registry_; }
    [[nodiscard]] const entt::registry& get_registry() const { return registry_; }

    // Clear the registry (useful for loading a new game)
    void clear_registry();

    // Action queue system
    void queue_action(std::unique_ptr<Action> action);
    bool process_next_action();  // Process one action, returns true if turn was consumed
    void clear_action_queue();
    [[nodiscard]] bool has_queued_actions() const;

private:
    entt::registry registry_;
    entt::entity map_entity_;
    entt::entity player_entity_;
    entt::entity camera_entity_;
    std::queue<std::unique_ptr<Action>> action_queue_;
};

} // namespace engine
