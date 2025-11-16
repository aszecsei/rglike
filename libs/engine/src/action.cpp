#include <engine/action.h>
#include <engine/world.h>
#include <engine/components.h>
#include <engine/constants.h>
#include <sstream>

namespace engine {

// MoveAction implementation
ActionResult MoveAction::execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) {
    auto& registry = world.get_registry();
    auto* pos = registry.try_get<Position>(actor_);

    if (!pos) {
        return {ActionResult::Status::Invalid, 0};
    }

    int new_x = pos->x + dx_;
    int new_y = pos->y + dy_;

    // Check if target position is out of bounds
    auto terrain = world.get_terrain(new_x, new_y);
    if (!terrain.has_value()) {
        return {ActionResult::Status::Invalid, 0};
    }

    // Check for closed doors at the target position
    auto door_view = registry.view<Position, Door, BlocksMovement>();
    for (auto door_entity : door_view) {
        const auto& door_pos = door_view.get<Position>(door_entity);
        if (door_pos.x == new_x && door_pos.y == new_y) {
            // Found a closed door - queue an OpenDoorAction instead
            action_queue.push(std::make_unique<OpenDoorAction>(actor_, new_x, new_y));
            return {ActionResult::Status::Alternative, 0};
        }
    }

    // Check if the terrain is passable
    if (!terrain->passable) {
        return {ActionResult::Status::Failure, 0};
    }

    // Check for entities that block movement
    auto blocking_view = registry.view<Position, BlocksMovement>();
    for (auto blocking_entity : blocking_view) {
        const auto& blocking_pos = blocking_view.get<Position>(blocking_entity);
        if (blocking_pos.x == new_x && blocking_pos.y == new_y) {
            // Position is blocked by another entity
            return {ActionResult::Status::Failure, 0};
        }
    }

    // Calculate cooldown cost based on movement type
    bool is_diagonal = (dx_ != 0 && dy_ != 0);
    int cost = is_diagonal ? constants::MOVE_COST_DIAGONAL : constants::MOVE_COST_ORTHOGONAL;

    // Move is valid - execute it
    pos->x = new_x;
    pos->y = new_y;
    return {ActionResult::Status::Success, cost};
}

std::string MoveAction::description() const {
    std::ostringstream oss;
    oss << "Move (" << dx_ << ", " << dy_ << ")";
    return oss.str();
}

// OpenDoorAction implementation
ActionResult OpenDoorAction::execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) {
    auto& registry = world.get_registry();

    // Find the door at the target position
    auto door_view = registry.view<Position, Door, BlocksMovement>();
    for (auto door_entity : door_view) {
        const auto& door_pos = door_view.get<Position>(door_entity);
        if (door_pos.x == x_ && door_pos.y == y_) {
            auto& door = door_view.get<Door>(door_entity);

            // Open the door
            door.is_open = true;

            // Update the door's renderable to show open state
            auto* renderable = registry.try_get<Renderable>(door_entity);
            if (renderable) {
                renderable->glyph = door.open_glyph;
            }

            // Remove blocking components when door is opened
            registry.remove<BlocksMovement>(door_entity);
            registry.remove<BlocksVision>(door_entity);

            return {ActionResult::Status::Success, constants::ACTION_COST_OPEN_DOOR};
        }
    }

    // No door found at this position
    return {ActionResult::Status::Invalid, 0};
}

std::string OpenDoorAction::description() const {
    std::ostringstream oss;
    oss << "Open door at (" << x_ << ", " << y_ << ")";
    return oss.str();
}

// CloseDoorAction implementation
ActionResult CloseDoorAction::execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) {
    auto& registry = world.get_registry();

    // Find an open door at the target position
    auto door_view = registry.view<Position, Door>();
    for (auto door_entity : door_view) {
        const auto& door_pos = door_view.get<Position>(door_entity);
        auto& door = door_view.get<Door>(door_entity);

        if (door_pos.x == x_ && door_pos.y == y_ && door.is_open) {
            // Check if any entity is standing on the door
            auto entity_view = registry.view<Position>();
            for (auto entity : entity_view) {
                // Skip the door entity itself
                if (entity == door_entity) continue;

                const auto& pos = entity_view.get<Position>(entity);
                if (pos.x == x_ && pos.y == y_) {
                    // Can't close door with entity in the way
                    return {ActionResult::Status::Failure, 0};
                }
            }

            // Close the door (using mutable reference obtained from view)
            door.is_open = false;

            // Update the door's renderable to show closed state
            auto* renderable = registry.try_get<Renderable>(door_entity);
            if (renderable) {
                renderable->glyph = door.closed_glyph;
            }

            // Add blocking components back
            registry.emplace<BlocksMovement>(door_entity);
            registry.emplace<BlocksVision>(door_entity);

            return {ActionResult::Status::Success, constants::ACTION_COST_CLOSE_DOOR};
        }
    }

    // No open door found at this position
    return {ActionResult::Status::Invalid, 0};
}

std::string CloseDoorAction::description() const {
    std::ostringstream oss;
    oss << "Close door at (" << x_ << ", " << y_ << ")";
    return oss.str();
}

// WaitAction implementation
ActionResult WaitAction::execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) {
    // Simply pass the turn
    return {ActionResult::Status::Success, constants::ACTION_COST_WAIT};
}

std::string WaitAction::description() const {
    return "Wait";
}

} // namespace engine