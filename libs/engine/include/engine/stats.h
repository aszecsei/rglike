#pragma once

#include "well512.h"
#include <string>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace engine {

// Core stat categories
enum class StatCategory {
    PHYSICAL,
    MENTAL,
    SPIRITUAL,
    SOCIAL
};

// Stat types within each category
enum class StatType {
    BLUNT,
    FINESSE,
    DEFENSE
};

// All twelve core stats
enum class CoreStat {
    // Physical
    STRENGTH,       // Physical Blunt
    DEXTERITY,      // Physical Finesse
    CONSTITUTION,   // Physical Defense

    // Mental
    INTELLIGENCE,   // Mental Blunt
    CUNNING,        // Mental Finesse
    FOCUS,          // Mental Defense

    // Spiritual
    FAITH,          // Spiritual Blunt
    ATTUNEMENT,     // Spiritual Finesse
    RESILIENCE,     // Spiritual Defense

    // Social
    PRESENCE,       // Social Blunt
    CHARISMA,       // Social Finesse
    COMPOSURE       // Social Defense
};

// Resource pools derived from stats
enum class ResourcePool {
    HEALTH,         // Physical vitality
    STAMINA,        // Physical energy
    MANA,           // Spiritual energy
    FOCUS_POINTS,   // Mental energy
    WILLPOWER       // Social/Mental resistance
};

// Get stat category from core stat
constexpr StatCategory get_stat_category(CoreStat stat) {
    switch (stat) {
        case CoreStat::STRENGTH:
        case CoreStat::DEXTERITY:
        case CoreStat::CONSTITUTION:
            return StatCategory::PHYSICAL;

        case CoreStat::INTELLIGENCE:
        case CoreStat::CUNNING:
        case CoreStat::FOCUS:
            return StatCategory::MENTAL;

        case CoreStat::FAITH:
        case CoreStat::ATTUNEMENT:
        case CoreStat::RESILIENCE:
            return StatCategory::SPIRITUAL;

        case CoreStat::PRESENCE:
        case CoreStat::CHARISMA:
        case CoreStat::COMPOSURE:
            return StatCategory::SOCIAL;
    }
    return StatCategory::PHYSICAL;
}

// Get stat type from core stat
constexpr StatType get_stat_type(CoreStat stat) {
    switch (stat) {
        case CoreStat::STRENGTH:
        case CoreStat::INTELLIGENCE:
        case CoreStat::FAITH:
        case CoreStat::PRESENCE:
            return StatType::BLUNT;

        case CoreStat::DEXTERITY:
        case CoreStat::CUNNING:
        case CoreStat::ATTUNEMENT:
        case CoreStat::CHARISMA:
            return StatType::FINESSE;

        case CoreStat::CONSTITUTION:
        case CoreStat::FOCUS:
        case CoreStat::RESILIENCE:
        case CoreStat::COMPOSURE:
            return StatType::DEFENSE;
    }
    return StatType::BLUNT;
}

// Get string name for core stat
constexpr const char* get_stat_name(CoreStat stat) {
    switch (stat) {
        case CoreStat::STRENGTH: return "Strength";
        case CoreStat::DEXTERITY: return "Dexterity";
        case CoreStat::CONSTITUTION: return "Constitution";
        case CoreStat::INTELLIGENCE: return "Intelligence";
        case CoreStat::CUNNING: return "Wisdom";
        case CoreStat::FOCUS: return "Focus";
        case CoreStat::FAITH: return "Power";
        case CoreStat::ATTUNEMENT: return "Attunement";
        case CoreStat::RESILIENCE: return "Resilience";
        case CoreStat::PRESENCE: return "Presence";
        case CoreStat::CHARISMA: return "Charisma";
        case CoreStat::COMPOSURE: return "Composure";
    }
    return "Unknown";
}

// Get string name for resource pool
constexpr const char* get_resource_name(ResourcePool pool) {
    switch (pool) {
        case ResourcePool::HEALTH: return "Health";
        case ResourcePool::STAMINA: return "Stamina";
        case ResourcePool::MANA: return "Mana";
        case ResourcePool::FOCUS_POINTS: return "Focus Points";
        case ResourcePool::WILLPOWER: return "Willpower";
    }
    return "Unknown";
}

// Stats component - holds all character stats
struct Stats {
    // Core stats (0-100 range)
    std::unordered_map<CoreStat, int> core_stats;

    // Resource pools (current/max)
    std::unordered_map<ResourcePool, int> current_resources;
    std::unordered_map<ResourcePool, int> max_resources;

    // Experience and level
    int experience = 0;
    int level = 1;
    int experience_to_next_level = 100;

    Stats() {
        // Initialize all core stats to 10 (baseline)
        core_stats[CoreStat::STRENGTH] = 10;
        core_stats[CoreStat::DEXTERITY] = 10;
        core_stats[CoreStat::CONSTITUTION] = 10;
        core_stats[CoreStat::INTELLIGENCE] = 10;
        core_stats[CoreStat::CUNNING] = 10;
        core_stats[CoreStat::FOCUS] = 10;
        core_stats[CoreStat::FAITH] = 10;
        core_stats[CoreStat::ATTUNEMENT] = 10;
        core_stats[CoreStat::RESILIENCE] = 10;
        core_stats[CoreStat::PRESENCE] = 10;
        core_stats[CoreStat::CHARISMA] = 10;
        core_stats[CoreStat::COMPOSURE] = 10;

        // Initialize resources
        calculate_max_resources();
        refill_resources();
    }

    // Get a core stat value (clamped to 0-100)
    [[nodiscard]] int get_stat(CoreStat stat) const {
        auto it = core_stats.find(stat);
        if (it == core_stats.end()) return 0;
        return std::clamp(it->second, 0, 100);
    }

    // Set a core stat value (clamped to 0-100)
    void set_stat(CoreStat stat, int value) {
        core_stats[stat] = std::clamp(value, 0, 100);
        calculate_max_resources();
    }

    // Modify a core stat by a delta
    void modify_stat(CoreStat stat, int delta) {
        set_stat(stat, get_stat(stat) + delta);
    }

    // Calculate maximum resource pools based on core stats
    void calculate_max_resources() {
        // Health = Constitution * 10 + Strength * 2
        max_resources[ResourcePool::HEALTH] =
            get_stat(CoreStat::CONSTITUTION) * 10 + get_stat(CoreStat::STRENGTH) * 2;

        // Stamina = Constitution * 5 + Dexterity * 3
        max_resources[ResourcePool::STAMINA] =
            get_stat(CoreStat::CONSTITUTION) * 5 + get_stat(CoreStat::DEXTERITY) * 3;

        // Mana = Power * 10 + Attunement * 2
        max_resources[ResourcePool::MANA] =
            get_stat(CoreStat::FAITH) * 10 + get_stat(CoreStat::ATTUNEMENT) * 2;

        // Focus Points = Intelligence * 5 + Wisdom * 3
        max_resources[ResourcePool::FOCUS_POINTS] =
            get_stat(CoreStat::INTELLIGENCE) * 5 + get_stat(CoreStat::CUNNING) * 3;

        // Willpower = Composure * 5 + Focus * 3 + Resilience * 2
        max_resources[ResourcePool::WILLPOWER] =
            get_stat(CoreStat::COMPOSURE) * 5 + get_stat(CoreStat::FOCUS) * 3 + get_stat(CoreStat::RESILIENCE) * 2;

        // Clamp current resources to new max
        for (const auto& [pool, max_val] : max_resources) {
            auto& current = current_resources[pool];
            if (current > max_val) {
                current = max_val;
            }
        }
    }

    // Refill all resources to maximum
    void refill_resources() {
        for (const auto& [pool, max_val] : max_resources) {
            current_resources[pool] = max_val;
        }
    }

    // Get current resource value
    [[nodiscard]] int get_resource(ResourcePool pool) const {
        auto it = current_resources.find(pool);
        if (it == current_resources.end()) return 0;
        return it->second;
    }

    // Get max resource value
    [[nodiscard]] int get_max_resource(ResourcePool pool) const {
        auto it = max_resources.find(pool);
        if (it == max_resources.end()) return 0;
        return it->second;
    }

    // Modify a resource (clamped to 0-max)
    void modify_resource(ResourcePool pool, int delta) {
        int& current = current_resources[pool];
        int max_val = get_max_resource(pool);
        current = std::clamp(current + delta, 0, max_val);
    }

    // Set a resource directly (clamped to 0-max)
    void set_resource(ResourcePool pool, int value) {
        int max_val = get_max_resource(pool);
        current_resources[pool] = std::clamp(value, 0, max_val);
    }

    // Add experience and check for level up
    // Returns true if leveled up
    bool add_experience(int amount) {
        experience += amount;
        if (experience >= experience_to_next_level) {
            return true; // Signal level up needed
        }
        return false;
    }

    // Perform level up - increases level and calculates next level XP requirement
    void level_up() {
        level++;
        experience -= experience_to_next_level;
        // Exponential XP curve: 100 * (1.5 ^ (level - 1))
        experience_to_next_level = static_cast<int>(100.0 * std::pow(1.5, level - 1));
    }

    // Get experience progress as percentage (0.0 - 1.0)
    [[nodiscard]] float get_experience_progress() const {
        if (experience_to_next_level <= 0) return 1.0f;
        return static_cast<float>(experience) / static_cast<float>(experience_to_next_level);
    }
};

// Stat growth pattern for classes/archetypes
struct StatGrowthPattern {
    std::string name;
    std::unordered_map<CoreStat, float> growth_rates; // 0.0 to 1.0, determines random stat increase on level up

    // Apply this growth pattern on level up using WELL512 RNG
    // Returns the stats that were increased
    std::unordered_map<CoreStat, int> apply_level_up(Stats& stats, WELL512& rng) const {
        std::unordered_map<CoreStat, int> increases;

        for (const auto& [stat, rate] : growth_rates) {
            // Random chance based on growth rate
            double roll = rng.uniform();
            if (roll < rate) {
                // Random increase of 1-3 points
                int increase = rng.range(1, 3);
                stats.modify_stat(stat, increase);
                increases[stat] = increase;
            }
        }

        return increases;
    }
};

} // namespace engine