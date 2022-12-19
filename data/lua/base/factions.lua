local ____lualib = require("lualib_bundle")
local __TS__SourceMapTraceBack = ____lualib.__TS__SourceMapTraceBack
__TS__SourceMapTraceBack(debug.getinfo(1).short_src, {["5"] = 1,["6"] = 8,["7"] = 13,["8"] = 20,["9"] = 29});
local ____exports = {}
____exports.DEFAULT_FACTIONS = {PLAYER = "player", MINDLESS = "mindless", TOWNSFOLK = "townsfolk", BANDITS = "bandits"}
rglike.defineFaction({id = ____exports.DEFAULT_FACTIONS.PLAYER, name = "Player", responses = {}})
rglike.defineFaction({id = ____exports.DEFAULT_FACTIONS.MINDLESS, name = "Mindless", responses = {[rglike.Faction.DEFAULT] = rglike.Response.ATTACK}})
rglike.defineFaction({id = ____exports.DEFAULT_FACTIONS.TOWNSFOLK, name = "Townsfolk", responses = {[rglike.Faction.DEFAULT] = rglike.Response.FLEE, [rglike.Faction.SELF] = rglike.Response.IGNORE, [____exports.DEFAULT_FACTIONS.PLAYER] = rglike.Response.IGNORE}})
rglike.defineFaction({id = ____exports.DEFAULT_FACTIONS.BANDITS, name = "Bandits", responses = {[rglike.Faction.DEFAULT] = rglike.Response.ATTACK, [rglike.Faction.SELF] = rglike.Response.IGNORE}})
return ____exports
