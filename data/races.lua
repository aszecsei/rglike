-- Race definitions
-- Engine.CreateRace(table) - takes a table with the following fields:
--   id: string - unique identifier
--   name: string - display name
--   description: string - race description
--   stat_modifiers: table - stat bonuses/penalties applied to base stats

-- Human - Balanced, no particular strengths or weaknesses
Engine.CreateRace({
    id = "human",
    name = "Human",
    description = "Humans are versatile and adaptable, with no particular strengths or weaknesses. Their balanced nature allows them to excel in any class.",
    stat_modifiers = {
        -- No modifiers - pure baseline
    }
})

-- Elf - Agile and perceptive, magically attuned
Engine.CreateRace({
    id = "elf",
    name = "Elf",
    description = "Elves are graceful and long-lived, with keen senses and a natural affinity for magic. They excel in dexterity and spiritual pursuits but are physically frail.",
    stat_modifiers = {
        dexterity = 3,
        cunning = 2,
        attunement = 3,
        charisma = 2,
        strength = -2,
        constitution = -2
    }
})

-- Dwarf - Hardy and resilient, strong warriors
Engine.CreateRace({
    id = "dwarf",
    name = "Dwarf",
    description = "Dwarves are stout and sturdy, renowned for their craftsmanship and resilience. They make excellent warriors and can endure great hardship, though they lack agility.",
    stat_modifiers = {
        strength = 2,
        constitution = 4,
        resilience = 3,
        composure = 2,
        dexterity = -2,
        charisma = -1
    }
})

-- Gnome - Clever tinkers and illusionists
Engine.CreateRace({
    id = "gnome",
    name = "Gnome",
    description = "Gnomes are small but brilliant, with a knack for magic and trickery. Their intelligence and cunning make them excellent spellcasters, though they are physically weak.",
    stat_modifiers = {
        intelligence = 3,
        cunning = 4,
        attunement = 2,
        charisma = 2,
        strength = -3,
        constitution = -2
    }
})

-- Demon - Powerful and charismatic, spiritually strong
Engine.CreateRace({
    id = "demon",
    name = "Demon",
    description = "Demons are supernatural beings of immense power and presence. Their physical and spiritual might is unmatched, but they struggle with focus and wisdom.",
    stat_modifiers = {
        strength = 3,
        faith = 4,
        resilience = 3,
        presence = 4,
        charisma = 2,
        cunning = -2,
        focus = -3,
        composure = -2
    }
})