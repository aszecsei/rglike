-- Mob definitions
-- Engine.CreateMob(table) - takes a table with the following fields:
--   id: string - unique identifier
--   name: string - display name
--   glyph: string - Unicode character to display
--   fg_color: {r, g, b} - foreground color (required)
--   bg_color: {r, g, b} - background color (optional, defaults to black)
--   bold: boolean - render text in bold (optional, defaults to false)
--   render_order: integer - drawing priority (optional, defaults to 50)
--   blocks_movement: boolean - does this mob block the tile? (optional, defaults to true)
--   blocks_vision: boolean - does this mob block line of sight? (optional, defaults to false)
--   vision_range: integer - how far the mob can see
--   faction_id: string - faction ID (optional, defaults to "neutral")
--
-- Stats system (new, optional):
--   stats: table - contains core stats and level range
--     strength, dexterity, constitution (Physical)
--     intelligence, cunning, focus (Mental)
--     faith, attunement, resilience (Spiritual)
--     presence, charisma, composure (Social)
--     min_level, max_level - level range for spawned mobs (defaults to 1)
--
-- Legacy system (if stats not provided):
--   max_hp: integer - maximum hit points
--   defense: integer - armor class / defense value
--   power: integer - attack power / damage

-- Weak enemies (using new stats system)
Engine.CreateMob({
    id = "goblin",
    name = "Goblin",
    glyph = "g",
    fg_color = {0, 200, 0},
    vision_range = 6,
    faction_id = "monsters",
    stats = {
        strength = 8,
        dexterity = 12,
        constitution = 10,
        intelligence = 6,
        cunning = 10,
        focus = 7,
        faith = 3,
        attunement = 3,
        resilience = 5,
        presence = 5,
        charisma = 4,
        composure = 6,
        min_level = 1,
        max_level = 3
    }
})

Engine.CreateMob({
    id = "rat",
    name = "Giant Rat",
    glyph = "r",
    fg_color = {139, 69, 19},
    vision_range = 4,
    max_hp = 3,
    defense = 0,
    power = 1,
    faction_id = "wildlife"
})

-- Hostile pest that infests town buildings. Cheap to fight one-on-one but
-- dangerous in numbers — TownBuilder seeds a few per house.
Engine.CreateMob({
    id = "house_rat",
    name = "Rat",
    glyph = "r",
    fg_color = {120, 60, 30},
    vision_range = 5,
    max_hp = 4,
    defense = 0,
    power = 3,
    faction_id = "monsters"
})

-- Medium enemies
Engine.CreateMob({
    id = "orc",
    name = "Orc Warrior",
    glyph = "o",
    fg_color = {255, 100, 0},
    bold = true,
    vision_range = 7,
    max_hp = 10,
    defense = 1,
    power = 4,
    faction_id = "monsters"
})

Engine.CreateMob({
    id = "skeleton",
    name = "Skeleton",
    glyph = "s",
    fg_color = {255, 255, 255},
    vision_range = 8,
    max_hp = 8,
    defense = 2,
    power = 3,
    faction_id = "undead"
})

-- Strong enemies
Engine.CreateMob({
    id = "troll",
    name = "Troll",
    glyph = "T",
    fg_color = {0, 150, 0},
    bold = true,
    vision_range = 6,
    max_hp = 20,
    defense = 3,
    power = 6,
    faction_id = "monsters"
})

Engine.CreateMob({
    id = "dragon",
    name = "Dragon",
    glyph = "D",
    fg_color = {255, 0, 0},
    bold = true,
    vision_range = 10,
    max_hp = 50,
    defense = 5,
    power = 10,
    faction_id = "monsters"
})

-- NPCs
Engine.CreateMob({
    id = "villager",
    name = "Villager",
    glyph = "h",
    fg_color = {200, 200, 200},
    vision_range = 8,
    max_hp = 5,
    defense = 0,
    power = 0,
    faction_id = "villagers"
})

Engine.CreateMob({
    id = "merchant",
    name = "Merchant",
    glyph = "h",
    fg_color = {255, 215, 0},
    bold = true,
    vision_range = 8,
    max_hp = 8,
    defense = 0,
    power = 0,
    faction_id = "villagers"
})