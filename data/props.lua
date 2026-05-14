-- Prop definitions
-- Engine.CreateProp(table) - fields:
--   id: string (unique)
--   name: string (display name; defaults to id)
--   glyph: string (default/closed glyph)
--   fg_color: {r,g,b} (required)
--   bg_color: {r,g,b} (optional)
--   bold: boolean (optional, defaults to false)
--   render_order: int (optional; defaults to props/doors layer)
--   blocks_movement: boolean (optional, defaults to false)
--   blocks_vision: boolean (optional, defaults to false)
--   open_glyph: string (optional). Presence flags this as a door — the
--                spawned entity gets toggleable open/closed state.

-- Door: closed glyph "+", open glyph "'". Blocks movement and vision when
-- closed. When opened by the bump-action both blocks are removed.
Engine.CreateProp({
    id = "door",
    name = "Door",
    glyph = "+",
    fg_color = {139, 69, 19},
    blocks_movement = true,
    blocks_vision = true,
    open_glyph = "'"
})

-- Decorative / collidable furniture. None of these are openable.
Engine.CreateProp({
    id = "candle",
    name = "Candle",
    glyph = "i",
    fg_color = {255, 220, 100},
    bold = true,
    blocks_movement = false,
    blocks_vision = false
})

Engine.CreateProp({
    id = "table",
    name = "Table",
    glyph = "T",
    fg_color = {160, 110, 60},
    blocks_movement = true,
    blocks_vision = false
})

Engine.CreateProp({
    id = "chair",
    name = "Chair",
    glyph = "h",
    fg_color = {140, 90, 50},
    blocks_movement = false,
    blocks_vision = false
})

Engine.CreateProp({
    id = "barrel",
    name = "Barrel",
    glyph = "o",
    fg_color = {120, 80, 40},
    blocks_movement = true,
    blocks_vision = true
})
