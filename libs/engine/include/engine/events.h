#pragma once

#include "stats.h"
#include <entt/entt.hpp>
#include <unordered_map>

namespace engine {

// Event emitted when an entity levels up
struct LevelUpEvent {
    entt::entity entity;                        // The entity that leveled up
    int new_level;                              // The new level
    std::unordered_map<CoreStat, int> stat_increases;  // Stats that were increased and by how much
};

// Event emitted when an entity gains experience
struct ExperienceGainEvent {
    entt::entity entity;                        // The entity that gained XP
    int amount;                                 // Amount of XP gained
    int new_total;                              // New total XP
    bool leveled_up;                            // Whether this triggered a level up
};

// Event emitted when an entity takes damage
struct DamageEvent {
    entt::entity attacker;                      // The attacking entity
    entt::entity target;                        // The target entity
    int damage;                                 // Amount of damage dealt
    bool killed;                                // Whether this killed the target
};

// Event emitted when an entity dies
struct DeathEvent {
    entt::entity entity;                        // The entity that died
    entt::entity killer;                        // The entity that dealt the killing blow (optional)
    int xp_reward;                              // XP awarded to the killer
};

} // namespace engine