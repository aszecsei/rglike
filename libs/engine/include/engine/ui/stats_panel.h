#pragma once

#include <engine/world.h>
#include <ftxui/component/component.hpp>

namespace engine::ui {

// HUD panel showing the player's level, XP progress, and resource pools.
// Reads the player entity's StatsComponent each frame, so changes from
// combat and level-ups are reflected immediately.
[[nodiscard]] auto create_stats_panel(const World& world) -> ftxui::Component;

} // namespace engine::ui
