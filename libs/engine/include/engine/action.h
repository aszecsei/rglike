#pragma once

#include <entt/entt.hpp>
#include <memory>
#include <queue>

namespace engine {

// Forward declarations
class World;

/**
 * @brief Result of executing an action.
 *
 * Actions return a status indicating how the game should proceed:
 * - Success: Action succeeded, apply cooldown cost and consume turn
 * - Failure: Action failed, apply cooldown cost and consume turn (e.g., bumped into wall)
 * - Alternative: Action queued alternative actions without consuming turn yet
 *   (e.g., MoveAction queued OpenDoorAction when encountering closed door)
 * - Invalid: Action is invalid, don't consume turn (e.g., missing components)
 */
struct ActionResult {
    enum class Status {
        Success,        // Action succeeded, consume the turn
        Failure,        // Action failed, consume the turn
        Alternative,    // Action failed but queued alternative actions (don't consume turn yet)
        Invalid         // Action is invalid, don't consume turn
    };

    Status status;
    int cooldown_cost = 0;  // How much cooldown to add after this action (see constants.h)

    ActionResult(Status s, int cost = 0) : status(s), cooldown_cost(cost) {}
};

// Base class for all game actions
class Action {
public:
    explicit Action(entt::entity actor) : actor_(actor) {}
    virtual ~Action() = default;

    // Execute the action
    // Returns the result and may queue additional actions
    virtual ActionResult execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) = 0;

    // Get a description of the action for logging
    [[nodiscard]] virtual std::string description() const = 0;

    // Get the entity performing this action
    [[nodiscard]] entt::entity get_actor() const { return actor_; }

protected:
    entt::entity actor_;
};

// Action: Try to move an entity
class MoveAction : public Action {
public:
    MoveAction(entt::entity entity, int dx, int dy)
        : Action(entity), dx_(dx), dy_(dy) {}

    ActionResult execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) override;
    [[nodiscard]] std::string description() const override;

private:
    int dx_;
    int dy_;
};

// Action: Try to open a door
class OpenDoorAction : public Action {
public:
    OpenDoorAction(entt::entity actor, int x, int y)
        : Action(actor), x_(x), y_(y) {}

    ActionResult execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) override;
    [[nodiscard]] std::string description() const override;

private:
    int x_;
    int y_;
};

// Action: Try to close a door
class CloseDoorAction : public Action {
public:
    CloseDoorAction(entt::entity actor, int x, int y)
        : Action(actor), x_(x), y_(y) {}

    ActionResult execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) override;
    [[nodiscard]] std::string description() const override;

private:
    int x_;
    int y_;
};

// Action: Wait/skip turn
class WaitAction : public Action {
public:
    explicit WaitAction(entt::entity entity) : Action(entity) {}

    ActionResult execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) override;
    [[nodiscard]] std::string description() const override;
};

// Action: Melee-attack a specific target entity. Damage is derived from the
// attacker's STR and the target's CON via the StatsComponent on each side.
// Both entities must have StatsComponent; AttackAction is queued only from
// places that have already verified this (e.g., MoveAction's bump-attack).
class AttackAction : public Action {
public:
    AttackAction(entt::entity attacker, entt::entity target)
        : Action(attacker), target_(target) {}

    ActionResult execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) override;
    [[nodiscard]] std::string description() const override;

private:
    entt::entity target_;
};

} // namespace engine