export const DEFAULT_CATEGORIES: { [key: string]: rglike.VendorCategory; } = {
    ALCHEMY: {
        id: 'alchemy',
        name: rglike.localize('alchemy_category', 'Alchemy'),
    },
    FOOD: {
        id: 'food',
        name: rglike.localize('food_category', 'Food'),
    },
    JUNK: {
        id: 'junk',
        name: rglike.localize('junk_category', 'Junk'),
    },
    WEAPONS: {
        id: 'weapons',
        name: rglike.localize('weapons_category', 'Weapons'),
    },
    ARMOR: {
        id: 'armor',
        name: rglike.localize('armor_category', 'Armor'),
    },
    CLOTHES: {
        id: 'clothes',
        name: rglike.localize('clothes_category', 'Clothes'),
    },
}

for (const v of Object.values(DEFAULT_CATEGORIES)) {
    rglike.defineVendorCategory(v)
}
