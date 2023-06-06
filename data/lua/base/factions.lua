local ____lualib = require("lualib_bundle")
local __TS__SourceMapTraceBack = ____lualib.__TS__SourceMapTraceBack
__TS__SourceMapTraceBack(debug.getinfo(1).short_src, {["5"] = 1,["6"] = 8,["7"] = 8,["8"] = 8,["9"] = 8,["10"] = 8,["11"] = 13,["12"] = 13,["13"] = 13,["14"] = 13,["15"] = 13,["16"] = 20,["17"] = 20,["18"] = 20,["19"] = 20,["20"] = 20,["21"] = 29,["22"] = 29,["23"] = 29,["24"] = 29,["25"] = 29});
local ____exports = {}
____exports.DEFAULT_FACTIONS = {PLAYER = "player", MINDLESS = "mindless", TOWNSFOLK = "townsfolk", BANDITS = "bandits"}
rglike.defineFaction({
    id = ____exports.DEFAULT_FACTIONS.PLAYER,
    name = rglike.localize("player_faction_name", "Player"),
    responses = {}
})
rglike.defineFaction({
    id = ____exports.DEFAULT_FACTIONS.MINDLESS,
    name = rglike.localize("mindless_faction_name", "Mindless"),
    responses = {[rglike.Faction.DEFAULT] = rglike.Response.ATTACK}
})
rglike.defineFaction({
    id = ____exports.DEFAULT_FACTIONS.TOWNSFOLK,
    name = rglike.localize("townsfolk_faction_name", "Townsfolk"),
    responses = {[rglike.Faction.DEFAULT] = rglike.Response.FLEE, [rglike.Faction.SELF] = rglike.Response.IGNORE, [____exports.DEFAULT_FACTIONS.PLAYER] = rglike.Response.IGNORE}
})
rglike.defineFaction({
    id = ____exports.DEFAULT_FACTIONS.BANDITS,
    name = rglike.localize("bandits_faction_name", "Bandits"),
    responses = {[rglike.Faction.DEFAULT] = rglike.Response.ATTACK, [rglike.Faction.SELF] = rglike.Response.IGNORE}
})
return ____exports
