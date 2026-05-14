#include <engine/world.h>
#include <engine/systems.h>
#include <engine/ai.h>
#include <engine/action.h>
#include <engine/constants.h>

#include <utility>
#include <algorithm>
#include <cassert>
#include <vector>

namespace engine {

World::World(int width, int height, std::string map_name, uint32_t rng_seed)
    : rng_(rng_seed) {
    // Create map entity with MapComponent
    map_entity_ = registry_.create();
    auto& map_component = registry_.emplace<MapComponent>(map_entity_, width, height, 1, std::move(map_name));

    // Initialize with default floor terrain
    Terrain default_floor{
        ".", ftxui::Color::White, ftxui::Color::Default, ftxui::Color::Default, true
    };
    for (auto& tile : map_component.map.tiles) {
        tile = default_floor;
    }

    // Create player entity with Position, Player, Renderable, Vision, and ActionCooldown components
    player_entity_ = registry_.create();
    registry_.emplace<Position>(player_entity_, 0, 0);
    registry_.emplace<Player>(player_entity_);
    registry_.emplace<Renderable>(player_entity_, "@", ftxui::Color::Yellow, ftxui::Color::Default, true, constants::RENDER_ORDER_PLAYER);
    registry_.emplace<VisionComponent>(player_entity_, constants::DEFAULT_VISION_RANGE);
    registry_.emplace<ActionCooldown>(player_entity_, 0);  // Player starts ready to act

    // Create camera entity with Camera component
    camera_entity_ = registry_.create();
    registry_.emplace<Camera>(camera_entity_, 0, 0);
}

int World::get_width() const {
    const auto* map_comp = registry_.try_get<MapComponent>(map_entity_);
    return map_comp ? map_comp->map.width : 0;
}

int World::get_height() const {
    const auto* map_comp = registry_.try_get<MapComponent>(map_entity_);
    return map_comp ? map_comp->map.height : 0;
}

std::string World::get_map_name() const {
    const auto* map_comp = registry_.try_get<MapComponent>(map_entity_);
    return map_comp ? map_comp->name : "";
}

Position World::get_player_position() const {
    const auto* pos = registry_.try_get<Position>(player_entity_);
    return pos ? *pos : Position{0, 0};
}

void World::set_player_position(int x, int y) {
    auto* pos = registry_.try_get<Position>(player_entity_);
    assert(pos && "Player entity must have Position component");
    if (!pos) {
        // In release builds, log error if player lost its Position component
        return;
    }
    pos->x = x;
    pos->y = y;
}

void World::apply_player_action(std::unique_ptr<Action> action) {
    if (player_dead_) return;

    queue_action(std::move(action));

    // Drain the player's action and any alternatives it queues (bump-door,
    // bump-attack).
    while (has_queued_actions()) {
        process_next_action();
    }

    // Now run the AI tick loop. Each iteration:
    //   - Snapshot non-player actors with cooldown 0, then let each one act.
    //     We snapshot first because combat may destroy entities and the
    //     iterator can't survive that.
    //   - If the player became ready, break.
    //   - If nothing acted this iteration, advance time so the next ready
    //     actor surfaces. The safety net break protects against an empty
    //     world (no cooldowns at all).
    while (!player_dead_) {
        std::vector<entt::entity> ready;
        auto view = registry_.view<ActionCooldown>(entt::exclude<Player>);
        for (auto e : view) {
            if (view.get<ActionCooldown>(e).cooldown == 0) {
                ready.push_back(e);
            }
        }

        bool any_acted = false;
        for (auto e : ready) {
            if (!registry_.valid(e)) continue;  // Destroyed earlier this round.
            systems::ai_take_turn(*this, e, rng_);
            while (has_queued_actions()) {
                process_next_action();
            }
            any_acted = true;
            if (player_dead_) break;
        }

        if (auto* pc = registry_.try_get<ActionCooldown>(player_entity_); pc && pc->cooldown == 0) {
            break;
        }

        if (!any_acted) {
            if (!systems::advance_time(registry_)) break;
        }
    }
}

std::optional<Terrain> World::get_terrain(int x, int y) const {
    const auto* map_comp = registry_.try_get<MapComponent>(map_entity_);
    if (!map_comp) return std::nullopt;
    return map_comp->map.get(x, y);
}

bool World::set_terrain(int x, int y, const Terrain& terrain) {
    auto* map_comp = registry_.try_get<MapComponent>(map_entity_);
    if (!map_comp) return false;

    auto existing = map_comp->map.get(x, y);
    if (!existing.has_value()) {
        return false;
    }
    map_comp->map.tiles[y * map_comp->map.width + x] = terrain;
    return true;
}

bool World::is_passable(int x, int y) const {
    // Check terrain passability
    auto terrain = get_terrain(x, y);
    if (!terrain.has_value() || !terrain->passable) {
        return false;
    }

    // Check for entities that block movement at this position
    auto view = registry_.view<Position, BlocksMovement>();
    for (auto entity : view) {
        const auto& pos = view.get<Position>(entity);
        if (pos.x == x && pos.y == y) {
            return false;  // Entity blocking movement at this position
        }
    }

    return true;
}

bool World::is_visible(int x, int y) const {
    const auto* map_comp = registry_.try_get<MapComponent>(map_entity_);
    if (!map_comp) return false;
    return map_comp->map.is_visible(x, y);
}

bool World::is_revealed(int x, int y) const {
    const auto* map_comp = registry_.try_get<MapComponent>(map_entity_);
    if (!map_comp) return false;
    return map_comp->map.is_revealed(x, y);
}

std::vector<entt::entity> World::get_entities_at(int x, int y) const {
    std::vector<entt::entity> result;

    auto view = registry_.view<Position, Renderable>();
    for (auto entity : view) {
        const auto& pos = view.get<Position>(entity);
        if (pos.x == x && pos.y == y) {
            result.push_back(entity);
        }
    }

    // Sort by render_order
    std::sort(result.begin(), result.end(), [this](entt::entity a, entt::entity b) {
        const auto* render_a = registry_.try_get<Renderable>(a);
        const auto* render_b = registry_.try_get<Renderable>(b);
        int order_a = render_a ? render_a->render_order : 0;
        int order_b = render_b ? render_b->render_order : 0;
        return order_a < order_b;
    });

    return result;
}

void World::fill_rect(int x, int y, int width, int height, const Terrain& terrain) {
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            set_terrain(x + dx, y + dy, terrain);
        }
    }
}

const MapComponent* World::get_map_component() const {
    return registry_.try_get<MapComponent>(map_entity_);
}

MapComponent* World::get_map_component() {
    return registry_.try_get<MapComponent>(map_entity_);
}

void World::set_map(const Map& map) {
    auto* map_comp = registry_.try_get<MapComponent>(map_entity_);
    if (map_comp) {
        map_comp->map = map;
    }
}

void World::clear_registry() {
    registry_.clear();
    // Recreate essential entities
    map_entity_ = registry_.create();
    player_entity_ = registry_.create();
    camera_entity_ = registry_.create();
}

entt::entity World::spawn_ground_item(const std::string& item_id, int x, int y, int count) {
    if (!item_registry_) return entt::null;
    auto tmpl = item_registry_->get(item_id);
    if (!tmpl) return entt::null;
    if (count < 1) count = 1;

    auto e = registry_.create();
    registry_.emplace<Position>(e, x, y);
    registry_.emplace<Renderable>(e,
                                  tmpl->glyph,
                                  tmpl->fg_color,
                                  tmpl->bg_color,
                                  tmpl->bold,
                                  tmpl->render_order);
    registry_.emplace<NameComponent>(e, tmpl->name);
    ItemComponent ic;
    ic.item_id = item_id;
    ic.is_stackable = tmpl->is_stackable;
    ic.count = tmpl->is_stackable ? count : 1;
    registry_.emplace<ItemComponent>(e, std::move(ic));
    return e;
}

void World::update_systems() {
    // Update field-of-view for entities with vision
    systems::update_viewshed(registry_, map_entity_);

    // Update camera to follow player with deadzone
    systems::update_camera_follow(registry_, player_entity_, camera_entity_, constants::CAMERA_DEADZONE_TILES);

    // Future: Add more systems here (AI, physics, etc.)
}

// Action queue system implementation
void World::queue_action(std::unique_ptr<Action> action) {
    action_queue_.push(std::move(action));
}

bool World::process_next_action() {
    if (action_queue_.empty()) {
        return false;
    }

    auto action = std::move(action_queue_.front());
    action_queue_.pop();

    // Get the entity performing this action
    entt::entity actor = action->get_actor();

    // Execute the action
    ActionResult result = action->execute(*this, action_queue_);

    // Apply cooldown cost if the action succeeded or failed
    if (result.status == ActionResult::Status::Success || result.status == ActionResult::Status::Failure) {
        auto* cooldown = registry_.try_get<ActionCooldown>(actor);
        if (cooldown) {
            cooldown->cooldown += result.cooldown_cost;
        }
        return true;  // Turn was consumed
    }

    // Return true if the turn was consumed
    return false;
}

void World::clear_action_queue() {
    while (!action_queue_.empty()) {
        action_queue_.pop();
    }
}

bool World::has_queued_actions() const {
    return !action_queue_.empty();
}

} // namespace engine
