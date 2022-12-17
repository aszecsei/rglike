return {
    {
        name = "Barkeep",
        renderable = {
            glyph = "☻",
            fg = "#EE82EE",
            bg = "#000000",
            order = 1,
        },
        blocks_tile = true,
        vision_range = 4,
        movement = "static",
        attributes = {
            intelligence = 13,
        },
        skills = {
            melee = 2,
        },
        equipped = {
            "Cudgel",
            "Cloth Tunic",
            "Cloth Pants",
            "Slippers",
        },
        faction = "Townsfolk",
        gold = "2d6",
        vendor = { "food" }
    }
}