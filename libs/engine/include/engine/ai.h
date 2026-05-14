#pragma once

#include "action.h"
#include "components.h"
#include "faction.h"
#include "math.h"
#include "well512.h"
#include "world.h"
#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <vector>

namespace engine::systems {

// Pick one of nine relative directions {-1,0,1} x {-1,0,1} that minimizes
// (or maximizes, with flee=true) distance to the given target. Returns the
// direction as a Vector2; (0,0) means "no move improves the situation, wait".
inline Vector2 choose_step_toward(const World& world,
                                  const Vector2& from,
                                  const Vector2& target,
                                  bool flee = false) {
    int best_dx = 0;
    int best_dy = 0;
    int best_dsq = from.distance_squared(target);

    for (int sy = -1; sy <= 1; ++sy) {
        for (int sx = -1; sx <= 1; ++sx) {
            if (sx == 0 && sy == 0) continue;
            int nx = from.x + sx;
            int ny = from.y + sy;
            if (!world.is_passable(nx, ny)) continue;
            int new_dsq = Vector2(nx, ny).distance_squared(target);
            bool better = flee ? (new_dsq > best_dsq) : (new_dsq < best_dsq);
            if (better) {
                best_dsq = new_dsq;
                best_dx = sx;
                best_dy = sy;
            }
        }
    }

    return {best_dx, best_dy};
}

// Plan and queue one action for a single non-player actor.
// Behavior (MVP):
//   1. Scan all entities in vision_range. Track nearest ATTACK target and
//      nearest FLEE source per the actor's faction relationships.
//   2. If an attack target exists:
//        - Adjacent (Chebyshev distance <= 1): queue AttackAction.
//        - Otherwise: greedy step toward target, or WaitAction if blocked.
//   3. Else if a flee source exists: greedy step away.
//   4. Else idle: ~50% Wait, ~50% step in a random passable direction.
//
// Vision is range-only (Euclidean^2) — no line-of-sight for the MVP.
inline void ai_take_turn(World& world, entt::entity actor, WELL512& rng) {
    auto& registry = world.get_registry();

    auto* actor_pos = registry.try_get<Position>(actor);
    auto* actor_faction_comp = registry.try_get<FactionComponent>(actor);
    if (!actor_pos || !actor_faction_comp) {
        // Can't reason about this actor; spend the turn waiting so the queue
        // makes progress.
        world.queue_action(std::make_unique<WaitAction>(actor));
        return;
    }

    const auto* faction_registry = world.get_faction_registry();
    if (!faction_registry) {
        world.queue_action(std::make_unique<WaitAction>(actor));
        return;
    }
    auto actor_faction = faction_registry->get(actor_faction_comp->faction_id);
    if (!actor_faction) {
        world.queue_action(std::make_unique<WaitAction>(actor));
        return;
    }

    const auto* vision = registry.try_get<VisionComponent>(actor);
    const int vision_range = vision ? vision->range : 8;
    const int vision_range_sq = vision_range * vision_range;

    const Vector2 origin{actor_pos->x, actor_pos->y};

    entt::entity attack_target = entt::null;
    entt::entity flee_source = entt::null;
    int best_attack_dsq = std::numeric_limits<int>::max();
    int best_flee_dsq = std::numeric_limits<int>::max();

    auto view = registry.view<Position, FactionComponent>();
    for (auto other : view) {
        if (other == actor) continue;
        const auto& other_pos = view.get<Position>(other);
        const Vector2 op{other_pos.x, other_pos.y};
        const int dsq = origin.distance_squared(op);
        if (dsq > vision_range_sq) continue;

        const auto& other_faction = view.get<FactionComponent>(other);
        const auto response = actor_faction->get_response_to(other_faction.faction_id);
        if (response == FactionResponse::ATTACK && dsq < best_attack_dsq) {
            attack_target = other;
            best_attack_dsq = dsq;
        } else if (response == FactionResponse::FLEE && dsq < best_flee_dsq) {
            flee_source = other;
            best_flee_dsq = dsq;
        }
    }

    if (attack_target != entt::null) {
        const auto& tpos = registry.get<Position>(attack_target);
        const int dx = tpos.x - origin.x;
        const int dy = tpos.y - origin.y;
        const bool adjacent = std::abs(dx) <= 1 && std::abs(dy) <= 1 && (dx != 0 || dy != 0);
        if (adjacent) {
            world.queue_action(std::make_unique<AttackAction>(actor, attack_target));
            return;
        }
        const Vector2 step = choose_step_toward(world, origin, {tpos.x, tpos.y}, /*flee=*/false);
        if (step.x == 0 && step.y == 0) {
            world.queue_action(std::make_unique<WaitAction>(actor));
        } else {
            world.queue_action(std::make_unique<MoveAction>(actor, step.x, step.y));
        }
        return;
    }

    if (flee_source != entt::null) {
        const auto& spos = registry.get<Position>(flee_source);
        const Vector2 step = choose_step_toward(world, origin, {spos.x, spos.y}, /*flee=*/true);
        if (step.x == 0 && step.y == 0) {
            world.queue_action(std::make_unique<WaitAction>(actor));
        } else {
            world.queue_action(std::make_unique<MoveAction>(actor, step.x, step.y));
        }
        return;
    }

    // Idle: half the time wait, half the time wander into a passable tile.
    if (rng.uniform() < 0.5) {
        world.queue_action(std::make_unique<WaitAction>(actor));
        return;
    }

    static constexpr int dx_table[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
    static constexpr int dy_table[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
    const int dir = rng.range(0, 8);
    const int dx = dx_table[dir];
    const int dy = dy_table[dir];
    if ((dx == 0 && dy == 0) || !world.is_passable(origin.x + dx, origin.y + dy)) {
        world.queue_action(std::make_unique<WaitAction>(actor));
    } else {
        world.queue_action(std::make_unique<MoveAction>(actor, dx, dy));
    }
}

} // namespace engine::systems
