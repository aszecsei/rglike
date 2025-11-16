#pragma once

#include "registry.h"
#include "stats.h"
#include <string>
#include <unordered_map>

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

    // Generate initial stats based on race and class
    Stats generate_stats() const {
        Stats stats;
        // Stats start at 10 baseline by default
        return stats;
    }
};

} // namespace engine