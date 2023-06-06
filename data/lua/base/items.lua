local ____lualib = require("lualib_bundle")
local __TS__SourceMapTraceBack = ____lualib.__TS__SourceMapTraceBack
__TS__SourceMapTraceBack(debug.getinfo(1).short_src, {["5"] = 1,["6"] = 1,["7"] = 2,["8"] = 2,["9"] = 4,["10"] = 9,["11"] = 9,["12"] = 9,["13"] = 9,["14"] = 9,["15"] = 9,["16"] = 9,["17"] = 9,["18"] = 9});
local ____exports = {}
local ____vendors = require("base.vendors")
local DEFAULT_CATEGORIES = ____vendors.DEFAULT_CATEGORIES
local ____spells = require("base.spells")
local DEFAULT_SPELLS = ____spells.DEFAULT_SPELLS
____exports.DEFAULT_ITEMS = {BEGIN_MAGIC = "BeginnerMagic"}
rglike.defineItem({
    id = ____exports.DEFAULT_ITEMS.BEGIN_MAGIC,
    name = rglike.localize("beginner_magic_title", "Beginner's Magic"),
    renderable = {glyph = "¶", fg = "M", bg = "k", order = 2},
    consumable = {effects = {teach_spell = DEFAULT_SPELLS.FIREBALL}},
    weight = 0.5,
    base_value = 50,
    vendor_category = DEFAULT_CATEGORIES.ALCHEMY.id
})
return ____exports
