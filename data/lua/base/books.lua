local ____lualib = require("lualib_bundle")
local __TS__SourceMapTraceBack = ____lualib.__TS__SourceMapTraceBack
__TS__SourceMapTraceBack(debug.getinfo(1).short_src, {["4"] = 1,["5"] = 1,["6"] = 1,["7"] = 4,["8"] = 4,["9"] = 4,["10"] = 1,["11"] = 1});
rglike.defineBook({
    id = "BasicBook",
    name = rglike.localize("basic_book_title", "Basic Book"),
    pages = {
        {rglike.localize("basic_book_page_1", "HELLO!")},
        {rglike.localize("basic_book_page_2", "WORLD!")}
    }
})
