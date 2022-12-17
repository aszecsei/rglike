-- Types of responses.
local Response = {
    ATTACK = "attack",
    IGNORE = "ignore",
    FLEE = "flee",
}

-- Special faction types for responses.
local Faction = {
    -- The default faction. Any unspecified faction will use the
    -- response associated with this faction.
    DEFAULT = "Default",
    -- This is a generic way to refer to the faction currently being defined.
    SELF = "Self",
}

return {
    {
        name = "Player",
        responses = {},
    },
    {
        name = "Mindless",
        responses = {
            [Faction.DEFAULT] = Response.ATTACK,
        }
    },
    {
        name = "Townsfolk",
        responses = {
            [Faction.DEFAULT] = Response.FLEE,
            [Faction.SELF] = Response.IGNORE,
            ["Player"] = Response.IGNORE,
        }
    },
    {
        name = "Bandits",
        responses = {
            [Faction.DEFAULT] = Response.ATTACK,
            [Faction.SELF] = Response.IGNORE,
        }
    },
}