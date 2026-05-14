-- Item definitions
-- Engine.CreateItem(table) - fields:
--   id: string (unique)
--   name: string (display name; defaults to id)
--   glyph: string
--   fg_color: {r,g,b} (required)
--   bg_color: {r,g,b} (optional)
--   bold: boolean (optional, defaults to false)
--   render_order: int (optional; defaults to RENDER_ORDER_ITEMS)
--   is_stackable: boolean (optional, defaults to false)
--
-- Stackable items merge into a single inventory slot by id, with a count.
-- Non-stackable items occupy one slot per pickup and survive as their own
-- entt entities so per-instance state (durability, charges, modifiers) can
-- be added later without changing the inventory shape.

Engine.CreateItem({
    id = "gold",
    name = "Gold",
    glyph = "$",
    fg_color = {255, 215, 0},
    bold = true,
    is_stackable = true,
})

Engine.CreateItem({
    id = "healing_potion",
    name = "Healing Potion",
    glyph = "!",
    fg_color = {220, 40, 80},
    is_stackable = false,
})

Engine.CreateItem({
    id = "scroll",
    name = "Scroll",
    glyph = "?",
    fg_color = {220, 220, 160},
    is_stackable = false,
})

Engine.CreateItem({
    id = "bone",
    name = "Bone",
    glyph = ",",
    fg_color = {230, 230, 220},
    is_stackable = true,
})
