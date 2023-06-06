/** @noSelfInFile */

declare namespace rglike {
    interface Book {
        id: string
        name: string
        pages: string[][]
    }
    function defineBook(book: Book): void

    interface Color {
        name: string
        code: string
    }
    function defineColor(color: Color): void

    interface Shader {
        name: string
        colors: string
        type: ShaderType
    }
    enum ShaderType {
        SEQUENCE,
        ALTERNATION,
        CHAOTIC,
        BORDERED,
        SOLID,
    }
    function defineShader(shader: Shader): void

    interface Faction {
        id: string
        name: string
        responses: { [id: string]: Response }
    }
    enum Response {
        IGNORE,
        ATTACK,
        FLEE,
    }
    namespace Faction {
        const DEFAULT = 'Default'
        const SELF = 'Self'
    }
    function defineFaction(faction: Faction): void

    interface VendorCategory {
        id: string
        name: string
    }
    function defineVendorCategory(category: VendorCategory): void

    interface Renderable {
        glyph: string
        fg: string
        bg: string
        order: number
    }

    interface Effects {
        provides_healing?: string
        provides_mana?: string
        teach_spell?: string
        ranged?: number
        damage?: string
        aoe?: string
        confusion?: string
        magic_mapping?: boolean
        town_portal?: boolean
        food?: boolean
        single_activation?: boolean
        particle?: string
        particle_line?: boolean
        remove_curse?: boolean
        identify?: boolean
        slow?: string
        damage_over_time?: string
        target_self?: boolean
    }

    interface Consumable {
        effects: Effects
    }
    interface Item {
        id: string
        name: string
        renderable?: Renderable
        consumable?: Consumable
        weight?: number
        base_value?: number
        vendor_category?: string
    }
    function defineItem(item: Item): void

    interface Spell {
        id: string
        name: string
        mana_cost: number
        effects?: Effects
    }
    function defineSpell(spell: Spell): void

    function setLocale(locale: string): void
    interface Arg {
        key: string
        value: string | number
    }
    function localize(key: string, fallback: string, args?: Arg[]): string
}
