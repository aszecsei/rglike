local VendorCategory = {
    ALCHEMY = "alchemy",
}

return {
    {
        name = "Health Potion",
        renderable = {
            glyph = "!",
            fg = "#FF00FF",
            bg = "#000000",
            order = 2,
        },
        consumable = {
            effects = {
                provides_healing = 8,
            }
        },
        weight = 0.5,
        base_value = 50,
        vendor = VendorCategory.ALCHEMY,
        magic = {
            class = "common",
            naming = "potion",
        }
    }
}