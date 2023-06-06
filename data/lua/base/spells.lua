local ____lualib = require("lualib_bundle")
local __TS__SourceMapTraceBack = ____lualib.__TS__SourceMapTraceBack
__TS__SourceMapTraceBack(debug.getinfo(1).short_src, {["5"] = 1,["6"] = 6,["7"] = 6,["8"] = 6,["9"] = 6,["10"] = 6,["11"] = 6});
local ____exports = {}
____exports.DEFAULT_SPELLS = {MAGIC_MISSILE = "mmissile", FIREBALL = "fireball"}
rglike.defineSpell({
    id = ____exports.DEFAULT_SPELLS.MAGIC_MISSILE,
    name = rglike.localize("magic_missile_spell", "Magic Missile"),
    mana_cost = 10,
    effects = {ranged = 6, damage = "5", particle = "▓;#00FFFF;400.0"}
})
return ____exports
