export const DEFAULT_CATEGORIES = {
    ALCHEMY: 'alchemy',
    FOOD: 'food',
    JUNK: 'junk',
    WEAPONS: 'weapons',
    ARMOR: 'armor',
    CLOTHES: 'clothes',
}

for (const v of Object.values(DEFAULT_CATEGORIES)) {
    rglike.defineVendorCategory(v)
}
