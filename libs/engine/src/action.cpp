#include <engine/action.h>
#include <engine/world.h>
#include <engine/components.h>
#include <engine/constants.h>
#include <engine/equipment.h>
#include <engine/faction.h>
#include <engine/item.h>
#include <engine/stats.h>
#include <ftxui/screen/color.hpp>
#include <algorithm>
#include <sstream>

namespace engine {

// MoveAction implementation
ActionResult MoveAction::execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) {
    auto& registry = world.get_registry();
    auto* pos = registry.try_get<Position>(actor_);

    if (!pos) {
        return {ActionResult::Status::Invalid, 0};
    }

    int new_x = pos->x + dx_;
    int new_y = pos->y + dy_;

    // Check if target position is out of bounds
    auto terrain = world.get_terrain(new_x, new_y);
    if (!terrain.has_value()) {
        return {ActionResult::Status::Invalid, 0};
    }

    // Check for closed doors at the target position
    auto door_view = registry.view<Position, Door, BlocksMovement>();
    for (auto door_entity : door_view) {
        const auto& door_pos = door_view.get<Position>(door_entity);
        if (door_pos.x == new_x && door_pos.y == new_y) {
            // Found a closed door - queue an OpenDoorAction instead
            action_queue.push(std::make_unique<OpenDoorAction>(actor_, new_x, new_y));
            return {ActionResult::Status::Alternative, 0};
        }
    }

    // Check if the terrain is passable
    if (!terrain->passable) {
        return {ActionResult::Status::Failure, 0};
    }

    // Bump-attack: if a combatant occupies the target tile and the actor's
    // faction wants to attack them, swap this move for an AttackAction. Mirrors
    // the door-bump pattern above.
    const auto* faction_registry = world.get_faction_registry();
    const auto* actor_faction_comp = registry.try_get<FactionComponent>(actor_);
    if (faction_registry && actor_faction_comp) {
        auto combatant_view = registry.view<Position, StatsComponent, FactionComponent>();
        for (auto target_entity : combatant_view) {
            if (target_entity == actor_) continue;
            const auto& target_pos = combatant_view.get<Position>(target_entity);
            if (target_pos.x != new_x || target_pos.y != new_y) continue;

            const auto& target_faction_comp = combatant_view.get<FactionComponent>(target_entity);
            auto actor_faction = faction_registry->get(actor_faction_comp->faction_id);
            if (!actor_faction) break;

            if (actor_faction->get_response_to(target_faction_comp.faction_id) == FactionResponse::ATTACK) {
                action_queue.push(std::make_unique<AttackAction>(actor_, target_entity));
                return {ActionResult::Status::Alternative, 0};
            }
            break;  // Only one entity can occupy a tile in practice.
        }
    }

    // Check for entities that block movement
    auto blocking_view = registry.view<Position, BlocksMovement>();
    for (auto blocking_entity : blocking_view) {
        const auto& blocking_pos = blocking_view.get<Position>(blocking_entity);
        if (blocking_pos.x == new_x && blocking_pos.y == new_y) {
            // Position is blocked by another entity
            return {ActionResult::Status::Failure, 0};
        }
    }

    // Calculate cooldown cost based on movement type
    bool is_diagonal = (dx_ != 0 && dy_ != 0);
    int cost = is_diagonal ? constants::MOVE_COST_DIAGONAL : constants::MOVE_COST_ORTHOGONAL;

    // Move is valid - execute it
    pos->x = new_x;
    pos->y = new_y;
    return {ActionResult::Status::Success, cost};
}

std::string MoveAction::description() const {
    std::ostringstream oss;
    oss << "Move (" << dx_ << ", " << dy_ << ")";
    return oss.str();
}

// OpenDoorAction implementation
ActionResult OpenDoorAction::execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) {
    auto& registry = world.get_registry();

    // Find the door at the target position
    auto door_view = registry.view<Position, Door, BlocksMovement>();
    for (auto door_entity : door_view) {
        const auto& door_pos = door_view.get<Position>(door_entity);
        if (door_pos.x == x_ && door_pos.y == y_) {
            auto& door = door_view.get<Door>(door_entity);

            // Open the door
            door.is_open = true;

            // Update the door's renderable to show open state
            auto* renderable = registry.try_get<Renderable>(door_entity);
            if (renderable) {
                renderable->glyph = door.open_glyph;
            }

            // Remove blocking components when door is opened
            registry.remove<BlocksMovement>(door_entity);
            registry.remove<BlocksVision>(door_entity);

            return {ActionResult::Status::Success, constants::ACTION_COST_OPEN_DOOR};
        }
    }

    // No door found at this position
    return {ActionResult::Status::Invalid, 0};
}

std::string OpenDoorAction::description() const {
    std::ostringstream oss;
    oss << "Open door at (" << x_ << ", " << y_ << ")";
    return oss.str();
}

// CloseDoorAction implementation
ActionResult CloseDoorAction::execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) {
    auto& registry = world.get_registry();

    // Find an open door at the target position
    auto door_view = registry.view<Position, Door>();
    for (auto door_entity : door_view) {
        const auto& door_pos = door_view.get<Position>(door_entity);
        auto& door = door_view.get<Door>(door_entity);

        if (door_pos.x == x_ && door_pos.y == y_ && door.is_open) {
            // Check if any entity is standing on the door
            auto entity_view = registry.view<Position>();
            for (auto entity : entity_view) {
                // Skip the door entity itself
                if (entity == door_entity) continue;

                const auto& pos = entity_view.get<Position>(entity);
                if (pos.x == x_ && pos.y == y_) {
                    // Can't close door with entity in the way
                    return {ActionResult::Status::Failure, 0};
                }
            }

            // Close the door (using mutable reference obtained from view)
            door.is_open = false;

            // Update the door's renderable to show closed state
            auto* renderable = registry.try_get<Renderable>(door_entity);
            if (renderable) {
                renderable->glyph = door.closed_glyph;
            }

            // Add blocking components back
            registry.emplace<BlocksMovement>(door_entity);
            registry.emplace<BlocksVision>(door_entity);

            return {ActionResult::Status::Success, constants::ACTION_COST_CLOSE_DOOR};
        }
    }

    // No open door found at this position
    return {ActionResult::Status::Invalid, 0};
}

std::string CloseDoorAction::description() const {
    std::ostringstream oss;
    oss << "Close door at (" << x_ << ", " << y_ << ")";
    return oss.str();
}

// WaitAction implementation
ActionResult WaitAction::execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) {
    // Simply pass the turn
    return {ActionResult::Status::Success, constants::ACTION_COST_WAIT};
}

std::string WaitAction::description() const {
    return "Wait";
}

// Helper: return the entity's display name, or a generic fallback.
static std::string display_name(const entt::registry& registry, entt::entity e) {
    const auto* nc = registry.try_get<NameComponent>(e);
    return nc ? nc->name : std::string{"something"};
}

// AttackAction implementation
ActionResult AttackAction::execute(World& world, std::queue<std::unique_ptr<Action>>& action_queue) {
    auto& registry = world.get_registry();

    if (!registry.valid(actor_) || !registry.valid(target_)) {
        return {ActionResult::Status::Invalid, 0};
    }

    auto* attacker_stats = registry.try_get<StatsComponent>(actor_);
    auto* target_stats = registry.try_get<StatsComponent>(target_);
    auto* target_pos = registry.try_get<Position>(target_);
    if (!attacker_stats || !target_stats || !target_pos) {
        return {ActionResult::Status::Invalid, 0};
    }

    // Damage formula: effective STR + weapon damage_bonus,
    // mitigated by half of effective CON plus armor defense_bonus,
    // floor of 1. "Effective" stats add summed equipped stat_bonuses on
    // top of base StatsComponent values; resource pool maxes (HP, mana)
    // still derive from base stats only, so wearing a CON-boosting
    // amulet hardens combat math without retroactively raising HP cap.
    const auto* item_reg = world.get_item_registry();
    int attacker_str = item_reg
        ? effective_stat(registry, *item_reg, actor_, CoreStat::STRENGTH)
        : attacker_stats->get_stat(CoreStat::STRENGTH);
    int target_con = item_reg
        ? effective_stat(registry, *item_reg, target_, CoreStat::CONSTITUTION)
        : target_stats->get_stat(CoreStat::CONSTITUTION);
    int weapon_damage = item_reg
        ? equipment_damage_bonus(registry, *item_reg, actor_) : 0;
    int armor_value = item_reg
        ? equipment_defense_bonus(registry, *item_reg, target_) : 0;
    int damage = std::max(1, attacker_str + weapon_damage - target_con / 2 - armor_value);

    target_stats->modify_resource(ResourcePool::HEALTH, -damage);

    const bool attacker_is_player = registry.all_of<Player>(actor_);
    const bool target_is_player = registry.all_of<Player>(target_);
    const std::string attacker_name = display_name(registry, actor_);
    const std::string target_name = display_name(registry, target_);

    auto& log = world.get_game_log();
    if (attacker_is_player) {
        log.entry()
           .color(ftxui::Color::Yellow).text("You strike ").bold().text(target_name).reset_style()
           .color(ftxui::Color::Yellow).text(" for ").bold().text(std::to_string(damage)).reset_style()
           .color(ftxui::Color::Yellow).text(" damage.")
           .log();
    } else if (target_is_player) {
        log.entry()
           .color(ftxui::Color::Red).bold().text(attacker_name).reset_style()
           .color(ftxui::Color::Red).text(" strikes you for ").bold().text(std::to_string(damage)).reset_style()
           .color(ftxui::Color::Red).text(" damage.")
           .log();
    } else {
        log.entry().dim().text(attacker_name + " strikes " + target_name + ".").log();
    }

    const int target_hp = target_stats->get_resource(ResourcePool::HEALTH);
    if (target_hp <= 0) {
        if (target_is_player) {
            // Player death is sticky state on World; the scene transitions on
            // its next update tick. Don't destroy the entity — the game-over
            // scene needs the final stats snapshot.
            world.set_player_dead(attacker_name);
            log.entry().color(ftxui::Color::Red).bold().text(attacker_name + " slays you!").log();
        } else {
            // Mob death: bloodstain, destroy entity, award XP.
            if (auto* map_comp = world.get_map_component()) {
                int idx = target_pos->y * map_comp->map.width + target_pos->x;
                map_comp->map.bloodstains.insert(idx);
            }

            if (attacker_is_player) {
                log.entry()
                   .color(ftxui::Color::Green).text("You slay ").bold().text(target_name).reset_style()
                   .color(ftxui::Color::Green).text("!").log();
            } else {
                log.entry().dim().text(attacker_name + " slays " + target_name + ".").log();
            }

            // XP and pending level-up tag on the attacker.
            const bool leveled = attacker_stats->add_experience(constants::XP_REWARD_PER_KILL);
            if (attacker_is_player) {
                log.entry().color(ftxui::Color::GreenLight)
                   .text("(+" + std::to_string(constants::XP_REWARD_PER_KILL) + " XP)")
                   .log();
            }
            if (leveled) {
                // Tag the attacker; the scene applies the growth pattern.
                if (!registry.all_of<PendingLevelUp>(actor_)) {
                    registry.emplace<PendingLevelUp>(actor_);
                } else {
                    registry.get<PendingLevelUp>(actor_).levels += 1;
                }
            }

            // Roll drops before destroying the target so we still have its
            // position. Each entry is an independent Bernoulli trial; on
            // success, count is uniform on [min_count, max_count]. Stackable
            // items spawn a single ground entity with the rolled count;
            // non-stackable items spawn one entity per unit.
            const int drop_x = target_pos->x;
            const int drop_y = target_pos->y;
            if (auto* drops = registry.try_get<DropsComponent>(target_)) {
                auto& rng = world.get_rng();
                for (const auto& entry : drops->entries) {
                    if (rng.uniform() > entry.chance) continue;
                    int count = (entry.max_count <= entry.min_count)
                                ? entry.min_count
                                : rng.range(entry.min_count, entry.max_count);
                    if (count <= 0) continue;

                    bool stackable = false;
                    if (item_reg) {
                        if (auto tmpl = item_reg->get(entry.item_id)) {
                            stackable = tmpl->is_stackable;
                        } else {
                            // Unknown id — skip silently; the data file is at fault
                            // and the load-time logger will already have shown errors.
                            continue;
                        }
                    }
                    if (stackable) {
                        world.spawn_ground_item(entry.item_id, drop_x, drop_y, count);
                    } else {
                        for (int i = 0; i < count; ++i) {
                            world.spawn_ground_item(entry.item_id, drop_x, drop_y, 1);
                        }
                    }
                }
            }

            registry.destroy(target_);
        }
    }

    return {ActionResult::Status::Success, constants::ACTION_COST_ATTACK};
}

std::string AttackAction::description() const {
    std::ostringstream oss;
    oss << "Attack entity " << static_cast<uint32_t>(target_);
    return oss.str();
}

// Build a name string for log messages, including a stack-count suffix for
// stackable items so "You pick up Gold (x5)" reads naturally.
static std::string item_display_with_count(const std::string& base_name,
                                            const ItemComponent& ic) {
    if (ic.is_stackable && ic.count > 1) {
        return base_name + " (x" + std::to_string(ic.count) + ")";
    }
    return base_name;
}

ActionResult PickupAction::execute(World& world, std::queue<std::unique_ptr<Action>>&) {
    auto& registry = world.get_registry();
    if (!registry.valid(actor_) || !registry.valid(item_)) {
        return {ActionResult::Status::Invalid, 0};
    }

    auto* actor_pos = registry.try_get<Position>(actor_);
    auto* inv = registry.try_get<InventoryComponent>(actor_);
    auto* item_pos = registry.try_get<Position>(item_);
    auto* item_comp = registry.try_get<ItemComponent>(item_);
    if (!actor_pos || !inv || !item_pos || !item_comp) {
        return {ActionResult::Status::Invalid, 0};
    }
    if (actor_pos->x != item_pos->x || actor_pos->y != item_pos->y) {
        return {ActionResult::Status::Invalid, 0};
    }

    const bool actor_is_player = registry.all_of<Player>(actor_);
    auto& log = world.get_game_log();
    const std::string item_name = registry.try_get<NameComponent>(item_)
                                  ? registry.get<NameComponent>(item_).name
                                  : std::string{"item"};

    // Stackable + matching slot exists: merge.
    if (item_comp->is_stackable) {
        for (auto& slot : inv->slots) {
            if (slot.stack_item_id && *slot.stack_item_id == item_comp->item_id) {
                slot.count += item_comp->count;
                if (actor_is_player) {
                    log.entry().text("You pick up ").bold().text(
                        item_display_with_count(item_name, *item_comp)).log();
                }
                registry.destroy(item_);
                return {ActionResult::Status::Success, constants::ACTION_COST_PICKUP};
            }
        }
    }

    // Need a new slot.
    char letter = allocate_inventory_letter(*inv);
    if (letter == 0) {
        if (actor_is_player) {
            log.entry().color(ftxui::Color::YellowLight).text("Your pack is full.").log();
        }
        return {ActionResult::Status::Failure, constants::ACTION_COST_PICKUP};
    }

    InventorySlot new_slot;
    new_slot.letter = letter;
    if (item_comp->is_stackable) {
        new_slot.stack_item_id = item_comp->item_id;
        new_slot.count = item_comp->count;
        if (actor_is_player) {
            log.entry().text("You pick up ").bold().text(
                item_display_with_count(item_name, *item_comp)).text(" (").text(std::string(1, letter)).text(").").log();
        }
        registry.destroy(item_);
    } else {
        // Non-stackable: detach Position so the item disappears from the world,
        // tag with Carried, and store the entity handle on the slot.
        new_slot.unique_item = item_;
        new_slot.count = 1;
        registry.remove<Position>(item_);
        registry.emplace_or_replace<Carried>(item_, Carried{actor_});
        if (actor_is_player) {
            log.entry().text("You pick up ").bold().text(item_name)
               .text(" (").text(std::string(1, letter)).text(").").log();
        }
    }
    inv->slots.push_back(std::move(new_slot));
    return {ActionResult::Status::Success, constants::ACTION_COST_PICKUP};
}

std::string PickupAction::description() const {
    std::ostringstream oss;
    oss << "Pick up entity " << static_cast<uint32_t>(item_);
    return oss.str();
}

ActionResult DropAction::execute(World& world, std::queue<std::unique_ptr<Action>>&) {
    auto& registry = world.get_registry();
    if (!registry.valid(actor_)) return {ActionResult::Status::Invalid, 0};

    auto* actor_pos = registry.try_get<Position>(actor_);
    auto* inv = registry.try_get<InventoryComponent>(actor_);
    if (!actor_pos || !inv) return {ActionResult::Status::Invalid, 0};

    auto it = std::find_if(inv->slots.begin(), inv->slots.end(),
                            [this](const InventorySlot& s) { return s.letter == letter_; });
    if (it == inv->slots.end()) return {ActionResult::Status::Invalid, 0};

    const bool actor_is_player = registry.all_of<Player>(actor_);
    auto& log = world.get_game_log();

    if (it->stack_item_id) {
        // Stackable: spawn the whole stack on the ground as a single entity.
        const std::string id = *it->stack_item_id;
        const int count = it->count;
        auto e = world.spawn_ground_item(id, actor_pos->x, actor_pos->y, count);
        if (e == entt::null) {
            // Item id not registered — refuse to lose the stack.
            return {ActionResult::Status::Invalid, 0};
        }
        if (actor_is_player) {
            std::string name = registry.try_get<NameComponent>(e)
                               ? registry.get<NameComponent>(e).name : id;
            if (count > 1) name += " (x" + std::to_string(count) + ")";
            log.entry().text("You drop ").bold().text(name).log();
        }
    } else if (it->unique_item) {
        entt::entity carried = *it->unique_item;
        if (!registry.valid(carried)) {
            // Defensive — slot points at a dead entity. Clear it.
            inv->slots.erase(it);
            return {ActionResult::Status::Invalid, 0};
        }
        registry.emplace_or_replace<Position>(carried, actor_pos->x, actor_pos->y);
        registry.remove<Carried>(carried);
        if (actor_is_player) {
            std::string name = registry.try_get<NameComponent>(carried)
                               ? registry.get<NameComponent>(carried).name : std::string{"item"};
            log.entry().text("You drop ").bold().text(name).log();
        }
    } else {
        return {ActionResult::Status::Invalid, 0};
    }

    inv->slots.erase(it);
    return {ActionResult::Status::Success, constants::ACTION_COST_DROP};
}

std::string DropAction::description() const {
    std::ostringstream oss;
    oss << "Drop slot " << letter_;
    return oss.str();
}

// EquipAction / UnequipAction ----------------------------------------------
//
// Equipped items live in EquipmentComponent.slots rather than the bag, so
// equipping moves the item entity out of an InventorySlot and unequipping
// moves it back. The entity itself persists across the transition,
// preserving any future per-instance state (durability, enchants).

namespace {

// Resolve a non-stackable inventory slot to its backing entity. Returns
// entt::null when the letter is missing, the slot is stackable (which is
// never equippable), or the unique_item handle is stale.
entt::entity find_inventory_entity(InventoryComponent& inv, char letter,
                                    const entt::registry& reg,
                                    std::vector<InventorySlot>::iterator& out_it) {
    out_it = std::find_if(inv.slots.begin(), inv.slots.end(),
                          [letter](const InventorySlot& s) { return s.letter == letter; });
    if (out_it == inv.slots.end()) return entt::null;
    if (!out_it->unique_item) return entt::null;
    entt::entity e = *out_it->unique_item;
    if (!reg.valid(e)) return entt::null;
    return e;
}

// Return the EquipmentComponent for `actor`, creating an empty one if it
// is missing. Equipment can be initialized at character creation, but
// mob equipping (e.g. from a future loot pickup AI) should not crash if
// the component was never attached.
EquipmentComponent& ensure_equipment(entt::registry& reg, entt::entity actor) {
    if (auto* eq = reg.try_get<EquipmentComponent>(actor)) return *eq;
    return reg.emplace<EquipmentComponent>(actor);
}

// Common log fragment: "<item name>".
std::string equipped_item_name(const entt::registry& reg, entt::entity e) {
    if (const auto* nc = reg.try_get<NameComponent>(e)) return nc->name;
    return "something";
}

} // namespace

ActionResult EquipAction::execute(World& world, std::queue<std::unique_ptr<Action>>&) {
    auto& registry = world.get_registry();
    if (!registry.valid(actor_)) return {ActionResult::Status::Invalid, 0};

    auto* inv = registry.try_get<InventoryComponent>(actor_);
    const auto* item_reg = world.get_item_registry();
    if (!inv || !item_reg) return {ActionResult::Status::Invalid, 0};

    const bool actor_is_player = registry.all_of<Player>(actor_);
    auto& log = world.get_game_log();

    std::vector<InventorySlot>::iterator slot_it;
    entt::entity item_entity = find_inventory_entity(*inv, letter_, registry, slot_it);
    if (item_entity == entt::null) return {ActionResult::Status::Invalid, 0};

    const auto* ic = registry.try_get<ItemComponent>(item_entity);
    if (!ic) return {ActionResult::Status::Invalid, 0};
    auto tmpl = item_reg->get(ic->item_id);
    if (!tmpl || !tmpl->equip_slot) {
        if (actor_is_player) {
            log.entry().color(ftxui::Color::YellowLight)
               .text("You can't equip that.").log();
        }
        return {ActionResult::Status::Failure, constants::ACTION_COST_EQUIP};
    }

    auto& equip = ensure_equipment(registry, actor_);
    const ItemSlotKind kind = *tmpl->equip_slot;

    // Resolve target physical slot. Rings auto-fill the first empty ring,
    // otherwise the scene must have supplied a ring_choice (after a
    // follow-up modal). For non-rings there is exactly one candidate.
    EquipmentSlot target_slot;
    if (kind == ItemSlotKind::RING) {
        const bool r1_empty = equip.slots.find(EquipmentSlot::RING_1) == equip.slots.end();
        const bool r2_empty = equip.slots.find(EquipmentSlot::RING_2) == equip.slots.end();
        if (r1_empty) target_slot = EquipmentSlot::RING_1;
        else if (r2_empty) target_slot = EquipmentSlot::RING_2;
        else if (ring_choice_ &&
                 (*ring_choice_ == EquipmentSlot::RING_1 ||
                  *ring_choice_ == EquipmentSlot::RING_2)) {
            target_slot = *ring_choice_;
        } else {
            // Should not happen — the scene is expected to prompt and resupply
            // ring_choice when both ring slots are full.
            if (actor_is_player) {
                log.entry().color(ftxui::Color::YellowLight)
                   .text("Choose which ring to replace.").log();
            }
            return {ActionResult::Status::Invalid, 0};
        }
    } else {
        auto candidates = physical_slots_for(kind);
        if (candidates.empty()) return {ActionResult::Status::Invalid, 0};
        target_slot = candidates.front();
    }

    // Determine which currently-equipped items must be evicted back to
    // the bag. Direct conflict: anything already in target_slot. Cross
    // conflict: a two-handed main weapon evicts off-hand; an off-hand
    // item evicts a two-handed main weapon.
    std::vector<EquipmentSlot> evictions;
    auto target_existing = equip.slots.find(target_slot);
    if (target_existing != equip.slots.end()) {
        evictions.push_back(target_slot);
    }

    if (target_slot == EquipmentSlot::MAIN_HAND && tmpl->two_handed) {
        auto oh = equip.slots.find(EquipmentSlot::OFF_HAND);
        if (oh != equip.slots.end() && oh->first != target_slot) {
            evictions.push_back(EquipmentSlot::OFF_HAND);
        }
    } else if (target_slot == EquipmentSlot::OFF_HAND) {
        auto mh = equip.slots.find(EquipmentSlot::MAIN_HAND);
        if (mh != equip.slots.end()) {
            // Check whether the currently-equipped main-hand is two-handed.
            if (const auto* mh_ic = registry.try_get<ItemComponent>(mh->second)) {
                if (auto mh_tmpl = item_reg->get(mh_ic->item_id)) {
                    if (mh_tmpl->two_handed) {
                        evictions.push_back(EquipmentSlot::MAIN_HAND);
                    }
                }
            }
        }
    }

    // Bag capacity check. The item being equipped frees one slot; each
    // evicted item needs one back. If the net change overflows the cap,
    // refuse without mutating state.
    const std::size_t projected = inv->slots.size() - 1 + evictions.size();
    if (projected > InventoryComponent::MAX_SLOTS) {
        if (actor_is_player) {
            log.entry().color(ftxui::Color::YellowLight)
               .text("Your pack is too full to swap that gear.").log();
        }
        return {ActionResult::Status::Failure, constants::ACTION_COST_EQUIP};
    }

    // Evict conflicting items into the bag. Order matters only insofar
    // as letter assignment depends on which slots are currently free;
    // since we already confirmed capacity, every push will succeed.
    for (EquipmentSlot evict_slot : evictions) {
        auto it = equip.slots.find(evict_slot);
        if (it == equip.slots.end()) continue;
        entt::entity evicted = it->second;
        equip.slots.erase(it);
        registry.remove<Equipped>(evicted);

        char letter = allocate_inventory_letter(*inv);
        // Should never be 0 given the capacity pre-check above.
        InventorySlot returning;
        returning.letter = letter;
        returning.unique_item = evicted;
        returning.count = 1;
        inv->slots.push_back(std::move(returning));

        if (actor_is_player) {
            log.entry().text("You unequip ").bold()
               .text(equipped_item_name(registry, evicted)).text(".").log();
        }
    }

    // Move the chosen item from the bag to the equipment slot. The bag
    // iterator may have been invalidated by the push_back above, so
    // re-locate by letter.
    {
        auto re_it = std::find_if(inv->slots.begin(), inv->slots.end(),
                                   [this](const InventorySlot& s) { return s.letter == letter_; });
        if (re_it != inv->slots.end()) inv->slots.erase(re_it);
    }
    registry.emplace_or_replace<Equipped>(item_entity, Equipped{actor_, target_slot});
    equip.slots[target_slot] = item_entity;

    if (actor_is_player) {
        const std::string name = equipped_item_name(registry, item_entity);
        log.entry().color(ftxui::Color::GreenLight)
           .text("You equip ").bold().text(name).reset_style()
           .text(" (").text(std::string(display_name(target_slot))).text(").")
           .log();
    }
    return {ActionResult::Status::Success, constants::ACTION_COST_EQUIP};
}

std::string EquipAction::description() const {
    std::ostringstream oss;
    oss << "Equip slot " << letter_;
    return oss.str();
}

ActionResult UnequipAction::execute(World& world, std::queue<std::unique_ptr<Action>>&) {
    auto& registry = world.get_registry();
    if (!registry.valid(actor_)) return {ActionResult::Status::Invalid, 0};

    auto* equip = registry.try_get<EquipmentComponent>(actor_);
    auto* inv = registry.try_get<InventoryComponent>(actor_);
    if (!equip || !inv) return {ActionResult::Status::Invalid, 0};

    const bool actor_is_player = registry.all_of<Player>(actor_);
    auto& log = world.get_game_log();

    auto it = equip->slots.find(slot_);
    if (it == equip->slots.end()) {
        if (actor_is_player) {
            log.entry().text("Nothing equipped there.").log();
        }
        return {ActionResult::Status::Failure, constants::ACTION_COST_UNEQUIP};
    }

    char letter = allocate_inventory_letter(*inv);
    if (letter == 0) {
        if (actor_is_player) {
            log.entry().color(ftxui::Color::YellowLight).text("Your pack is full.").log();
        }
        return {ActionResult::Status::Failure, constants::ACTION_COST_UNEQUIP};
    }

    entt::entity e = it->second;
    equip->slots.erase(it);
    registry.remove<Equipped>(e);

    InventorySlot returning;
    returning.letter = letter;
    returning.unique_item = e;
    returning.count = 1;
    inv->slots.push_back(std::move(returning));

    if (actor_is_player) {
        log.entry().text("You unequip ").bold()
           .text(equipped_item_name(registry, e)).text(".").log();
    }
    return {ActionResult::Status::Success, constants::ACTION_COST_UNEQUIP};
}

std::string UnequipAction::description() const {
    std::ostringstream oss;
    oss << "Unequip slot " << static_cast<int>(slot_);
    return oss.str();
}

} // namespace engine