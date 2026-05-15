#pragma once

#include "equipment.h"
#include "registry.h"
#include "stats.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

// Race definition - provides base stat modifiers and description
struct Race {
    std::string id;
    std::string name;
    std::string description;

    // Base stat modifiers (applied to starting stats)
    std::unordered_map<CoreStat, int> stat_modifiers;

    Race() = default;
};

// Character class definition - provides growth pattern and description
struct CharacterClass {
    std::string id;
    std::string name;
    std::string description;

    // Growth pattern ID for leveling up
    std::string growth_pattern_id;

    // Starting stat bonuses (in addition to race)
    std::unordered_map<CoreStat, int> starting_stats;

    // Items the character begins wearing. Keys are physical slots, so
    // rings explicitly target ring_1 / ring_2. Each value is an Item
    // template id; missing/unknown ids are logged and skipped at spawn.
    std::unordered_map<EquipmentSlot, std::string> starting_equipment;

    // Items the character begins with in the bag, in spawn order. Each
    // entry is an Item template id and produces one slot (or one stack
    // increment for stackable items).
    std::vector<std::string> starting_inventory;

    CharacterClass() = default;
};

// Type aliases for registries
using RaceRegistry = Registry<Race>;
using CharacterClassRegistry = Registry<CharacterClass>;

// Character creation data - stores player choices during character creation
struct CharacterCreationData {
    std::string name = "";
    std::string gender = "neutral";  // "male", "female", "neutral", or custom
    std::string race_id = "";
    std::string class_id = "";
};

// Build the initial Stats for a freshly created character.
// Starts from baseline 10s, applies race stat_modifiers, then class starting_stats,
// then refills all resources to the final max so the character begins at full health.
inline Stats build_initial_stats(const CharacterCreationData& /*data*/,
                                  const Race* race,
                                  const CharacterClass* cls) {
    Stats stats;  // Baseline 10s, resources filled.

    if (race) {
        for (const auto& [stat, modifier] : race->stat_modifiers) {
            stats.modify_stat(stat, modifier);
        }
    }

    if (cls) {
        for (const auto& [stat, bonus] : cls->starting_stats) {
            stats.modify_stat(stat, bonus);
        }
    }

    // modify_stat recalculates maxes incrementally but leaves current resources
    // below the new max when bonuses raise them. Refill so the character starts
    // at full health/mana/etc.
    stats.refill_resources();

    return stats;
}

} // namespace engine