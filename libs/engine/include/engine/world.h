#pragma once

#include "map.h"
#include "terrain.h"
#include "components.h"
#include "action.h"
#include "faction.h"
#include "item.h"
#include "game_log.h"
#include "well512.h"
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
    World(int width, int height, std::string map_name, uint32_t rng_seed = 0xCAFEBABEu);
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

    // Apply a player-issued action and then advance the simulation until the
    // player is ready to act again (or has died). This is the single entry
    // point for player input from the UI layer.
    void apply_player_action(std::unique_ptr<Action> action);

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

    // Game log — combat messages, level-ups, etc. are written here by actions
    // and read by the log UI panel. Owned by World so engine code can log
    // without UI plumbing.
    GameLog& get_game_log() { return game_log_; }
    [[nodiscard]] const GameLog& get_game_log() const { return game_log_; }

    // Faction registry — set by the scene; used by combat/AI to resolve
    // relationships between entities. World does not own it.
    void set_faction_registry(const FactionRegistry* registry) { faction_registry_ = registry; }
    [[nodiscard]] const FactionRegistry* get_faction_registry() const { return faction_registry_; }

    // Item registry — set by the scene; used by AttackAction to spawn dropped
    // items on mob death and by DropAction to look up template data when an
    // inventory stack is split back onto the ground. World does not own it.
    void set_item_registry(const ItemRegistry* registry) { item_registry_ = registry; }
    [[nodiscard]] const ItemRegistry* get_item_registry() const { return item_registry_; }

    // Spawn a ground item entity at (x, y) using the given template id.
    // Returns entt::null if the item registry is missing or the id is unknown.
    // `count` is meaningful for stackable items only; non-stackable items
    // always spawn one entity per call (caller should loop if needed).
    entt::entity spawn_ground_item(const std::string& item_id, int x, int y, int count = 1);

    // RNG owned by the world so AI and combat are deterministic per seed.
    WELL512& get_rng() { return rng_; }

    // Player death state. Set by AttackAction when the player's HP hits zero.
    // The scene polls this each frame and transitions to the game-over scene.
    [[nodiscard]] bool is_player_dead() const { return player_dead_; }
    [[nodiscard]] const std::optional<std::string>& get_player_death_cause() const {
        return player_death_cause_;
    }
    void set_player_dead(std::string cause) {
        player_dead_ = true;
        player_death_cause_ = std::move(cause);
    }

private:
    entt::registry registry_;
    entt::entity map_entity_;
    entt::entity player_entity_;
    entt::entity camera_entity_;
    std::queue<std::unique_ptr<Action>> action_queue_;
    GameLog game_log_;
    const FactionRegistry* faction_registry_ = nullptr;
    const ItemRegistry* item_registry_ = nullptr;
    WELL512 rng_;
    bool player_dead_ = false;
    std::optional<std::string> player_death_cause_;
};

} // namespace engine
