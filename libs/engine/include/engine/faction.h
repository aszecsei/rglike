#pragma once

#include "registry.h"
#include "components.h"
#include <string>
#include <unordered_map>

namespace engine {

// Special faction ID constants
constexpr const char* FACTION_DEFAULT = "DEFAULT";  // Fallback response for unspecified factions
constexpr const char* FACTION_SELF = "SELF";        // Response to members of the same faction

// Faction definition - defines a faction and its relationships with other factions
struct Faction {
    std::string id;                                              // Unique faction identifier
    std::string name;                                            // Display name
    std::unordered_map<std::string, FactionResponse> responses;  // Map of faction_id -> response

    Faction() = default;
    Faction(std::string id, std::string name)
        : id(std::move(id)), name(std::move(name)) {}

    // Get response to another faction, with fallback to DEFAULT and then IGNORE
    [[nodiscard]] FactionResponse get_response_to(const std::string& other_faction_id) const {
        // Check for SELF first
        if (other_faction_id == id) {
            auto it = responses.find(FACTION_SELF);
            if (it != responses.end()) {
                return it->second;
            }
        }

        // Check for specific faction
        auto it = responses.find(other_faction_id);
        if (it != responses.end()) {
            return it->second;
        }

        // Fall back to DEFAULT
        auto default_it = responses.find(FACTION_DEFAULT);
        if (default_it != responses.end()) {
            return default_it->second;
        }

        // Ultimate fallback
        return FactionResponse::IGNORE;
    }

    // Set response to a faction
    void set_response(const std::string& faction_id, FactionResponse response) {
        responses[faction_id] = response;
    }
};

// Type alias for faction registry
using FactionRegistry = Registry<Faction>;

} // namespace engine