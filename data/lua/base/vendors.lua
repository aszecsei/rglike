local ____lualib = require("lualib_bundle")
local __TS__ObjectValues = ____lualib.__TS__ObjectValues
local __TS__SourceMapTraceBack = ____lualib.__TS__SourceMapTraceBack
__TS__SourceMapTraceBack(debug.getinfo(1).short_src, {["6"] = 1,["7"] = 2,["8"] = 2,["9"] = 2,["10"] = 1,["11"] = 6,["12"] = 6,["13"] = 6,["14"] = 1,["15"] = 10,["16"] = 10,["17"] = 10,["18"] = 1,["19"] = 14,["20"] = 14,["21"] = 14,["22"] = 1,["23"] = 18,["24"] = 18,["25"] = 18,["26"] = 1,["27"] = 22,["28"] = 22,["29"] = 22,["30"] = 1,["31"] = 1,["32"] = 28,["33"] = 29});
local ____exports = {}
____exports.DEFAULT_CATEGORIES = {
    ALCHEMY = {
        id = "alchemy",
        name = rglike.localize("alchemy_category", "Alchemy")
    },
    FOOD = {
        id = "food",
        name = rglike.localize("food_category", "Food")
    },
    JUNK = {
        id = "junk",
        name = rglike.localize("junk_category", "Junk")
    },
    WEAPONS = {
        id = "weapons",
        name = rglike.localize("weapons_category", "Weapons")
    },
    ARMOR = {
        id = "armor",
        name = rglike.localize("armor_category", "Armor")
    },
    CLOTHES = {
        id = "clothes",
        name = rglike.localize("clothes_category", "Clothes")
    }
}
for ____, v in ipairs(__TS__ObjectValues(____exports.DEFAULT_CATEGORIES)) do
    rglike.defineVendorCategory(v)
end
return ____exports
