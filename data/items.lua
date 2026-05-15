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
-- Equipment metadata (all optional):
--   equip_slot: one of "main_hand" | "off_hand" | "head" | "chest" | "legs"
--               | "boots" | "gloves" | "amulet" | "ring"
--   two_handed: boolean (main_hand only — locks off_hand while worn)
--   damage_bonus: int (applied to STR-driven damage when worn)
--   defense_bonus: int (subtracts from incoming damage when worn)
--   stat_bonuses: table { strength=N, dexterity=N, ... } applied to
--                 combat-effective stats while worn
--
-- Stackable items merge into a single inventory slot by id, with a count.
-- Non-stackable items occupy one slot per pickup and survive as their own
-- entt entities so per-instance state can be added later without changing
-- the inventory shape. Equippable items must be non-stackable.

-- Consumables --------------------------------------------------------------

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

-- Weapons ------------------------------------------------------------------
--
-- Generate a family of "<base> +N" variants in one loop. Each iteration
-- calls Engine.CreateItem so we end up with N+1 distinct item templates in
-- the registry without copy-pasting tables. The +0 variant uses the bare
-- name and id; +N variants append a suffix and bump damage_bonus.

local function create_weapon_variants(opts)
    -- opts = {
    --   base_id, base_name, glyph, fg_color,
    --   base_damage, plus_max,
    --   two_handed = false (optional),
    -- }
    for plus = 0, opts.plus_max do
        local id = (plus == 0) and opts.base_id
                                or (opts.base_id .. "_plus_" .. plus)
        local name = (plus == 0) and opts.base_name
                                  or (opts.base_name .. " +" .. plus)
        Engine.CreateItem({
            id = id,
            name = name,
            glyph = opts.glyph,
            fg_color = opts.fg_color,
            bold = plus > 0,  -- magical variants pop visually
            equip_slot = "main_hand",
            two_handed = opts.two_handed or false,
            damage_bonus = opts.base_damage + plus,
        })
    end
end

create_weapon_variants({
    base_id = "longsword", base_name = "Longsword",
    glyph = "/", fg_color = {200, 200, 220},
    base_damage = 4, plus_max = 3,
})

create_weapon_variants({
    base_id = "dagger", base_name = "Dagger",
    glyph = "-", fg_color = {180, 180, 200},
    base_damage = 2, plus_max = 2,
})

create_weapon_variants({
    base_id = "greatsword", base_name = "Greatsword",
    glyph = "(", fg_color = {220, 220, 240},
    base_damage = 7, plus_max = 2,
    two_handed = true,
})

Engine.CreateItem({
    id = "staff",
    name = "Wizard's Staff",
    glyph = "|",
    fg_color = {160, 120, 80},
    equip_slot = "main_hand",
    two_handed = true,
    damage_bonus = 2,
    stat_bonuses = { intelligence = 2, attunement = 1 },
})

Engine.CreateItem({
    id = "mace",
    name = "Mace",
    glyph = "\\",
    fg_color = {180, 180, 180},
    equip_slot = "main_hand",
    damage_bonus = 5,
})

-- Off-hand -----------------------------------------------------------------

Engine.CreateItem({
    id = "wooden_shield",
    name = "Wooden Shield",
    glyph = "[",
    fg_color = {150, 100, 60},
    equip_slot = "off_hand",
    defense_bonus = 1,
})

Engine.CreateItem({
    id = "steel_shield",
    name = "Steel Shield",
    glyph = "[",
    fg_color = {180, 180, 200},
    bold = true,
    equip_slot = "off_hand",
    defense_bonus = 3,
})

-- Armor --------------------------------------------------------------------

Engine.CreateItem({
    id = "leather_armor",
    name = "Leather Armor",
    glyph = "]",
    fg_color = {150, 100, 60},
    equip_slot = "chest",
    defense_bonus = 1,
})

Engine.CreateItem({
    id = "chain_mail",
    name = "Chain Mail",
    glyph = "]",
    fg_color = {180, 180, 180},
    equip_slot = "chest",
    defense_bonus = 3,
})

Engine.CreateItem({
    id = "leather_cap",
    name = "Leather Cap",
    glyph = "^",
    fg_color = {150, 100, 60},
    equip_slot = "head",
    defense_bonus = 1,
})

Engine.CreateItem({
    id = "leather_boots",
    name = "Leather Boots",
    glyph = "b",
    fg_color = {150, 100, 60},
    equip_slot = "boots",
    defense_bonus = 1,
})

Engine.CreateItem({
    id = "leather_gloves",
    name = "Leather Gloves",
    glyph = "g",
    fg_color = {150, 100, 60},
    equip_slot = "gloves",
    defense_bonus = 1,
})

Engine.CreateItem({
    id = "leather_leggings",
    name = "Leather Leggings",
    glyph = "p",
    fg_color = {150, 100, 60},
    equip_slot = "legs",
    defense_bonus = 1,
})

Engine.CreateItem({
    id = "robe",
    name = "Robe",
    glyph = "]",
    fg_color = {130, 90, 180},
    equip_slot = "chest",
    stat_bonuses = { intelligence = 1, attunement = 1 },
})

-- Jewelry ------------------------------------------------------------------

Engine.CreateItem({
    id = "amulet_of_health",
    name = "Amulet of Health",
    glyph = "\"",
    fg_color = {220, 180, 60},
    bold = true,
    equip_slot = "amulet",
    stat_bonuses = { constitution = 2 },
})

Engine.CreateItem({
    id = "ring_of_strength",
    name = "Ring of Strength",
    glyph = "=",
    fg_color = {220, 220, 80},
    bold = true,
    equip_slot = "ring",
    stat_bonuses = { strength = 2 },
})

Engine.CreateItem({
    id = "ring_of_dexterity",
    name = "Ring of Dexterity",
    glyph = "=",
    fg_color = {120, 220, 120},
    bold = true,
    equip_slot = "ring",
    stat_bonuses = { dexterity = 2 },
})

Engine.CreateItem({
    id = "ring_of_protection",
    name = "Ring of Protection",
    glyph = "=",
    fg_color = {180, 200, 220},
    bold = true,
    equip_slot = "ring",
    defense_bonus = 1,
})
