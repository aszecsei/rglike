#include <engine/action.h>
#include <engine/world.h>
#include <engine/components.h>
#include <engine/constants.h>
#include <engine/faction.h>
#include <engine/stats.h>
#include <ftxui/screen/color.hpp>
#include <algorithm>
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

    // Bump-attack: if a combatant occupies the target tile and the actor's
    // faction wants to attack them, swap this move for an AttackAction. Mirrors
    // the door-bump pattern above.
    const auto* faction_registry = world.get_faction_registry();
    const auto* actor_faction_comp = registry.try_get<FactionComponent>(actor_);
    if (faction_registry && actor_faction_comp) {
        auto combatant_view = registry.view<Position, StatsComponent, FactionComponent>();
        for (auto target_entity : combatant_view) {
            if (target_entity == actor_) continue;
            const auto& target_pos = combatant_view.get<Position>(target_entity);
            if (target_pos.x != new_x || target_pos.y != new_y) continue;

            const auto& target_faction_comp = combatant_view.get<FactionComponent>(target_entity);
            auto actor_faction = faction_registry->get(actor_faction_comp->faction_id);
            if (!actor_faction) break;

            if (actor_faction->get_response_to(target_faction_comp.faction_id) == FactionResponse::ATTACK) {
                action_queue.push(std::make_unique<AttackAction>(actor_, target_entity));
                return {ActionResult::Status::Alternative, 0};
            }
            break;  // Only one entity can occupy a tile in practice.
        }
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

// Helper: return the entity's display name, or a generic fallback.
static std::string display_name(const entt::registry& registry, entt::entity e) {
    const auto* nc = registry.try_get<NameComponent>(e);
    return nc ? nc->name : std::string{"something"};
}

// AttackAction implementation
ActionResult AttackAction::execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) {
    auto& registry = world.get_registry();

    if (!registry.valid(actor_) || !registry.valid(target_)) {
        return {ActionResult::Status::Invalid, 0};
    }

    auto* attacker_stats = registry.try_get<StatsComponent>(actor_);
    auto* target_stats = registry.try_get<StatsComponent>(target_);
    auto* target_pos = registry.try_get<Position>(target_);
    if (!attacker_stats || !target_stats || !target_pos) {
        return {ActionResult::Status::Invalid, 0};
    }

    // Damage formula (MVP): STR-driven, halved by target CON, floor of 1.
    int attacker_str = attacker_stats->get_stat(CoreStat::STRENGTH);
    int target_con = target_stats->get_stat(CoreStat::CONSTITUTION);
    int damage = std::max(1, attacker_str - target_con / 2);

    target_stats->modify_resource(ResourcePool::HEALTH, -damage);

    const bool attacker_is_player = registry.all_of<Player>(actor_);
    const bool target_is_player = registry.all_of<Player>(target_);
    const std::string attacker_name = display_name(registry, actor_);
    const std::string target_name = display_name(registry, target_);

    auto& log = world.get_game_log();
    if (attacker_is_player) {
        log.entry()
           .color(ftxui::Color::Yellow).text("You strike ").bold().text(target_name).reset_style()
           .color(ftxui::Color::Yellow).text(" for ").bold().text(std::to_string(damage)).reset_style()
           .color(ftxui::Color::Yellow).text(" damage.")
           .log();
    } else if (target_is_player) {
        log.entry()
           .color(ftxui::Color::Red).bold().text(attacker_name).reset_style()
           .color(ftxui::Color::Red).text(" strikes you for ").bold().text(std::to_string(damage)).reset_style()
           .color(ftxui::Color::Red).text(" damage.")
           .log();
    } else {
        log.entry().dim().text(attacker_name + " strikes " + target_name + ".").log();
    }

    const int target_hp = target_stats->get_resource(ResourcePool::HEALTH);
    if (target_hp <= 0) {
        if (target_is_player) {
            // Player death is sticky state on World; the scene transitions on
            // its next update tick. Don't destroy the entity — the game-over
            // scene needs the final stats snapshot.
            world.set_player_dead(attacker_name);
            log.entry().color(ftxui::Color::Red).bold().text(attacker_name + " slays you!").log();
        } else {
            // Mob death: bloodstain, destroy entity, award XP.
            if (auto* map_comp = world.get_map_component()) {
                int idx = target_pos->y * map_comp->map.width + target_pos->x;
                map_comp->map.bloodstains.insert(idx);
            }

            if (attacker_is_player) {
                log.entry()
                   .color(ftxui::Color::Green).text("You slay ").bold().text(target_name).reset_style()
                   .color(ftxui::Color::Green).text("!").log();
            } else {
                log.entry().dim().text(attacker_name + " slays " + target_name + ".").log();
            }

            // XP and pending level-up tag on the attacker.
            const bool leveled = attacker_stats->add_experience(constants::XP_REWARD_PER_KILL);
            if (attacker_is_player) {
                log.entry().color(ftxui::Color::GreenLight)
                   .text("(+" + std::to_string(constants::XP_REWARD_PER_KILL) + " XP)")
                   .log();
            }
            if (leveled) {
                // Tag the attacker; the scene applies the growth pattern.
                if (!registry.all_of<PendingLevelUp>(actor_)) {
                    registry.emplace<PendingLevelUp>(actor_);
                } else {
                    registry.get<PendingLevelUp>(actor_).levels += 1;
                }
            }

            registry.destroy(target_);
        }
    }

    return {ActionResult::Status::Success, constants::ACTION_COST_ATTACK};
}

std::string AttackAction::description() const {
    std::ostringstream oss;
    oss << "Attack entity " << static_cast<uint32_t>(target_);
    return oss.str();
}

} // namespace engine