-- Growth pattern definitions
-- Engine.CreateGrowthPattern(table) - takes a table with:
--   id: string - unique identifier
--   name: string - display name
--   growth_rates: table - stat growth rates (0.0-1.0 chance to increase on level up)

-- Warrior growth - Focuses on physical stats
Engine.CreateGrowthPattern({
    id = "warrior_growth",
    name = "Warrior Growth",
    growth_rates = {
        strength = 0.80,
        dexterity = 0.50,
        constitution = 0.75,
        intelligence = 0.20,
        cunning = 0.30,
        focus = 0.35,
        faith = 0.25,
        attunement = 0.15,
        resilience = 0.50,
        presence = 0.40,
        charisma = 0.25,
        composure = 0.45
    }
})

-- Thief growth - Focuses on dexterity and cunning
Engine.CreateGrowthPattern({
    id = "thief_growth",
    name = "Thief Growth",
    growth_rates = {
        strength = 0.40,
        dexterity = 0.85,
        constitution = 0.45,
        intelligence = 0.35,
        cunning = 0.90,
        focus = 0.50,
        faith = 0.15,
        attunement = 0.30,
        resilience = 0.40,
        presence = 0.30,
        charisma = 0.55,
        composure = 0.60
    }
})

-- Sorcerer growth - Focuses on intelligence and attunement
Engine.CreateGrowthPattern({
    id = "sorcerer_growth",
    name = "Sorcerer Growth",
    growth_rates = {
        strength = 0.20,
        dexterity = 0.30,
        constitution = 0.35,
        intelligence = 0.85,
        cunning = 0.60,
        focus = 0.70,
        faith = 0.40,
        attunement = 0.90,
        resilience = 0.50,
        presence = 0.35,
        charisma = 0.40,
        composure = 0.45
    }
})

-- Cleric growth - Focuses on faith and resilience
Engine.CreateGrowthPattern({
    id = "cleric_growth",
    name = "Cleric Growth",
    growth_rates = {
        strength = 0.45,
        dexterity = 0.35,
        constitution = 0.60,
        intelligence = 0.50,
        cunning = 0.40,
        focus = 0.65,
        faith = 0.90,
        attunement = 0.70,
        resilience = 0.85,
        presence = 0.50,
        charisma = 0.45,
        composure = 0.70
    }
})

-- Paladin growth - Balanced physical and spiritual
Engine.CreateGrowthPattern({
    id = "paladin_growth",
    name = "Paladin Growth",
    growth_rates = {
        strength = 0.70,
        dexterity = 0.40,
        constitution = 0.65,
        intelligence = 0.35,
        cunning = 0.30,
        focus = 0.50,
        faith = 0.75,
        attunement = 0.50,
        resilience = 0.70,
        presence = 0.60,
        charisma = 0.55,
        composure = 0.65
    }
})

-- Tourist growth - Low growth in everything (weak class)
Engine.CreateGrowthPattern({
    id = "tourist_growth",
    name = "Tourist Growth",
    growth_rates = {
        strength = 0.30,
        dexterity = 0.35,
        constitution = 0.40,
        intelligence = 0.35,
        cunning = 0.40,
        focus = 0.30,
        faith = 0.25,
        attunement = 0.25,
        resilience = 0.35,
        presence = 0.30,
        charisma = 0.50,
        composure = 0.35
    }
})

-- Class definitions
-- Engine.CreateClass(table) - takes a table with:
--   id: string - unique identifier
--   name: string - display name
--   description: string - class description
--   growth_pattern_id: string - ID of growth pattern to use
--   starting_stats: table (optional) - starting stat bonuses

-- Warrior - Strong melee combatant
Engine.CreateClass({
    id = "warrior",
    name = "Warrior",
    description = "Warriors are masters of physical combat, wielding weapons with deadly efficiency. They excel in melee combat and can take punishment while dishing it out.",
    growth_pattern_id = "warrior_growth",
    starting_stats = {
        strength = 5,
        constitution = 3,
        dexterity = 2
    }
})

-- Thief - Agile and sneaky
Engine.CreateClass({
    id = "thief",
    name = "Thief",
    description = "Thieves rely on speed, stealth, and cunning to overcome their foes. They excel at quick strikes and evasion, though they cannot take many hits.",
    growth_pattern_id = "thief_growth",
    starting_stats = {
        dexterity = 5,
        cunning = 4,
        composure = 2
    }
})

-- Sorcerer - Powerful spellcaster
Engine.CreateClass({
    id = "sorcerer",
    name = "Sorcerer",
    description = "Sorcerers wield arcane magic with devastating effect. Their intelligence and attunement allow them to cast powerful spells, though they are physically frail.",
    growth_pattern_id = "sorcerer_growth",
    starting_stats = {
        intelligence = 5,
        attunement = 4,
        focus = 3
    }
})

-- Cleric - Divine spellcaster and healer
Engine.CreateClass({
    id = "cleric",
    name = "Cleric",
    description = "Clerics channel divine power to heal allies and smite their enemies. Their faith and resilience make them formidable defenders of righteousness.",
    growth_pattern_id = "cleric_growth",
    starting_stats = {
        faith = 5,
        resilience = 3,
        constitution = 2,
        composure = 2
    }
})

-- Paladin - Holy warrior
Engine.CreateClass({
    id = "paladin",
    name = "Paladin",
    description = "Paladins combine martial prowess with divine magic. They are stalwart defenders who can both fight in melee and channel holy power.",
    growth_pattern_id = "paladin_growth",
    starting_stats = {
        strength = 3,
        faith = 3,
        constitution = 3,
        resilience = 2
    }
})

-- Tourist - Weak generalist with high luck
Engine.CreateClass({
    id = "tourist",
    name = "Tourist",
    description = "Tourists are inexperienced adventurers with no particular skills. They grow slowly in all areas but have a knack for finding interesting items and making friends.",
    growth_pattern_id = "tourist_growth",
    starting_stats = {
        charisma = 3,
        presence = 2
    }
})