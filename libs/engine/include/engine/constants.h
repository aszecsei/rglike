#pragma once

namespace engine::constants {

// Action Costs
// Base unit is 100 to allow 1% granularity in speed modifiers.
// Diagonal cost of 141 approximates sqrt(2) ≈ 1.414 for consistent movement speed.
constexpr int MOVE_COST_ORTHOGONAL = 100;
constexpr int MOVE_COST_DIAGONAL = 141;
constexpr int ACTION_COST_WAIT = 100;
constexpr int ACTION_COST_OPEN_DOOR = 100;
constexpr int ACTION_COST_CLOSE_DOOR = 100;
constexpr int ACTION_COST_ATTACK = 100;
constexpr int ACTION_COST_PICKUP = 100;
constexpr int ACTION_COST_DROP = 100;

// XP reward for killing any mob. Placeholder until per-mob XP is data-driven.
constexpr int XP_REWARD_PER_KILL = 25;

// Vision & Camera
constexpr int DEFAULT_VISION_RANGE = 8;
constexpr int CAMERA_DEADZONE_TILES = 3;

// Render Order
// Higher values are drawn on top of lower values
constexpr int RENDER_ORDER_TERRAIN = 0;
constexpr int RENDER_ORDER_CORPSES = 10;
constexpr int RENDER_ORDER_ITEMS = 50;
constexpr int RENDER_ORDER_DOORS = 50;
constexpr int RENDER_ORDER_MOBS = 80;
constexpr int RENDER_ORDER_PLAYER = 100;

} // namespace engine::constants