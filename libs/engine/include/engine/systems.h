#pragma once

#include "components.h"
#include <entt/entt.hpp>
#include <algorithm>

namespace engine::systems {

// Camera system - updates camera position to follow target with deadzone
// target_entity: The entity the camera should follow (typically the player)
// camera_entity: The camera entity to update
// deadzone_half_width: Half-width of the deadzone rectangle on each axis
//
// The camera stores its center position. The target can move freely within a
// rectangular deadzone centered on the camera. When the target moves outside
// the deadzone on either axis, the camera follows on that axis to keep the
// target at the deadzone edge.
//
// The rendering system is responsible for converting camera center to viewport bounds.
inline void update_camera_follow(
    entt::registry& registry,
    entt::entity target_entity,
    entt::entity camera_entity,
    int deadzone_half_width = 3
) {
    auto* target_pos = registry.try_get<Position>(target_entity);
    auto* camera = registry.try_get<Camera>(camera_entity);

    if (!target_pos || !camera) return;

    // Calculate offset from camera center to target on each axis
    int dx = target_pos->x - camera->x;
    int dy = target_pos->y - camera->y;

    // Update camera X if target is outside horizontal deadzone
    if (dx < -deadzone_half_width) {
        camera->x = target_pos->x + deadzone_half_width;
    } else if (dx > deadzone_half_width) {
        camera->x = target_pos->x - deadzone_half_width;
    }

    // Update camera Y if target is outside vertical deadzone
    if (dy < -deadzone_half_width) {
        camera->y = target_pos->y + deadzone_half_width;
    } else if (dy > deadzone_half_width) {
        camera->y = target_pos->y - deadzone_half_width;
    }
}

// Helper function to check if a position has an entity that blocks vision
inline bool has_vision_blocker(const entt::registry& registry, int x, int y) {
    auto view = registry.view<Position, BlocksVision>();
    for (auto entity : view) {
        const auto& pos = view.get<Position>(entity);
        if (pos.x == x && pos.y == y) {
            return true;
        }
    }
    return false;
}

// Viewshed system - updates field-of-view for the player
// This uses a simple recursive shadowcasting algorithm to compute visible tiles
// and updates the map's visible/revealed arrays.
// Now also checks for entities with BlocksVision component (like closed doors).
// Only the player's vision is used to determine what tiles are visible.
inline void update_viewshed(
    entt::registry& registry,
    entt::entity map_entity
) {
    auto* map_comp = registry.try_get<MapComponent>(map_entity);
    if (!map_comp) return;

    auto& map = map_comp->map;

    // Clear all visible flags
    std::fill(map.visible.begin(), map.visible.end(), false);

    // Only process vision for the player entity
    auto view = registry.view<Position, VisionComponent, Player>();
    for (auto entity : view) {
        const auto& pos = view.get<Position>(entity);
        const auto& vision = view.get<VisionComponent>(entity);

        // Simple circular FOV - compute visible tiles
        for (int dy = -vision.range; dy <= vision.range; ++dy) {
            for (int dx = -vision.range; dx <= vision.range; ++dx) {
                int x = pos.x + dx;
                int y = pos.y + dy;

                // Check if in range
                if (dx * dx + dy * dy > vision.range * vision.range) continue;

                // Check if in bounds
                if (x < 0 || x >= map.width || y < 0 || y >= map.height) continue;

                // Simple line-of-sight check using Bresenham's line algorithm
                bool blocked = false;
                int x0 = pos.x, y0 = pos.y;
                int x1 = x, y1 = y;

                int dx_line = std::abs(x1 - x0);
                int dy_line = std::abs(y1 - y0);
                int sx = x0 < x1 ? 1 : -1;
                int sy = y0 < y1 ? 1 : -1;
                int err = dx_line - dy_line;

                int cx = x0, cy = y0;
                while (true) {
                    // Check if this tile blocks vision (but don't block the final tile)
                    if ((cx != x1 || cy != y1) && (cx != x0 || cy != y0)) {
                        // Check terrain blocking
                        auto terrain = map.get(cx, cy);
                        if (terrain.has_value() && terrain->blocks_vision) {
                            blocked = true;
                            break;
                        }

                        // Check for entities that block vision (e.g., closed doors)
                        if (has_vision_blocker(registry, cx, cy)) {
                            blocked = true;
                            break;
                        }
                    }

                    if (cx == x1 && cy == y1) break;

                    int e2 = 2 * err;
                    if (e2 > -dy_line) {
                        err -= dy_line;
                        cx += sx;
                    }
                    if (e2 < dx_line) {
                        err += dx_line;
                        cy += sy;
                    }
                }

                if (!blocked) {
                    int index = y * map.width + x;
                    map.visible[index] = true;
                    map.revealed[index] = true;
                }
            }
        }
    }
}

// Time system - advances time when all entities with cooldown have acted
// Finds the minimum cooldown among all entities and decrements all by that amount
// This brings the next actor(s) to cooldown 0, making them ready to act
//
// Returns true if time was advanced, false if any entity has cooldown 0 (still can act)
inline bool advance_time(entt::registry& registry) {
    auto view = registry.view<ActionCooldown>();

    // Find minimum cooldown
    int min_cooldown = std::numeric_limits<int>::max();
    for (auto entity : view) {
        const auto& cooldown = view.get<ActionCooldown>(entity);
        min_cooldown = std::min(min_cooldown, cooldown.cooldown);
    }

    // If any entity has cooldown 0, don't advance time
    if (min_cooldown == 0) {
        return false;
    }

    // If no entities with cooldown exist, don't advance time
    if (min_cooldown == std::numeric_limits<int>::max()) {
        return false;
    }

    // Advance time by decrementing all cooldowns
    for (auto entity : view) {
        auto& cooldown = view.get<ActionCooldown>(entity);
        cooldown.cooldown -= min_cooldown;
    }

    return true;
}

} // namespace engine::systems