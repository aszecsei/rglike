export const DEFAULT_SPELLS = {
    MAGIC_MISSILE: 'mmissile',
    FIREBALL: 'fireball',
}

rglike.defineSpell({
    id: DEFAULT_SPELLS.MAGIC_MISSILE,
    name: 'Magic Missile',
    mana_cost: 10,
    effects: {
        ranged: 6,
        damage: '5',
        particle: '▓;#00FFFF;400.0',
    },
})
