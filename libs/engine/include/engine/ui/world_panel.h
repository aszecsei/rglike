//
// Created by Alic Szecsei on 10/4/2025.
//

#include <engine/world.h>
#include <ftxui/component/component.hpp>

#pragma once
namespace engine::ui {
    [[nodiscard]] auto create_world_panel(World& world) -> ftxui::Component;
}
