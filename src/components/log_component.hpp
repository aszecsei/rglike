/**
 * @file log_component.hpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#pragma once

#include "../game_log.hpp"
#include <ftxui/component/component.hpp>

namespace rglike::components {
    [[nodiscard]] auto GameLogViewer(const GameLog& gameLog) -> ftxui::Component;
} // namespace rglike::components