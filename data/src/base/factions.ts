export const DEFAULT_FACTIONS = {
    PLAYER: 'player',
    MINDLESS: 'mindless',
    TOWNSFOLK: 'townsfolk',
    BANDITS: 'bandits',
}

rglike.defineFaction({
    id: DEFAULT_FACTIONS.PLAYER,
    name: 'Player',
    responses: {},
})
rglike.defineFaction({
    id: DEFAULT_FACTIONS.MINDLESS,
    name: 'Mindless',
    responses: {
        [rglike.Faction.DEFAULT]: rglike.Response.ATTACK,
    },
})
rglike.defineFaction({
    id: DEFAULT_FACTIONS.TOWNSFOLK,
    name: 'Townsfolk',
    responses: {
        [rglike.Faction.DEFAULT]: rglike.Response.FLEE,
        [rglike.Faction.SELF]: rglike.Response.IGNORE,
        [DEFAULT_FACTIONS.PLAYER]: rglike.Response.IGNORE,
    },
})
rglike.defineFaction({
    id: DEFAULT_FACTIONS.BANDITS,
    name: 'Bandits',
    responses: {
        [rglike.Faction.DEFAULT]: rglike.Response.ATTACK,
        [rglike.Faction.SELF]: rglike.Response.IGNORE,
    },
})
