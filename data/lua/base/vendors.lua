local ____lualib = require("lualib_bundle")
local __TS__ObjectValues = ____lualib.__TS__ObjectValues
local __TS__SourceMapTraceBack = ____lualib.__TS__SourceMapTraceBack
__TS__SourceMapTraceBack(debug.getinfo(1).short_src, {["6"] = 1,["7"] = 1,["8"] = 1,["9"] = 1,["10"] = 1,["11"] = 1,["12"] = 1,["13"] = 1,["14"] = 10,["15"] = 11});
local ____exports = {}
____exports.DEFAULT_CATEGORIES = {
    ALCHEMY = "alchemy",
    FOOD = "food",
    JUNK = "junk",
    WEAPONS = "weapons",
    ARMOR = "armor",
    CLOTHES = "clothes"
}
for ____, v in ipairs(__TS__ObjectValues(____exports.DEFAULT_CATEGORIES)) do
    rglike.defineVendorCategory(v)
end
return ____exports
