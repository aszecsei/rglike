local ____lualib = require("lualib_bundle")
local __TS__SourceMapTraceBack = ____lualib.__TS__SourceMapTraceBack
__TS__SourceMapTraceBack(debug.getinfo(1).short_src, {["5"] = 3,["6"] = 8});
local ____exports = {}
____exports.DEFAULT_SPELLS = {MAGIC_MISSILE = "mmissile", FIREBALL = "fireball"}
rglike.defineSpell({id = ____exports.DEFAULT_SPELLS.MAGIC_MISSILE, name = "Magic Missile", mana_cost = 10, effects = {ranged = 6, damage = "5", particle = "▓;#00FFFF;400.0"}})
return ____exports
