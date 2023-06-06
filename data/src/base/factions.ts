export const DEFAULT_FACTIONS = {
    PLAYER: 'player',
    MINDLESS: 'mindless',
    TOWNSFOLK: 'townsfolk',
    BANDITS: 'bandits',
}

rglike.defineFaction({
    id: DEFAULT_FACTIONS.PLAYER,
    name: rglike.localize('player_faction_name', 'Player'),
    responses: {},
})
rglike.defineFaction({
    id: DEFAULT_FACTIONS.MINDLESS,
    name: rglike.localize('mindless_faction_name', 'Mindless'),
    responses: {
        [rglike.Faction.DEFAULT]: rglike.Response.ATTACK,
    },
})
rglike.defineFaction({
    id: DEFAULT_FACTIONS.TOWNSFOLK,
    name: rglike.localize('townsfolk_faction_name', 'Townsfolk'),
    responses: {
        [rglike.Faction.DEFAULT]: rglike.Response.FLEE,
        [rglike.Faction.SELF]: rglike.Response.IGNORE,
        [DEFAULT_FACTIONS.PLAYER]: rglike.Response.IGNORE,
    },
})
rglike.defineFaction({
    id: DEFAULT_FACTIONS.BANDITS,
    name: rglike.localize('bandits_faction_name', 'Bandits'),
    responses: {
        [rglike.Faction.DEFAULT]: rglike.Response.ATTACK,
        [rglike.Faction.SELF]: rglike.Response.IGNORE,
    },
})
