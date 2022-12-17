/**
 * @file log_component.hpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#pragma once

#include "../game_log.hpp"
#include <ftxui/component/component.hpp>

/// View/rendering components for rglike
namespace rglike::ui {
    [[nodiscard]] auto GameLogViewer(const GameLog& gameLog) -> ftxui::Component;
} // namespace rglike::ui