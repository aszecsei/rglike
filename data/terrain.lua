-- Terrain definitions
-- Engine.CreateTerrain(table) - takes a table with the following fields:
--   id: string - unique identifier
--   glyph: string - Unicode character to display
--   fg_color: {r, g, b} - foreground color (required)
--   bg_color: {r, g, b} - background color (optional, defaults to black)
--   passable: boolean - whether entities can move through this terrain
--   blocks_vision: boolean - whether this terrain blocks line of sight

-- Floor tiles (passable, don't block vision)
Engine.CreateTerrain({
    id = "floor",
    glyph = ".",
    fg_color = {128, 128, 128},
    passable = true,
    blocks_vision = false
})

Engine.CreateTerrain({
    id = "grass",
    glyph = "\"",
    fg_color = {0, 255, 0},
    passable = true,
    blocks_vision = false
})

-- Wall tiles (impassable, block vision)
Engine.CreateTerrain({
    id = "wall",
    glyph = "#",
    fg_color = {255, 255, 255},
    passable = false,
    blocks_vision = true
})

Engine.CreateTerrain({
    id = "stone_wall",
    glyph = "█",
    fg_color = {128, 128, 128},
    bg_color = {64, 64, 64},
    passable = false,
    blocks_vision = true
})

-- Water (impassable, but doesn't block vision - you can see through it)
Engine.CreateTerrain({
    id = "water",
    glyph = "~",
    fg_color = {0, 128, 255},
    bg_color = {0, 0, 128},
    passable = false,
    blocks_vision = false
})

Engine.CreateTerrain({
    id = "deep_water",
    glyph = "≈",
    fg_color = {0, 64, 200},
    bg_color = {0, 0, 64},
    passable = false,
    blocks_vision = false
})

-- Trees (impassable, block vision)
Engine.CreateTerrain({
    id = "tree",
    glyph = "♠",
    fg_color = {0, 200, 0},
    passable = false,
    blocks_vision = true
})

Engine.CreateTerrain({
    id = "pine_tree",
    glyph = "↑",
    fg_color = {0, 128, 0},
    passable = false,
    blocks_vision = true
})
