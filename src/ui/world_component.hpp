/**
 * @file world_component.hpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#pragma once

#include "../world.hpp"
#include <ftxui/component/component.hpp>

namespace rglike::ui {
    [[nodiscard]] auto WorldViewer(World& gameLog) -> ftxui::Component;
}