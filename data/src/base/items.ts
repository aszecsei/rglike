import { DEFAULT_CATEGORIES } from './vendors'
import { DEFAULT_SPELLS } from './spells'

export const DEFAULT_ITEMS = {
    BEGIN_MAGIC: 'BeginnerMagic',
}

//#region Magic Tomes
rglike.defineItem({
    id: DEFAULT_ITEMS.BEGIN_MAGIC,
    name: rglike.localize('beginner_magic_title', "Beginner's Magic"),
    renderable: {
        glyph: '¶',
        fg: 'M',
        bg: 'k',
        order: 2,
    },
    consumable: {
        effects: {
            teach_spell: DEFAULT_SPELLS.FIREBALL,
        },
    },
    weight: 0.5,
    base_value: 50,
    vendor_category: DEFAULT_CATEGORIES.ALCHEMY.id,
})
//#endregion
