#include <engine/lua_bindings.h>
#include <engine/engine.h>
#include <engine/terrain.h>
#include <engine/mob.h>
#include <engine/prop.h>
#include <engine/item.h>
#include <engine/components.h>
#include <engine/equipment.h>
#include <engine/faction.h>
#include <engine/character_data.h>
#include <engine/constants.h>
#include <engine/growth_pattern.h>
#include <fstream>
#include <stdexcept>
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
                                                               const std::shared_ptr<spdlog::logger>& logger,
                                                               const std::string& context = "") {
    if (response_str == "IGNORE") {
        return FactionResponse::IGNORE;
    }
    if (response_str == "ATTACK") {
        return FactionResponse::ATTACK;
    }
    if (response_str == "FLEE") {
        return FactionResponse::FLEE;
    }
    logger->error("{}: Invalid response '{}' (must be IGNORE, ATTACK, or FLEE)",
                 context.empty() ? "parse_faction_response" : context, response_str);
    return std::nullopt;
}

// Helper function to parse RGB color from Lua table
static ftxui::Color parse_rgb_color(sol::table color_table) {
    return ftxui::Color::RGB(color_table[1], color_table[2], color_table[3]);
}

// Helper: read any subset of the 12 core stats from a sol::table into a map.
// Keys are the canonical lowercase stat names already used in races/classes.
// Unrecognized keys are ignored here — callers that want to flag them (e.g.
// CreateClass for unknown starting_loadout keys) inspect the table directly.
static void parse_core_stat_map(const sol::table& table, std::unordered_map<CoreStat, int>& out) {
    if (auto v = table.get<sol::optional<int>>("strength"))     out[CoreStat::STRENGTH]     = *v;
    if (auto v = table.get<sol::optional<int>>("dexterity"))    out[CoreStat::DEXTERITY]    = *v;
    if (auto v = table.get<sol::optional<int>>("constitution")) out[CoreStat::CONSTITUTION] = *v;
    if (auto v = table.get<sol::optional<int>>("intelligence")) out[CoreStat::INTELLIGENCE] = *v;
    if (auto v = table.get<sol::optional<int>>("cunning"))      out[CoreStat::CUNNING]      = *v;
    if (auto v = table.get<sol::optional<int>>("focus"))        out[CoreStat::FOCUS]        = *v;
    if (auto v = table.get<sol::optional<int>>("faith"))        out[CoreStat::FAITH]        = *v;
    if (auto v = table.get<sol::optional<int>>("attunement"))   out[CoreStat::ATTUNEMENT]   = *v;
    if (auto v = table.get<sol::optional<int>>("resilience"))   out[CoreStat::RESILIENCE]   = *v;
    if (auto v = table.get<sol::optional<int>>("presence"))     out[CoreStat::PRESENCE]     = *v;
    if (auto v = table.get<sol::optional<int>>("charisma"))     out[CoreStat::CHARISMA]     = *v;
    if (auto v = table.get<sol::optional<int>>("composure"))    out[CoreStat::COMPOSURE]    = *v;
}

// Helper to register a function in a table with its documentation
template<typename Func>
void register_table_function(sol::table& table, const std::string& table_name,
                             const std::string& name, Func&& func,
                             std::string description,
                             std::vector<LuaFunctionParam> params) {
    table.set_function(name, std::forward<Func>(func));
    function_docs.push_back(LuaFunctionDoc{
        .table_name = table_name,
        .name = name,
        .description = std::move(description),
        .params = std::move(params),
    });
}

void LuaBindings::initialize(sol::state& lua, Engine* engine, const std::shared_ptr<spdlog::logger>& logger) {
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
        "Create a terrain type",
        {
            {.name = "terrain_table", .type = "table", .description = "Table with fields: id, glyph, fg_color (RGB array), mg_color (optional RGB array), bg_color (optional RGB array), passable, blocks_vision"}
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

            // Optional render_order (defaults to RENDER_ORDER_MOBS so mobs
            // draw on top of doors/items but below the player).
            int render_order = mob_table.get_or("render_order", constants::RENDER_ORDER_MOBS);

            // Optional blocks_movement (defaults to true)
            bool blocks_movement = mob_table.get_or("blocks_movement", true);

            // Optional blocks_vision (defaults to false)
            bool blocks_vision = mob_table.get_or("blocks_vision", false);

            int vision_range = mob_table["vision_range"];

            // Optional faction_id (defaults to "neutral")
            std::string faction_id = mob_table.get_or<std::string>("faction_id", "neutral");

            // Optional drops table. Each entry: { id, chance?, min_count?, max_count? }.
            // Item ids are not validated here because Lua load order is filesystem
            // iteration order; items.lua may not have been processed yet. The
            // death handler will silently skip unknown ids and log a warning at
            // spawn time.
            std::vector<DropEntry> drops;
            sol::optional<sol::table> drops_opt = mob_table["drops"];
            if (drops_opt) {
                sol::table drops_table = *drops_opt;
                for (const auto& kv : drops_table) {
                    sol::object value = kv.second;
                    if (!value.is<sol::table>()) {
                        logger->warn("Mob '{}': drops entry is not a table, skipping", id);
                        continue;
                    }
                    sol::table entry = value.as<sol::table>();
                    DropEntry de;
                    sol::optional<std::string> entry_id = entry["id"];
                    if (!entry_id) {
                        logger->warn("Mob '{}': drops entry missing 'id', skipping", id);
                        continue;
                    }
                    de.item_id = *entry_id;
                    de.chance = entry.get_or("chance", 1.0F);
                    de.min_count = entry.get_or("min_count", 1);
                    de.max_count = entry.get_or("max_count", de.min_count);
                    de.max_count = std::max(de.max_count, de.min_count);
                    drops.push_back(std::move(de));
                }
            }

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
            mob.drops = std::move(drops);

            // Check for stats table (new system) or legacy hp/defense/power
            sol::optional<sol::table> stats_opt = mob_table["stats"];
            if (stats_opt) {
                // New stats system
                const sol::table& stats_table = *stats_opt;
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
        "Create a mob type",
        {
            {.name = "mob_table", .type = "table", .description = "Table with fields: id, name, glyph, fg_color (RGB array), bg_color (optional RGB array), bold (optional), render_order (optional), blocks_movement (optional), blocks_vision (optional), vision_range, max_hp, defense, power, faction_id (optional)"}
        }
    );

    // Bind CreateProp function. Props are decorative or interactive scenery
    // (doors, candles, tables, chairs, ...). If open_glyph is provided the
    // prop is treated as a door: spawning attaches a Door component and
    // opening swaps the rendered glyph.
    register_table_function(engine_table, "Engine", "CreateProp",
        [engine, logger](sol::table prop_table) {
            std::string id = prop_table["id"];
            std::string name = prop_table.get_or<std::string>("name", id);
            std::string glyph = prop_table["glyph"];

            ftxui::Color fg_color = parse_rgb_color(prop_table["fg_color"]);
            ftxui::Color bg_color = ftxui::Color::Default;
            sol::optional<sol::table> bg_color_opt = prop_table["bg_color"];
            if (bg_color_opt) {
                bg_color = parse_rgb_color(*bg_color_opt);
            }

            Prop prop;
            prop.id = id;
            prop.name = name;
            prop.glyph = glyph;
            prop.fg_color = fg_color;
            prop.bg_color = bg_color;
            prop.bold = prop_table.get_or("bold", false);
            prop.render_order = prop_table.get_or("render_order", constants::RENDER_ORDER_DOORS);
            prop.blocks_movement = prop_table.get_or("blocks_movement", false);
            prop.blocks_vision = prop_table.get_or("blocks_vision", false);

            sol::optional<std::string> open_glyph_opt = prop_table["open_glyph"];
            if (open_glyph_opt) {
                prop.open_glyph = *open_glyph_opt;
            }

            engine->get_prop_registry().register_item(id, prop);
            logger->debug("Registered prop '{}' ({}) glyph='{}' openable={}",
                         id, name, glyph, prop.open_glyph.has_value());
        },
        "Create a prop (scenery or interactive object)",
        {
            {.name = "prop_table", .type = "table",
             .description = "Fields: id, name (optional), glyph, fg_color (RGB array), "
                            "bg_color (optional RGB array), bold (optional), render_order (optional), "
                            "blocks_movement (optional), blocks_vision (optional), "
                            "open_glyph (optional string — presence flags it as a door)"}
        }
    );

    // Bind CreateItem. Items follow the Mob/Prop registry pattern: Lua loads
    // them as templates at startup, and spawning produces an entity with
    // Position/Renderable/NameComponent/ItemComponent. Stackable items merge
    // in inventory by template id.
    register_table_function(engine_table, "Engine", "CreateItem",
        [engine, logger](sol::table item_table) {
            std::string id = item_table["id"];
            std::string name = item_table.get_or<std::string>("name", id);
            std::string glyph = item_table["glyph"];

            ftxui::Color fg_color = parse_rgb_color(item_table["fg_color"]);
            ftxui::Color bg_color = ftxui::Color::Default;
            sol::optional<sol::table> bg_color_opt = item_table["bg_color"];
            if (bg_color_opt) {
                bg_color = parse_rgb_color(*bg_color_opt);
            }

            Item item;
            item.id = id;
            item.name = name;
            item.glyph = glyph;
            item.fg_color = fg_color;
            item.bg_color = bg_color;
            item.bold = item_table.get_or("bold", false);
            item.render_order = item_table.get_or("render_order", constants::RENDER_ORDER_ITEMS);
            item.is_stackable = item_table.get_or("is_stackable", false);

            // Optional equipment metadata. equip_slot is the gateway: when
            // absent, the item is a plain bag-only consumable; when present,
            // damage_bonus / defense_bonus / stat_bonuses become meaningful.
            sol::optional<std::string> equip_slot_opt = item_table["equip_slot"];
            if (equip_slot_opt) {
                auto kind = parse_item_slot_kind(*equip_slot_opt);
                if (!kind) {
                    logger->error("CreateItem '{}': unknown equip_slot '{}' "
                                  "(expected main_hand|off_hand|head|chest|legs|"
                                  "boots|gloves|amulet|ring)", id, *equip_slot_opt);
                    throw std::runtime_error("CreateItem: unknown equip_slot");
                }
                item.equip_slot = *kind;
            }

            item.two_handed   = item_table.get_or("two_handed", false);
            item.damage_bonus = item_table.get_or("damage_bonus", 0);
            item.defense_bonus = item_table.get_or("defense_bonus", 0);

            sol::optional<sol::table> stat_bonuses_opt = item_table["stat_bonuses"];
            if (stat_bonuses_opt) {
                parse_core_stat_map(*stat_bonuses_opt, item.stat_bonuses);
            }

            // Equippable + stackable is incoherent: equipped items live as
            // distinct entities so per-instance state (slot binding, future
            // durability) is tractable. Reject at registration time.
            if (item.equip_slot && item.is_stackable) {
                logger->error("CreateItem '{}': equippable items cannot be "
                              "stackable", id);
                throw std::runtime_error("CreateItem: equippable item is stackable");
            }
            // two_handed only makes sense for the main-hand slot; flag misuse.
            if (item.two_handed &&
                (!item.equip_slot || *item.equip_slot != ItemSlotKind::MAIN_HAND)) {
                logger->error("CreateItem '{}': two_handed=true is only valid "
                              "when equip_slot='main_hand'", id);
                throw std::runtime_error("CreateItem: two_handed misuse");
            }

            engine->get_item_registry().register_item(id, item);
            logger->debug("Registered item '{}' ({}) glyph='{}' stackable={} "
                         "equip_slot={} two_handed={} dmg={} def={}",
                         id, name, glyph, item.is_stackable,
                         item.equip_slot ? std::string(to_string(*item.equip_slot)) : "(none)",
                         item.two_handed, item.damage_bonus, item.defense_bonus);
        },
        "Create an item type",
        {
            {.name = "item_table", .type = "table",
             .description = "Fields: id, name (optional), glyph, fg_color (RGB array), "
                            "bg_color (optional RGB array), bold (optional), "
                            "render_order (optional), is_stackable (optional), "
                            "equip_slot (optional: main_hand|off_hand|head|chest|legs|"
                            "boots|gloves|amulet|ring), two_handed (optional), "
                            "damage_bonus (optional int), defense_bonus (optional int), "
                            "stat_bonuses (optional table of core stat → int)"}
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
        "Create a faction and return the faction object",
        {
            {.name = "faction_table", .type = "table", .description = "Table with fields: id, name, responses (optional table mapping target faction IDs to response strings)"}
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
                const sol::table& mods = *mods_opt;
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
        "Create a race",
        {
            {.name = "race_table", .type = "table", .description = "Table with fields: id, name, description, stat_modifiers"}
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
                parse_core_stat_map(*stats_opt, char_class.starting_stats);
            }

            // Parse starting loadout (worn equipment + bag contents). The
            // loadout table has slot-name keys (main_hand, off_hand, head,
            // chest, legs, boots, gloves, amulet, ring_1, ring_2) whose
            // values are item template ids, plus an optional `inventory`
            // key whose value is an array of item template ids that spawn
            // in the bag rather than equipped. Unknown keys are flagged
            // here so typos in data files surface at load time rather
            // than as silent missing gear at spawn time.
            sol::optional<sol::table> loadout_opt = class_table["starting_loadout"];
            if (loadout_opt) {
                sol::table loadout = *loadout_opt;
                for (const auto& pair : loadout) {
                    if (!pair.first.is<std::string>()) {
                        logger->error("CreateClass '{}': starting_loadout has "
                                      "non-string key", id);
                        throw std::runtime_error("CreateClass: bad loadout key");
                    }
                    std::string key = pair.first.as<std::string>();
                    if (key == "inventory") {
                        if (!pair.second.is<sol::table>()) {
                            logger->error("CreateClass '{}': starting_loadout."
                                          "inventory must be an array of item ids",
                                          id);
                            throw std::runtime_error("CreateClass: bad inventory list");
                        }
                        sol::table inv = pair.second.as<sol::table>();
                        for (std::size_t i = 1; i <= inv.size(); ++i) {
                            sol::optional<std::string> item_id = inv[i];
                            if (!item_id) {
                                logger->error("CreateClass '{}': "
                                              "starting_loadout.inventory[{}] "
                                              "is not a string", id, i);
                                throw std::runtime_error("CreateClass: inventory entry not string");
                            }
                            char_class.starting_inventory.push_back(*item_id);
                        }
                        continue;
                    }
                    auto slot = parse_equipment_slot(key);
                    if (!slot) {
                        logger->error("CreateClass '{}': starting_loadout has "
                                      "unknown slot key '{}' (expected main_hand|"
                                      "off_hand|head|chest|legs|boots|gloves|"
                                      "amulet|ring_1|ring_2|inventory)", id, key);
                        throw std::runtime_error("CreateClass: unknown loadout slot");
                    }
                    if (!pair.second.is<std::string>()) {
                        logger->error("CreateClass '{}': starting_loadout.{} "
                                      "must be a string item id", id, key);
                        throw std::runtime_error("CreateClass: loadout slot value not string");
                    }
                    char_class.starting_equipment[*slot] = pair.second.as<std::string>();
                }
            }

            engine->get_class_registry().register_item(id, char_class);
            logger->debug("Registered class '{}' ({}) with growth pattern '{}', "
                         "{} equipped items, {} bag items",
                         id, name, growth_pattern_id,
                         char_class.starting_equipment.size(),
                         char_class.starting_inventory.size());
        },
        "Create a character class",
        {
            {.name = "class_table", .type = "table",
             .description = "Fields: id, name, description, growth_pattern_id, "
                            "starting_stats (optional), starting_loadout (optional table "
                            "with slot keys + optional inventory array)"}
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
                const sol::table& rates = *rates_opt;
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
        "Create a stat growth pattern for character classes",
        {
            {.name = "pattern_table", .type = "table", .description = "Table with fields: id, name, growth_rates (0.0-1.0 chance per stat)"}
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
