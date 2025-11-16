-- Faction definitions
-- Engine.CreateFaction(table) - returns a Faction object with:
--   id: string - unique identifier
--   name: string - display name
--   responses: table (optional) - table mapping target faction IDs to response strings
--     Keys can be specific faction IDs, "DEFAULT" (fallback), or "SELF" (same faction)
--     Values: "IGNORE", "ATTACK", or "FLEE"
--
-- Faction:SetResponse(target_faction_id, response) - set individual response on the faction object
--   target_faction_id: string - the faction to respond to (or 'DEFAULT'/'SELF')
--   response: string - the response type: "IGNORE", "ATTACK", or "FLEE"

-- Player faction (using inline responses table)
local player = Engine.CreateFaction({
    id = "player",
    name = "Player",
    responses = {
        SELF = "IGNORE",
        DEFAULT = "IGNORE"
    }
})

-- Monsters faction - hostile to player, friendly to each other (using inline responses)
local monsters = Engine.CreateFaction({
    id = "monsters",
    name = "Monsters",
    responses = {
        SELF = "IGNORE",
        player = "ATTACK",
        DEFAULT = "IGNORE"
    }
})

-- Undead faction - hostile to everyone except other undead (using SetResponse method)
local undead = Engine.CreateFaction({
    id = "undead",
    name = "Undead"
})
undead:SetResponse("SELF", "IGNORE")
undead:SetResponse("player", "ATTACK")
undead:SetResponse("monsters", "ATTACK")
undead:SetResponse("villagers", "ATTACK")
undead:SetResponse("DEFAULT", "ATTACK")

-- Villagers faction - friendly to player, flee from hostiles
Engine.CreateFaction({
    id = "villagers",
    name = "Villagers",
    responses = {
        SELF = "IGNORE",
        player = "IGNORE",
        monsters = "FLEE",
        undead = "FLEE",
        DEFAULT = "IGNORE"
    }
})

-- Wildlife faction - mostly neutral, some will flee
Engine.CreateFaction({
    id = "wildlife",
    name = "Wildlife",
    responses = {
        SELF = "IGNORE",
        player = "FLEE",
        DEFAULT = "FLEE"
    }
})

-- Neutral faction - ignores everyone including itself
Engine.CreateFaction({
    id = "neutral",
    name = "Neutral",
    responses = {
        SELF = "IGNORE",
        DEFAULT = "IGNORE"
    }
})