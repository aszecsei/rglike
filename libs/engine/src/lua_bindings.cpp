#include <engine/lua_bindings.h>
#include <engine/engine.h>
#include <engine/terrain.h>
#include <engine/mob.h>
#include <engine/faction.h>
#include <engine/character_data.h>
#include <engine/growth_pattern.h>
#include <fstream>
#include <vector>

namespace engine {

// Metadata for Lua function documentation
struct LuaFunctionParam {
    std::string name;
    std::string type;
    std::string description;
};

struct LuaFunctionDoc {
    std::string table_name;  // Empty for global functions
    std::string name;
    std::string description;
    std::vector<LuaFunctionParam> params;
};

// Store all function documentation
static std::vector<LuaFunctionDoc> function_docs;

// Helper function to parse faction response string
static std::optional<FactionResponse> parse_faction_response(const std::string& response_str,
                                                               std::shared_ptr<spdlog::logger> logger,
                                                               const std::string& context = "") {
    if (response_str == "IGNORE") {
        return FactionResponse::IGNORE;
    } else if (response_str == "ATTACK") {
        return FactionResponse::ATTACK;
    } else if (response_str == "FLEE") {
        return FactionResponse::FLEE;
    } else {
        logger->error("{}: Invalid response '{}' (must be IGNORE, ATTACK, or FLEE)",
                     context.empty() ? "parse_faction_response" : context, response_str);
        return std::nullopt;
    }
}

// Helper function to parse RGB color from Lua table
static ftxui::Color parse_rgb_color(sol::table color_table) {
    return ftxui::Color::RGB(color_table[1], color_table[2], color_table[3]);
}

// Helper to register a function in a table with its documentation
template<typename Func>
void register_table_function(sol::table& table, const std::string& table_name,
                             const std::string& name, Func&& func, LuaFunctionDoc doc) {
    table.set_function(name, std::forward<Func>(func));
    doc.table_name = table_name;
    doc.name = name;
    function_docs.push_back(std::move(doc));
}

void LuaBindings::initialize(sol::state& lua, Engine* engine, std::shared_ptr<spdlog::logger> logger) {
    // Clear previous documentation
    function_docs.clear();

    // Open standard Lua libraries
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::string);

    // Register Faction usertype
    lua.new_usertype<Faction>("Faction",
        sol::no_constructor,
        "id", &Faction::id,
        "name", &Faction::name,
        "SetResponse", [engine, logger](Faction& faction, const std::string& target_faction_id, const std::string& response_str) {
            auto response = parse_faction_response(response_str, logger, "Faction.SetResponse");
            if (!response) return;

            faction.set_response(target_faction_id, *response);
            engine->get_faction_registry().register_item(faction.id, faction);

            logger->debug("Set faction '{}' response to '{}': {}", faction.id, target_faction_id, response_str);
        }
    );

    // Create Engine table for engine-related functions
    auto engine_table = lua.create_named_table("Engine");

    // Bind CreateTerrain function in Engine namespace
    register_table_function(engine_table, "Engine", "CreateTerrain",
        [engine, logger](sol::table terrain_table) {
            std::string id = terrain_table["id"];
            std::string glyph = terrain_table["glyph"];

            // Foreground color (required)
            ftxui::Color fg_color = parse_rgb_color(terrain_table["fg_color"]);

            // Middle ground color (optional)
            ftxui::Color mg_color = ftxui::Color::Default;
            sol::optional<sol::table> mg_color_opt = terrain_table["mg_color"];
            if (mg_color_opt.has_value()) { 
                mg_color = parse_rgb_color(*mg_color_opt); 
            }

            // Background color (optional)
            ftxui::Color bg_color = ftxui::Color::Default;
            sol::optional<sol::table> bg_color_opt = terrain_table["bg_color"];
            if (bg_color_opt.has_value()) {
                bg_color = parse_rgb_color(*bg_color_opt);
            }

            bool passable = terrain_table["passable"];
            bool blocks_vision = terrain_table["blocks_vision"];

            Terrain terrain;
            terrain.glyph = glyph;
            terrain.fg_color = fg_color;
            terrain.mg_color = mg_color;
            terrain.bg_color = bg_color;
            terrain.passable = passable;
            terrain.blocks_vision = blocks_vision;

            engine->get_terrain_registry().register_item(id, terrain);
            logger->debug("Registered terrain '{}' with glyph '{}' (passable={}, blocks_vision={})",
                         id, glyph, passable, blocks_vision);
        },
        LuaFunctionDoc{
            .description = "Create a terrain type",
            .params = {
                {"terrain_table", "table", "Table with fields: id, glyph, fg_color (RGB array), mg_color (optional RGB array), bg_color (optional RGB array), passable, blocks_vision"}
            }
        }
    );

    // Bind CreateMob function in Engine namespace
    register_table_function(engine_table, "Engine", "CreateMob",
        [engine, logger](sol::table mob_table) {
            std::string id = mob_table["id"];
            std::string name = mob_table["name"];
            std::string glyph = mob_table["glyph"];

            // Foreground color (required)
            ftxui::Color fg_color = parse_rgb_color(mob_table["fg_color"]);

            // Background color (optional)
            ftxui::Color bg_color = ftxui::Color::Default;
            sol::optional<sol::table> bg_color_opt = mob_table["bg_color"];
            if (bg_color_opt) {
                bg_color = parse_rgb_color(*bg_color_opt);
            }

            // Optional bold (defaults to false)
            bool bold = mob_table.get_or("bold", false);

            // Optional render_order (defaults to 50)
            int render_order = mob_table.get_or("render_order", 50);

            // Optional blocks_movement (defaults to true)
            bool blocks_movement = mob_table.get_or("blocks_movement", true);

            // Optional blocks_vision (defaults to false)
            bool blocks_vision = mob_table.get_or("blocks_vision", false);

            int vision_range = mob_table["vision_range"];

            // Optional faction_id (defaults to "neutral")
            std::string faction_id = mob_table.get_or<std::string>("faction_id", "neutral");

            Mob mob;
            mob.name = name;
            mob.glyph = glyph;
            mob.fg_color = fg_color;
            mob.bg_color = bg_color;
            mob.bold = bold;
            mob.render_order = render_order;
            mob.blocks_movement = blocks_movement;
            mob.blocks_vision = blocks_vision;
            mob.vision_range = vision_range;
            mob.faction_id = faction_id;

            // Check for stats table (new system) or legacy hp/defense/power
            sol::optional<sol::table> stats_opt = mob_table["stats"];
            if (stats_opt) {
                // New stats system
                sol::table stats_table = *stats_opt;
                Stats stats;

                // Set core stats if provided
                if (auto val = stats_table.get<sol::optional<int>>("strength")) stats.set_stat(CoreStat::STRENGTH, *val);
                if (auto val = stats_table.get<sol::optional<int>>("dexterity")) stats.set_stat(CoreStat::DEXTERITY, *val);
                if (auto val = stats_table.get<sol::optional<int>>("constitution")) stats.set_stat(CoreStat::CONSTITUTION, *val);
                if (auto val = stats_table.get<sol::optional<int>>("intelligence")) stats.set_stat(CoreStat::INTELLIGENCE, *val);
                if (auto val = stats_table.get<sol::optional<int>>("cunning")) stats.set_stat(CoreStat::CUNNING, *val);
                if (auto val = stats_table.get<sol::optional<int>>("focus")) stats.set_stat(CoreStat::FOCUS, *val);
                if (auto val = stats_table.get<sol::optional<int>>("faith")) stats.set_stat(CoreStat::FAITH, *val);
                if (auto val = stats_table.get<sol::optional<int>>("attunement")) stats.set_stat(CoreStat::ATTUNEMENT, *val);
                if (auto val = stats_table.get<sol::optional<int>>("resilience")) stats.set_stat(CoreStat::RESILIENCE, *val);
                if (auto val = stats_table.get<sol::optional<int>>("presence")) stats.set_stat(CoreStat::PRESENCE, *val);
                if (auto val = stats_table.get<sol::optional<int>>("charisma")) stats.set_stat(CoreStat::CHARISMA, *val);
                if (auto val = stats_table.get<sol::optional<int>>("composure")) stats.set_stat(CoreStat::COMPOSURE, *val);

                // Optional level range for random spawning
                mob.min_level = stats_table.get_or("min_level", 1);
                mob.max_level = stats_table.get_or("max_level", mob.min_level);

                mob.base_stats = stats;
                logger->debug("Registered mob '{}' ({}) with stats - Level range:{}-{} Base HP:{}",
                             id, name, mob.min_level, mob.max_level, stats.get_max_resource(ResourcePool::HEALTH));
            } else {
                // Legacy system - require hp/defense/power
                int max_hp = mob_table["max_hp"];
                int defense = mob_table["defense"];
                int power = mob_table["power"];

                mob.max_hp = max_hp;
                mob.hp = max_hp;
                mob.defense = defense;
                mob.power = power;

                logger->debug("Registered mob '{}' ({}) - HP:{} DEF:{} POW:{}",
                             id, name, max_hp, defense, power);
            }

            engine->get_mob_registry().register_item(id, mob);
        },
        LuaFunctionDoc{
            .description = "Create a mob type",
            .params = {
                {"mob_table", "table", "Table with fields: id, name, glyph, fg_color (RGB array), bg_color (optional RGB array), bold (optional), render_order (optional), blocks_movement (optional), blocks_vision (optional), vision_range, max_hp, defense, power, faction_id (optional)"}
            }
        }
    );

    // Bind CreateFaction function in Engine namespace
    register_table_function(engine_table, "Engine", "CreateFaction",
        [engine, logger](sol::table faction_table) -> Faction {
            std::string id = faction_table["id"];
            std::string name = faction_table["name"];

            Faction faction;
            faction.id = id;
            faction.name = name;

            // Process optional responses table
            sol::optional<sol::table> responses_opt = faction_table["responses"];
            if (responses_opt) {
                sol::table responses = *responses_opt;

                // Iterate over the responses table
                for (const auto& pair : responses) {
                    std::string target_faction_id = pair.first.as<std::string>();
                    std::string response_str = pair.second.as<std::string>();

                    auto response = parse_faction_response(response_str, logger, "CreateFaction");
                    if (!response) continue;

                    faction.set_response(target_faction_id, *response);
                    logger->debug("Set faction '{}' response to '{}': {}", id, target_faction_id, response_str);
                }
            }

            engine->get_faction_registry().register_item(id, faction);
            logger->debug("Registered faction '{}' ({})", id, name);

            return faction;
        },
        LuaFunctionDoc{
            .description = "Create a faction and return the faction object",
            .params = {
                {"faction_table", "table", "Table with fields: id, name, responses (optional table mapping target faction IDs to response strings)"}
            }
        }
    );

    // Bind CreateRace function
    register_table_function(engine_table, "Engine", "CreateRace",
        [engine, logger](sol::table race_table) {
            std::string id = race_table["id"];
            std::string name = race_table["name"];
            std::string description = race_table.get_or<std::string>("description", "");

            Race race;
            race.id = id;
            race.name = name;
            race.description = description;

            // Parse stat modifiers if provided
            sol::optional<sol::table> mods_opt = race_table["stat_modifiers"];
            if (mods_opt) {
                sol::table mods = *mods_opt;
                if (auto val = mods.get<sol::optional<int>>("strength")) race.stat_modifiers[CoreStat::STRENGTH] = *val;
                if (auto val = mods.get<sol::optional<int>>("dexterity")) race.stat_modifiers[CoreStat::DEXTERITY] = *val;
                if (auto val = mods.get<sol::optional<int>>("constitution")) race.stat_modifiers[CoreStat::CONSTITUTION] = *val;
                if (auto val = mods.get<sol::optional<int>>("intelligence")) race.stat_modifiers[CoreStat::INTELLIGENCE] = *val;
                if (auto val = mods.get<sol::optional<int>>("cunning")) race.stat_modifiers[CoreStat::CUNNING] = *val;
                if (auto val = mods.get<sol::optional<int>>("focus")) race.stat_modifiers[CoreStat::FOCUS] = *val;
                if (auto val = mods.get<sol::optional<int>>("faith")) race.stat_modifiers[CoreStat::FAITH] = *val;
                if (auto val = mods.get<sol::optional<int>>("attunement")) race.stat_modifiers[CoreStat::ATTUNEMENT] = *val;
                if (auto val = mods.get<sol::optional<int>>("resilience")) race.stat_modifiers[CoreStat::RESILIENCE] = *val;
                if (auto val = mods.get<sol::optional<int>>("presence")) race.stat_modifiers[CoreStat::PRESENCE] = *val;
                if (auto val = mods.get<sol::optional<int>>("charisma")) race.stat_modifiers[CoreStat::CHARISMA] = *val;
                if (auto val = mods.get<sol::optional<int>>("composure")) race.stat_modifiers[CoreStat::COMPOSURE] = *val;
            }

            engine->get_race_registry().register_item(id, race);
            logger->debug("Registered race '{}' ({})", id, name);
        },
        LuaFunctionDoc{
            .description = "Create a race",
            .params = {
                {"race_table", "table", "Table with fields: id, name, description, stat_modifiers"}
            }
        }
    );

    // Bind CreateClass function
    register_table_function(engine_table, "Engine", "CreateClass",
        [engine, logger](sol::table class_table) {
            std::string id = class_table["id"];
            std::string name = class_table["name"];
            std::string description = class_table.get_or<std::string>("description", "");
            std::string growth_pattern_id = class_table.get_or<std::string>("growth_pattern_id", "");

            CharacterClass char_class;
            char_class.id = id;
            char_class.name = name;
            char_class.description = description;
            char_class.growth_pattern_id = growth_pattern_id;

            // Parse starting stat bonuses if provided
            sol::optional<sol::table> stats_opt = class_table["starting_stats"];
            if (stats_opt) {
                sol::table stats = *stats_opt;
                if (auto val = stats.get<sol::optional<int>>("strength")) char_class.starting_stats[CoreStat::STRENGTH] = *val;
                if (auto val = stats.get<sol::optional<int>>("dexterity")) char_class.starting_stats[CoreStat::DEXTERITY] = *val;
                if (auto val = stats.get<sol::optional<int>>("constitution")) char_class.starting_stats[CoreStat::CONSTITUTION] = *val;
                if (auto val = stats.get<sol::optional<int>>("intelligence")) char_class.starting_stats[CoreStat::INTELLIGENCE] = *val;
                if (auto val = stats.get<sol::optional<int>>("cunning")) char_class.starting_stats[CoreStat::CUNNING] = *val;
                if (auto val = stats.get<sol::optional<int>>("focus")) char_class.starting_stats[CoreStat::FOCUS] = *val;
                if (auto val = stats.get<sol::optional<int>>("faith")) char_class.starting_stats[CoreStat::FAITH] = *val;
                if (auto val = stats.get<sol::optional<int>>("attunement")) char_class.starting_stats[CoreStat::ATTUNEMENT] = *val;
                if (auto val = stats.get<sol::optional<int>>("resilience")) char_class.starting_stats[CoreStat::RESILIENCE] = *val;
                if (auto val = stats.get<sol::optional<int>>("presence")) char_class.starting_stats[CoreStat::PRESENCE] = *val;
                if (auto val = stats.get<sol::optional<int>>("charisma")) char_class.starting_stats[CoreStat::CHARISMA] = *val;
                if (auto val = stats.get<sol::optional<int>>("composure")) char_class.starting_stats[CoreStat::COMPOSURE] = *val;
            }

            engine->get_class_registry().register_item(id, char_class);
            logger->debug("Registered class '{}' ({}) with growth pattern '{}'", id, name, growth_pattern_id);
        },
        LuaFunctionDoc{
            .description = "Create a character class",
            .params = {
                {"class_table", "table", "Table with fields: id, name, description, growth_pattern_id, starting_stats"}
            }
        }
    );

    // Bind CreateGrowthPattern function
    register_table_function(engine_table, "Engine", "CreateGrowthPattern",
        [engine, logger](sol::table pattern_table) {
            std::string id = pattern_table["id"];
            std::string name = pattern_table["name"];

            StatGrowthPattern pattern;
            pattern.name = name;

            // Parse growth rates (0.0 - 1.0 floats)
            sol::optional<sol::table> rates_opt = pattern_table["growth_rates"];
            if (rates_opt) {
                sol::table rates = *rates_opt;
                if (auto val = rates.get<sol::optional<float>>("strength")) pattern.growth_rates[CoreStat::STRENGTH] = *val;
                if (auto val = rates.get<sol::optional<float>>("dexterity")) pattern.growth_rates[CoreStat::DEXTERITY] = *val;
                if (auto val = rates.get<sol::optional<float>>("constitution")) pattern.growth_rates[CoreStat::CONSTITUTION] = *val;
                if (auto val = rates.get<sol::optional<float>>("intelligence")) pattern.growth_rates[CoreStat::INTELLIGENCE] = *val;
                if (auto val = rates.get<sol::optional<float>>("cunning")) pattern.growth_rates[CoreStat::CUNNING] = *val;
                if (auto val = rates.get<sol::optional<float>>("focus")) pattern.growth_rates[CoreStat::FOCUS] = *val;
                if (auto val = rates.get<sol::optional<float>>("faith")) pattern.growth_rates[CoreStat::FAITH] = *val;
                if (auto val = rates.get<sol::optional<float>>("attunement")) pattern.growth_rates[CoreStat::ATTUNEMENT] = *val;
                if (auto val = rates.get<sol::optional<float>>("resilience")) pattern.growth_rates[CoreStat::RESILIENCE] = *val;
                if (auto val = rates.get<sol::optional<float>>("presence")) pattern.growth_rates[CoreStat::PRESENCE] = *val;
                if (auto val = rates.get<sol::optional<float>>("charisma")) pattern.growth_rates[CoreStat::CHARISMA] = *val;
                if (auto val = rates.get<sol::optional<float>>("composure")) pattern.growth_rates[CoreStat::COMPOSURE] = *val;
            }

            engine->get_growth_pattern_registry().register_item(id, pattern);
            logger->debug("Registered growth pattern '{}' ({})", id, name);
        },
        LuaFunctionDoc{
            .description = "Create a stat growth pattern for character classes",
            .params = {
                {"pattern_table", "table", "Table with fields: id, name, growth_rates (0.0-1.0 chance per stat)"}
            }
        }
    );

    logger->info("Lua bindings initialized");
}

void LuaBindings::generate_type_definitions(const std::string& output_path) {
    std::ofstream file(output_path);

    file << "---@meta\n\n";
    file << "-- Roguelike Engine Lua API\n";
    file << "-- Auto-generated type definitions for editor support\n\n";

    // Group functions by table
    std::map<std::string, std::vector<const LuaFunctionDoc*>> tables;
    for (const auto& func : function_docs) {
        tables[func.table_name].push_back(&func);
    }

    // Generate definitions for each table
    for (const auto& [table_name, functions] : tables) {
        if (!table_name.empty()) {
            file << "---@class " << table_name << "\n";
            file << table_name << " = {}\n\n";
        }

        for (const auto* func : functions) {
            file << "---" << func->description << "\n";
            for (const auto& param : func->params) {
                file << "---@param " << param.name << " " << param.type << " " << param.description << "\n";
            }

            if (!table_name.empty()) {
                file << "function " << table_name << "." << func->name << "(";
            } else {
                file << "function " << func->name << "(";
            }

            for (size_t i = 0; i < func->params.size(); ++i) {
                if (i > 0) file << ", ";
                file << func->params[i].name;
            }

            file << ") end\n\n";
        }
    }

    file.close();
}

} // namespace engine
